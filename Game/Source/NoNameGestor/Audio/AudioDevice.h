#pragma once

#include <functional>

class AudioDevice
{
public:

    virtual ~AudioDevice() = default;
    virtual bool Start() = 0;
    virtual void Stop() = 0;

    // El consumidor rep blocs de mostres mono 16 kHz float
    virtual void SetCallback(std::function<void(const float*, size_t)> cb) = 0;

};

