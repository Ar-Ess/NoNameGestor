#pragma once

#include "NoNameGestor/Audio/Response.h"
#include <vector>
#include <deque>
#include <mutex>

struct ma_device;
template<class In, class Out> class ThreadWorker;

class VoiceSynth
{
public:

	~VoiceSynth();

	void Start();
	void Stop();

	void Generate(Response&& response);
	void Speak();
	void Fill(float* out, size_t frames);

private:

	std::deque<std::vector<float>> messages;
	ThreadWorker<Response, std::vector<float>>* synthesizer = nullptr;

	ma_device* device = nullptr;

	bool deviceInitialized = false;

	std::mutex mtx;
	std::vector<float> current;
	size_t pos = 0;

};

