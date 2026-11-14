#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <deque>
#include <vector>
#include <string>
#include <thread>
#include "NoNameGestor/External/libfvad/include/fvad.h"

struct whisper_context;
template<class In, class Out> class ThreadWorker;

class SpeechSegmenter
{
public:

    struct Segment
    {
        std::vector<float> samples;

        uint32_t sampleRate = 16000;

        double GetDurationSeconds() const
        {
            if (sampleRate == 0)
                return 0.0;

            return static_cast<double>(samples.size()) / sampleRate;
        }
    };

    struct Transcription
    {
        std::string text;
        double ms = 0.0;
    };

private:

    enum class State
    {
        Idle,       // No hem detectat cap possible frase.
        Pending,    // Hem detectat veu, però encara no està confirmada.
        Speaking    // La frase està confirmada.
    };

public:

    SpeechSegmenter();
    ~SpeechSegmenter();

    void Start();
    void Stop();
    void Process(const float* samples, size_t n);
    void Transcribe(Segment&& segment);

    // Estat públic.
    bool IsSpeaking() const { return state == State::Speaking; }
    bool IsPending() const { return state == State::Pending; }

    bool IsValid() const { return inst != nullptr; }

    // Recupera una frase quan ha acabat.
    bool TryPopSegment(Segment& segment);
    bool TryPopTranscription(Transcription& transcription);

    // Configuració del segmentador.
    void SetPreRollMs(uint32_t ms);
    void SetMinSpeechMs(uint32_t ms);
    void SetHangoverMs(uint32_t ms);
    void SetStartTimeoutMs(uint32_t ms);

    // Informació de diagnòstic.
    size_t GetCompletedSegmentCount() const
    {
        return completedSegments.size();
    }

    size_t GetTotalSegmentsCompleted() const
    {
        return totalSegmentsCompleted;
    }

    double GetLastSegmentDurationSeconds() const
    {
        return lastSegmentDurationSeconds;
    }

private:

    // ===== Processament intern =====

    void ProcessFrame();

    bool DetectVoice();

    void StartCandidate();

    void ConfirmSpeech();

    void FinishSpeech();

    void PushPreRoll();

    static size_t MillisecondsToFrames(uint32_t ms);

private:

    // Configuració d'àudio.
    static constexpr uint32_t SampleRate = 16000;
    static constexpr size_t FrameSamples = 320;
    static constexpr uint32_t FrameDurationMs = 20;
    static constexpr uint32_t MaxSpeechFrames = 750;
    static constexpr uint32_t TailKeepFrames = 8;

    // ===== VAD intern =====

    Fvad* inst = nullptr;

    std::array<int16_t, FrameSamples> pcmFrame = {};


    // ===== Acumulació de frames =====

    std::array<float, FrameSamples> floatFrame = {};
    size_t filled = 0;

    // ===== Segmentació =====

    State state = State::Idle;

    // Historial d'àudio anterior a l'inici de veu.
    std::deque<std::array<float, FrameSamples>> preRollFrames;

    // Àudio de la frase en construcció.
    std::vector<float> currentSamples;

    // Frases acabades pendents de recuperar.
    std::deque<Segment> completedSegments;


    // ===== Comptadors =====

    size_t candidateVoicedFrames = 0;
    size_t candidateSilenceFrames = 0;
    size_t hangoverSilenceFrames = 0;


    // ===== Configuració temporal =====

    size_t preRollFrameLimit = 10;          // 200 ms
    size_t minSpeechFrames = 13;            // 260 ms
    size_t startTimeoutFrames = 15;         // 300 ms
    size_t hangoverFrames = 30;             // 600 ms

    // ===== Diagnostic =====
    size_t totalSegmentsCompleted = 0;
    double lastSegmentDurationSeconds = 0.0;

    // ===== Transcribe =====
    whisper_context* ctx = nullptr;
    ThreadWorker<Segment, Transcription>* transcriber = nullptr;
};
