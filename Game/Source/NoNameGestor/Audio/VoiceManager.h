#pragma once

#include "NoNameGestor/Audio/CaptureDevice.h"
#include "NoNameGestor/Audio/ActionResolver.h"
#include "NoNameGestor/Audio/ActionRegistry.h"
#include "NoNameGestor/Audio/VoiceSynth.h"

class VoiceManager
{
public:

	VoiceManager();
	~VoiceManager();

	void Update();
	void Draw();

private:

	bool TryPopSentence(std::string& sentence);

private:

	std::deque<std::string> sentences;
	std::deque<Response> responses;

	CaptureDevice* device = nullptr;
	Interpreter* interpreter = nullptr;
	ActionResolver* resolver = nullptr;
	ActionRegistry* registry = nullptr;
	VoiceSynth* synth = nullptr;

};

