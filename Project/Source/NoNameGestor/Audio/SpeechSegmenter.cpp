#include "SpeechSegmenter.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "NoNameGestor/Audio/ThreadWorker.h"
#include "NoNameGestor/External/whisper.cpp/include/whisper.h"

SpeechSegmenter::SpeechSegmenter()
{
    inst = fvad_new();

    if (inst == nullptr)
        return;

    if (fvad_set_sample_rate(inst, SampleRate) != 0 ||
        fvad_set_mode(inst, 2) != 0)
    {
        fvad_free(inst);
        inst = nullptr;
    }

    whisper_context_params params = whisper_context_default_params();
    ctx = whisper_init_from_file_with_params("ggml-base.en.bin", params);

    transcriber = new ThreadWorker<Segment, Transcription>(
        [&](Segment& segment)
        {
            // Whisper es queixa si l'àudio dura menys d'1 s: afegim silenci fins a ~1,1 s.
            std::vector<float> audio = segment.samples;
            const size_t minSamples = 17600; // 1,1 s a 16 kHz
            if (audio.size() < minSamples)
                audio.resize(minSamples, 0.0f);

            whisper_full_params wp = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

            wp.language = "en";
            wp.translate = false;
            wp.n_threads = (int)std::min(4u, std::max(1u, std::thread::hardware_concurrency()));
            wp.no_context = true; // cada frase comença de zero
            wp.single_segment = true; // pensat per a frases curtes
            wp.no_timestamps = true;
            wp.print_progress = false;
            wp.print_realtime = false;
            wp.print_timestamps = false;
            wp.print_special = false;
            wp.suppress_blank = true;
            wp.suppress_nst = true;

            auto t0 = std::chrono::steady_clock::now();
            int rc = whisper_full(ctx, wp, audio.data(), (int)audio.size());
            auto t1 = std::chrono::steady_clock::now();

            Transcription result;

            result.ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

            if (rc != 0)
            {
                result.text = {};
                return result;
            }

            std::string text;
            const int n = whisper_full_n_segments(ctx);
            for (int i = 0; i < n; ++i)
                text += whisper_full_get_segment_text(ctx, i);

            // Whisper sol posar un espai al davant.
            size_t start = text.find_first_not_of(" \t\n");
            if (start == std::string::npos)
            {
                result.text = {};
                return result;
            }

            size_t end = text.find_last_not_of(" \t\n");
            result.text = text.substr(start, end - start + 1);
            return result;
        }
    );
}

SpeechSegmenter::~SpeechSegmenter()
{
    if (inst != nullptr)
    {
        fvad_free(inst);
        inst = nullptr;
    }

    if (transcriber != nullptr)
        delete transcriber;

    if (ctx != nullptr)
        whisper_free(ctx);
}

void SpeechSegmenter::Process(const float* samples, size_t n)
{
    if (inst == nullptr || samples == nullptr)
        return;

    while (n > 0)
    {
        float sample = *samples++;

        --n;

        if (!std::isfinite(sample))
            sample = 0.0f;

        sample = std::clamp(sample, -1.0f, 1.0f);

        floatFrame[filled++] = sample;

        if (filled == FrameSamples)
        {
            ProcessFrame();
            filled = 0;
        }
    }
}

void SpeechSegmenter::Transcribe(Segment&& segment)
{
    if (ctx == nullptr || segment.samples.empty())
        return;

    transcriber->Push(segment);
}

bool SpeechSegmenter::DetectVoice()
{
    for (size_t i = 0; i < FrameSamples; ++i)
    {
        const float sample = floatFrame[i];

        const int32_t pcm = sample < 0.0f
            ? static_cast<int32_t>(std::lround(sample * 32768.0f))
            : static_cast<int32_t>(std::lround(sample * 32767.0f));

        pcmFrame[i] = static_cast<int16_t>(pcm);
    }

    const int result = fvad_process(
        inst,
        pcmFrame.data(),
        FrameSamples
    );

    return result == 1;
}

void SpeechSegmenter::ProcessFrame()
{
    const bool voiced = DetectVoice();

    switch (state)
    {
    case State::Idle:
    {
        if (voiced)
        {
            StartCandidate();
        }
        else
        {
            PushPreRoll();
        }

        break;
    }

    case State::Pending:
    {
        // Acumulem el frame actual.
        currentSamples.insert(
            currentSamples.end(),
            floatFrame.begin(),
            floatFrame.end()
        );

        if (voiced)
        {
            ++candidateVoicedFrames;
            candidateSilenceFrames = 0;
        }
        else
        {
            ++candidateSilenceFrames;
        }

        PushPreRoll();

        // Ja tenim prou activitat de veu.
        if (candidateVoicedFrames >= minSpeechFrames)
        {
            ConfirmSpeech();
        }
        // Massa silenci sense confirmar la frase.
        else if (candidateSilenceFrames >= startTimeoutFrames)
        {
            state = State::Idle;

            currentSamples.clear();

            candidateVoicedFrames = 0;
            candidateSilenceFrames = 0;
            hangoverSilenceFrames = 0;
        }

        break;
    }

    case State::Speaking:
    {
        currentSamples.insert(currentSamples.end(), floatFrame.begin(), floatFrame.end());

        // 1. Límit de durada: independent de si hi ha veu o silenci.
        if (currentSamples.size() >= MaxSpeechFrames * FrameSamples)
        {
            FinishSpeech();
            break;
        }

        // 2. Hangover normal.
        if (voiced)
        {
            hangoverSilenceFrames = 0;
        }
        else
        {
            ++hangoverSilenceFrames;

            if (hangoverSilenceFrames >= hangoverFrames)
                FinishSpeech();
        }

        break;
    }
    }
}

