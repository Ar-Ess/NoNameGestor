#pragma once

#include "NoNameGestor/Audio/SpeechSegmenter.h"

struct ma_device;

class CaptureDevice
{
public:

	CaptureDevice(std::deque<std::string>& sentences);
	~CaptureDevice();

	bool Start();
	void Stop();

	size_t Read(float* dst, size_t maxFrames);
	void Update();
	void Draw();

private:

	ma_device* device = nullptr;
	void* ringBuffer = nullptr;

	float level = 0;

	std::vector<float> scratch;

	SpeechSegmenter* segmenter = nullptr;

	bool ringBufferInitialized = false;
	bool deviceInitialized = false;

	double lastTranscribeMs = 0.0;
	std::string lastText;

	std::deque<std::string>& sentences;

};

