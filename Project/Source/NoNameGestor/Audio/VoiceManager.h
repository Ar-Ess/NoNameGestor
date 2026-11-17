#pragma once

#include "NoNameGestor/Audio/CaptureDevice.h"
#include "NoNameGestor/Audio/ActionResolver.h"
#include "NoNameGestor/Audio/ActionRegistry.h"
#include "NoNameGestor/Audio/VoiceSynth.h"
#include "NoNameGestor/Audio/AwakeDetector.h"
#include "NoNameGestor/Audio/VoiceNode.h"
#include "NoNameGestor/Audio/DialogueContext.h"

struct Configuration;

class VoiceManager
{

	friend class VoiceNode;

public:

	VoiceManager(Configuration& config);
	~VoiceManager();

	void Start();
	void Update();
	void Draw();
	void End();

	void AddCommand(std::string commandId, std::vector<std::string> activationCalls, std::function<Response(const CommandContext&)> handler);
	void SetRoot(VoiceNode* node);

private:

	bool TryPopSentence(std::string& sentence);
	void SwitchListenMode(int mode);
	bool OpenMicLogic();
	void Respond(Response&& response);
	void Respond(const char* response);
	std::string DescribeTree(VoiceNode* node, DetailLevel level);

	bool FindPath(VoiceNode* from, uint64_t id, std::vector<VoiceNode*>& path);
	VoiceNode* ResolveCurrent();

private:

	std::deque<std::string> sentences;
	std::deque<Response> responses;

	CaptureDevice* device = nullptr;
	Interpreter* interpreter = nullptr;
	ActionResolver* resolver = nullptr;
	ActionRegistry* registry = nullptr;
	VoiceSynth* synth = nullptr;
	AwakeDetector* awake = nullptr;
	DialogueContext* context = nullptr;
	VoiceNode* root = nullptr;

	bool active = false;

	Configuration& config;

	bool micOpenSwitch = false;

};