void SpeechSegmenter::StartCandidate()
{
    state = State::Pending;

    currentSamples.clear();

    // Recuperem els frames anteriors a la detecció.
    for (const auto& frame : preRollFrames)
    {
        currentSamples.insert(
            currentSamples.end(),
            frame.begin(),
            frame.end()
        );
    }

    // Afegim el frame que ha activat el VAD.
    currentSamples.insert(
        currentSamples.end(),
        floatFrame.begin(),
        floatFrame.end()
    );

    candidateVoicedFrames = 1;
    candidateSilenceFrames = 0;
    hangoverSilenceFrames = 0;

    PushPreRoll();

    if (candidateVoicedFrames >= minSpeechFrames)
        ConfirmSpeech();
}

void SpeechSegmenter::ConfirmSpeech()
{
    state = State::Speaking;

    candidateVoicedFrames = 0;
    candidateSilenceFrames = 0;
    hangoverSilenceFrames = 0;

    // Ja no necessitem l'historial anterior.
    preRollFrames.clear();
}

void SpeechSegmenter::FinishSpeech()
{
    if (!currentSamples.empty())
    {
        Segment segment;

        if (hangoverSilenceFrames > TailKeepFrames)
        {
            size_t removeFrames = hangoverSilenceFrames - TailKeepFrames;
            size_t removeSamples = removeFrames * FrameSamples;
            if (removeSamples < currentSamples.size())
                currentSamples.resize(currentSamples.size() - removeSamples);
        }

        // Transferim la propietat de les mostres al segment.
        segment.samples = std::move(currentSamples);

        lastSegmentDurationSeconds = segment.GetDurationSeconds();
        ++totalSegmentsCompleted;
        completedSegments.push_back(std::move(segment));
    }

    currentSamples.clear();

    state = State::Idle;

    candidateVoicedFrames = 0;
    candidateSilenceFrames = 0;
    hangoverSilenceFrames = 0;

    preRollFrames.clear();
}

void SpeechSegmenter::PushPreRoll()
{
    if (preRollFrameLimit == 0)
        return;

    while (preRollFrames.size() >= preRollFrameLimit)
        preRollFrames.pop_front();

    preRollFrames.push_back(floatFrame);
}

bool SpeechSegmenter::TryPopSegment(Segment& segment)
{
    if (completedSegments.empty())
        return false;

    segment = std::move(completedSegments.front());

    completedSegments.pop_front();

    return true;
}

bool SpeechSegmenter::TryPopTranscription(Transcription& tran)
{
    if (transcriber == nullptr)
        return false;

    return transcriber->TryPop(tran);
}

size_t SpeechSegmenter::MillisecondsToFrames(uint32_t ms)
{
    return (static_cast<size_t>(ms) + FrameDurationMs - 1)
        / FrameDurationMs;
}

void SpeechSegmenter::SetPreRollMs(uint32_t ms)
{
    preRollFrameLimit = MillisecondsToFrames(ms);

    while (preRollFrames.size() > preRollFrameLimit)
        preRollFrames.pop_front();
}

void SpeechSegmenter::SetMinSpeechMs(uint32_t ms)
{
    minSpeechFrames = std::max<size_t>(
        1,
        MillisecondsToFrames(ms)
    );
}

void SpeechSegmenter::SetHangoverMs(uint32_t ms)
{
    hangoverFrames = MillisecondsToFrames(ms);
}

void SpeechSegmenter::SetStartTimeoutMs(uint32_t ms)
{
    startTimeoutFrames = std::max<size_t>(
        1,
        MillisecondsToFrames(ms)
    );
}

void SpeechSegmenter::Flush()
{
    // Accepta també Pending: una ordre curta ("next") pot no arribar a minSpeechFrames.
    if ((state == State::Speaking || state == State::Pending) && !currentSamples.empty())
        FinishSpeech();

    state = State::Idle;
    currentSamples.clear();
    preRollFrames.clear();
    filled = 0;                       // descarta el bloc parcial
    candidateVoicedFrames = candidateSilenceFrames = hangoverSilenceFrames = 0;
}
