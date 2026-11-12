#pragma once

#include "NoNameGestor/Audio/SpeechSegmenter.h"
#include "NoNameGestor/Audio/AudioDevice.h"

struct ma_device;

class CaptureDevice : public AudioDevice
{
public:

	CaptureDevice();
	~CaptureDevice() override;

	bool Start() override { return true; }
	void Stop() override {}
	void SetCallback(std::function<void(const float*, size_t)> cb) override {}

	size_t Read(float* dst, size_t maxFrames);
	void Update();
	void Draw();

private:

	ma_device* device = nullptr;

	float level = 0;

	std::vector<float> scratch;

	SpeechSegmenter segmenter;

	bool ringBufferInitialized = false;
	bool deviceInitialized = false;

};

