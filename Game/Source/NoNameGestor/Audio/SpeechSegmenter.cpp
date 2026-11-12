#include "SpeechSegmenter.h"

#include <algorithm>
#include <cmath>
#include <utility>

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
}

SpeechSegmenter::~SpeechSegmenter()
{
    if (inst != nullptr)
    {
        fvad_free(inst);
        inst = nullptr;
    }
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
        // Sempre guardem el frame, també si és silenci.
        currentSamples.insert(
            currentSamples.end(),
            floatFrame.begin(),
            floatFrame.end()
        );

        if (voiced)
        {
            // La veu ha tornat: reiniciem el hangover.
            hangoverSilenceFrames = 0;
        }
        else
        {
            ++hangoverSilenceFrames;

            if (hangoverSilenceFrames >= hangoverFrames)
            {
                FinishSpeech();
            }
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
