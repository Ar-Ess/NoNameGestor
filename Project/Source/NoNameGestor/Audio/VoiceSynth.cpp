#include "VoiceSynth.h"

#include <sapi.h>
#include <string>
#include "NoNameGestor/Audio/ThreadWorker.h"
#include "NoNameGestor/External/Miniaudio/miniaudio.h"

namespace
{
    // S'inicialitza una vegada per fil: el constructor corre la primera vegada
    // que el fil treballador la fa servir, i el destructor quan el fil acaba.
    struct ComInit
    {
        HRESULT hr;
        ComInit() { hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED); }
        ~ComInit() { if (SUCCEEDED(hr)) CoUninitialize(); }
    };

    std::wstring Utf8ToWide(const std::string& s)
    {
        if (s.empty())
            return {};
        int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
        std::wstring w(n, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), w.data(), n);
        return w;
    }
}

static void data_callback(ma_device* pDevice, void* pOutput, const void*, ma_uint32 frameCount)
{
    static_cast<VoiceSynth*>(pDevice->pUserData)->Fill((float*)pOutput, frameCount);
}

VoiceSynth::~VoiceSynth()
{
    Stop();
}

void VoiceSynth::Start()
{
    if (device != nullptr)
        return;

    device = new ma_device;

    ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format = ma_format_f32;
    cfg.playback.channels = 1;
    cfg.sampleRate = 22050;
    cfg.dataCallback = data_callback;
    cfg.pUserData = this;

    if (ma_device_init(NULL, &cfg, device) != MA_SUCCESS)
    {
        delete device;
        device = nullptr;
        return;
    }

    deviceInitialized = true;

    synthesizer = new ThreadWorker<Response, std::vector<float>>(
        [](Response& response) -> std::vector<float>
        {
            // COM per fil: s'executa només la primera vegada en aquest fil.
            thread_local ComInit com;
            if (FAILED(com.hr))
                return {};

            constexpr DWORD sampleRate = 22050;
            const std::wstring text = Utf8ToWide(response.text);
            std::vector<float> out;

            ISpVoice* voice = nullptr;
            IStream* mem = nullptr;
            ISpStream* sp = nullptr;

            do
            {
                if (FAILED(CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL,
                    IID_ISpVoice, (void**)&voice)))
                    break;

                WAVEFORMATEX wfx = {};
                wfx.wFormatTag = WAVE_FORMAT_PCM;
                wfx.nChannels = 1;
                wfx.nSamplesPerSec = sampleRate;
                wfx.wBitsPerSample = 16;
                wfx.nBlockAlign = 2;
                wfx.nAvgBytesPerSec = sampleRate * 2;

                if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &mem)))
                    break;

                if (FAILED(CoCreateInstance(CLSID_SpStream, nullptr, CLSCTX_ALL,
                    IID_ISpStream, (void**)&sp)))
                    break;

                if (FAILED(sp->SetBaseStream(mem, SPDFID_WaveFormatEx, &wfx)))
                    break;

                if (FAILED(voice->SetOutput(sp, TRUE)))
                    break;

                if (FAILED(voice->Speak(text.c_str(), SPF_DEFAULT, nullptr)))
                    break;
                voice->WaitUntilDone(INFINITE);

                // Llegim el stream de memòria
                HGLOBAL h = nullptr;
                if (FAILED(GetHGlobalFromStream(mem, &h)))
                    break;

                STATSTG st = {};
                if (FAILED(mem->Stat(&st, STATFLAG_NONAME)))
                    break;
                const size_t bytes = (size_t)st.cbSize.QuadPart;

                if (void* p = GlobalLock(h))
                {
                    const int16_t* pcm = static_cast<const int16_t*>(p);
                    out.resize(bytes / 2);
                    for (size_t i = 0; i < out.size(); ++i)
                        out[i] = pcm[i] / 32768.0f;
                    GlobalUnlock(h);
                }
            } while (false);

            if (sp)    sp->Release();
            if (mem)   mem->Release();
            if (voice) voice->Release();
            return out;
        }
    );
}

void VoiceSynth::Stop()
{
    if (device != nullptr)
    {
        if (deviceInitialized)
            ma_device_uninit(device);

        delete device;
        device = nullptr;
    }

    delete synthesizer;
    synthesizer = nullptr;

    messages.clear();
    current.clear();

    deviceInitialized = false;
}

void VoiceSynth::Generate(Response&& response)
{
    if (synthesizer == nullptr || device == nullptr)
        return;

    synthesizer->Push(std::move(response));
}

void VoiceSynth::Speak()
{
    if (synthesizer == nullptr || device == nullptr)
        return;

    std::vector<float> audio;
    while (synthesizer->TryPop(audio))
    {
        if (audio.empty())
            continue;               // la síntesi ha fallat: no encuis res

        {
            std::lock_guard<std::mutex> l(mtx);
            messages.push_back(std::move(audio));
        }

        if (!ma_device_is_started(device))
            ma_device_start(device);
    }
}

void VoiceSynth::Fill(float* out, size_t frames)
{
    std::lock_guard<std::mutex> l(mtx);
    ma_uint32 written = 0;

    while (written < frames)
    {
        if (pos >= current.size())          // clip acabat: passem al següent
        {
            if (messages.empty())
                break;
            current = std::move(messages.front());
            messages.pop_front();
            pos = 0;
            continue;
        }

        size_t n = std::min<size_t>(frames - written, current.size() - pos);
        memcpy(out + written, current.data() + pos, n * sizeof(float));
        written += (ma_uint32)n;
        pos += n;
    }

    // El que no hem omplert, silenci.
    memset(out + written, 0, (frames - written) * sizeof(float));
}
