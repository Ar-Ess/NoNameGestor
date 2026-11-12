#include "CaptureDevice.h"

#define MINIAUDIO_IMPLEMENTATION
#include "NoNameGestor/External/Miniaudio/miniaudio.h"

#include "NoNameGestor/External/imgui/imgui_internal.h"

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
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

ma_pcm_rb ringBuffer;

CaptureDevice::CaptureDevice()
{
    const ma_uint32 samples = 16000;
    const ma_uint32 duration = 10;

    if (ma_pcm_rb_init(ma_format_f32, 1, samples * duration, NULL, NULL, &ringBuffer) != MA_SUCCESS)
        return; // Error

    device = new ma_device;
    ma_device_config dConfig = ma_device_config_init(ma_device_type_capture);

    dConfig.capture.format = ringBuffer.format;
    dConfig.capture.channels = ringBuffer.channels;
    dConfig.sampleRate = samples;
    dConfig.dataCallback = data_callback;
    dConfig.pUserData = &ringBuffer;

    if (ma_device_init(NULL, &dConfig, device) != MA_SUCCESS)
        return;  // Failed to initialize the device.

    if (ma_device_start(device) != MA_SUCCESS)
        return; // Failed to start device.

    scratch.resize(4096);
}

CaptureDevice::~CaptureDevice()
{
    if (device != nullptr)
    {
        ma_device_uninit(device);
        delete device;
    }

    ma_pcm_rb_uninit(&ringBuffer);
}

// Copia fins a maxFrames mostres a dst. Retorna quantes n'ha copiat.
size_t CaptureDevice::Read(float* dst, size_t maxFrames)
{
    size_t total = 0;

    while (total < maxFrames)
    {
        ma_uint32 chunk = (ma_uint32)(maxFrames - total);
        void* pRead = nullptr;

        if (ma_pcm_rb_acquire_read(&ringBuffer, &chunk, &pRead) != MA_SUCCESS || chunk == 0)
            break;  // no queda res per llegir

        ma_copy_pcm_frames(dst + total, pRead, chunk, ma_format_f32, 1);
        ma_pcm_rb_commit_read(&ringBuffer, chunk);

        total += chunk;
    }
    return total;
}

void CaptureDevice::Update()
{
    double sum = 0.0;
    size_t count = 0;

    size_t n;
    float* data = scratch.data();
    while ((n = Read(scratch.data(), 4096)) > 0)
    {
        for (size_t i = 0; i < n; ++i)
            sum += data[i] * data[i];
        count += n;
    }

    if (count > 0)
        level = (float)std::sqrt(sum / count);   // membre float, el que dibuixes a ImGui
}

void CaptureDevice::Draw()
{
    ImGui::ProgressBar(level * 3.f, ImVec2(200, 20));
}

