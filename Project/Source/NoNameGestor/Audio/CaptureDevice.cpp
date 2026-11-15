#include "CaptureDevice.h"

#include "NoNameGestor/External/Miniaudio/miniaudio.h"
#include "NoNameGestor/External/imgui/imgui_internal.h"
#include "Framework/Coroutine/CoroutineManager.h"
#include <algorithm>
#include <cmath>

static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_pcm_rb* rb = (ma_pcm_rb*)pDevice->pUserData;
    const float* in = (const float*)pInput;
    ma_uint32 remaining = frameCount;

    while (remaining > 0)
    {
        void* pWrite = nullptr;
        ma_uint32 chunk = remaining;

        if (ma_pcm_rb_acquire_write(rb, &chunk, &pWrite) != MA_SUCCESS || chunk == 0)
            break;  // buffer ple: la resta es descarta

        ma_copy_pcm_frames(pWrite, in, chunk, ma_format_f32, 1);
        ma_pcm_rb_commit_write(rb, chunk);

        in += chunk;        // mono: 1 float per frame
        remaining -= chunk;
    }

    // Si remaining > 0 aquí, hem perdut mostres.
    (void)pOutput;
}

CaptureDevice::CaptureDevice(std::deque<std::string>& sentences) :
    sentences(sentences), scratch(4096)
{ }

CaptureDevice::~CaptureDevice()
{
    Stop();
}

bool CaptureDevice::Start()
{
    const ma_uint32 samples = 16000;
    const ma_uint32 duration = 10;

    ma_pcm_rb* rb = new ma_pcm_rb;
    if (ma_pcm_rb_init(ma_format_f32, 1, samples * duration, NULL, NULL, rb) != MA_SUCCESS)
        return false; // Error

    ringBufferInitialized = true;

    device = new ma_device;
    ma_device_config dConfig = ma_device_config_init(ma_device_type_capture);

    dConfig.capture.format = rb->format;
    dConfig.capture.channels = rb->channels;
    dConfig.sampleRate = samples;
    dConfig.dataCallback = data_callback;
    dConfig.pUserData = rb;

    ringBuffer = rb;

    if (ma_device_init(NULL, &dConfig, device) != MA_SUCCESS)
        return false;  // Failed to initialize the device.

    deviceInitialized = true;

    if (ma_device_start(device) != MA_SUCCESS)
        return false; // Failed to start device.

    segmenter = new SpeechSegmenter;

    return true;
}

void CaptureDevice::Stop()
{
    if (segmenter != nullptr)
    {
        delete segmenter;
        segmenter = nullptr;
    }

    if (device != nullptr)
    {
        if (deviceInitialized)
        {
            ma_device_uninit(device);
            deviceInitialized = false;
        }

        delete device;
        device = nullptr;
    }

    if (ringBufferInitialized)
    {
        ma_pcm_rb_uninit((ma_pcm_rb*)ringBuffer);
        delete (ma_pcm_rb*)ringBuffer;
        ringBufferInitialized = false;
    }

}

// Copia fins a maxFrames mostres a dst. Retorna quantes n'ha copiat.
size_t CaptureDevice::Read(float* dst, size_t maxFrames)
{
    size_t total = 0;

    while (total < maxFrames)
    {
        ma_uint32 chunk = (ma_uint32)(maxFrames - total);
        void* pRead = nullptr;

        if (ma_pcm_rb_acquire_read((ma_pcm_rb*)ringBuffer, &chunk, &pRead) != MA_SUCCESS || chunk == 0)
            break;  // no queda res per llegir

        ma_copy_pcm_frames(dst + total, pRead, chunk, ma_format_f32, 1);
        ma_pcm_rb_commit_read((ma_pcm_rb*)ringBuffer, chunk);

        total += chunk;
    }
    return total;
}

void CaptureDevice::Update()
{
    if (!deviceInitialized)
        return;

    double sum = 0.0;
    size_t count = 0;

    size_t n;
    float* data = scratch.data();
    while ((n = Read(scratch.data(), 4096)) > 0)
    {
        // Alimentem el detector de veu.
        if (openGate) segmenter->Process(data, n);

        for (size_t i = 0; i < n; ++i)
            sum += data[i] * data[i];

        count += n;
    }

    SpeechSegmenter::Segment seg;
    while (segmenter->TryPopSegment(seg))
    {
        segmenter->Transcribe(std::move(seg));
    }

    SpeechSegmenter::Transcription tran;
    while (segmenter->TryPopTranscription(tran))
    {
        lastText = tran.text;
        lastTranscribeMs = tran.ms;
        sentences.push_back(tran.text);
    }

    if (count > 0)
        level = (float)std::sqrt(sum / count);   // membre float, el que dibuixes a ImGui
}

void CaptureDevice::Draw()
{
    // Nivell d'àudio.
    ImGui::BeginDisabled(!openGate);
    ImGui::ProgressBar(std::clamp(level * 3.f, 0.f, 1.f), ImVec2(200, 20), "Input RMS");
    ImGui::EndDisabled();

    ImGui::SameLine();

    // Estat del segmentador.
    if (!segmenter->IsValid())
        ImGui::TextColored(ImVec4(1.f, 0.2f, 0.2f, 1.f), "VAD: Error");
    else if (segmenter->IsSpeaking())
        ImGui::TextColored(ImVec4(0.2f, 1.f, 0.2f, 1.f), "Speaking");
    else if (segmenter->IsPending())
        ImGui::TextColored(ImVec4(1.f, 0.8f, 0.2f, 1.f), "Pending...");
    else
        ImGui::TextDisabled("Silence");

    ImGui::Separator();

    ImGui::Text("Last Text (%.0f ms): \"%s\"", lastTranscribeMs, lastText.c_str());

    // Estadístiques de segmentació.
    ImGui::Text("Completed segments: %llu", static_cast<unsigned long long>(segmenter->GetTotalSegmentsCompleted()));

    ImGui::Text("Segments waiting: %llu", static_cast<unsigned long long>(segmenter->GetCompletedSegmentCount()));

    if (segmenter->GetTotalSegmentsCompleted() > 0)
        ImGui::Text("Last segment duration: %.2f seconds", segmenter->GetLastSegmentDurationSeconds());
    else
        ImGui::TextDisabled("Last segment duration: --");
}

static Coroutine FlushAfterSeconds(SpeechSegmenter* segmenter, float seconds)
{
    co_yield Yield::WaitForSeconds(seconds);
    segmenter->Flush();
}

void CaptureDevice::SetGate(bool openGate)
{
    if (segmenter != nullptr && this->openGate && !openGate)
        CoroutineManager::Begin(FlushAfterSeconds(segmenter, 0.3f));
    this->openGate = openGate;
}
