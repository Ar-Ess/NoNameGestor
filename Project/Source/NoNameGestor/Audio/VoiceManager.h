#pragma once

#include "NoNameGestor/Audio/CaptureDevice.h"
#include "NoNameGestor/Audio/ActionResolver.h"
#include "NoNameGestor/Audio/ActionRegistry.h"
#include "NoNameGestor/Audio/VoiceSynth.h"
#include "NoNameGestor/Audio/AwakeDetector.h"

struct Configuration;

class VoiceManager
{
public:

	VoiceManager(Configuration& config);
	~VoiceManager();

	void Start();
	void Update();
	void Draw();
	void End();

private:

	bool TryPopSentence(std::string& sentence);
	void SwitchListenMode(int mode);
	bool OpenMicLogic();

private:

	std::deque<std::string> sentences;
	std::deque<Response> responses;

	CaptureDevice* device = nullptr;
	Interpreter* interpreter = nullptr;
	ActionResolver* resolver = nullptr;
	ActionRegistry* registry = nullptr;
	VoiceSynth* synth = nullptr;
	AwakeDetector* awake = nullptr;

	bool active = false;

	Configuration& config;

	bool micOpenSwitch = false;

};

