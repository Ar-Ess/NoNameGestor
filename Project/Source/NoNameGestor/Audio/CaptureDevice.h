#pragma once

#include "NoNameGestor/Audio/SpeechSegmenter.h"
#include "Framework/Coroutine/Coroutine.h"

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

	void SetGate(bool openGate);

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

	bool openGate = true;
	bool closing = false;
	std::chrono::steady_clock::time_point closeAt;

};

