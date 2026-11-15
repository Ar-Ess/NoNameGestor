#include "VoiceManager.h"
#include "NoNameGestor/External/imgui/imgui_internal.h"
#include "NoNameGestor/Utils/ImGuiExtension.h"
#include "NoNameGestor/Gestor/Configuration.h"
#include "Framework/Input/InputManager.h"
#include "Framework/Engine/App.h"

VoiceManager::VoiceManager(Configuration& config) :
	device(new CaptureDevice(sentences)),
	interpreter(new Interpreter),
	resolver(new ActionResolver),
	registry(new ActionRegistry),
	synth(new VoiceSynth),
	config(config)
{
	resolver->AddInstruction("describe_project", { "Describe the project", "What is the project state?" });
	resolver->AddInstruction("say_hello", { "Hello, how are you?" });

	registry->Register("describe_project", [](const Intent& intent) { return Response{ "This is my description of your project." }; });
	registry->Register("say_hello", [](const Intent& intent) { return Response{ "You are a fucking nerd." }; });
	
	config.onVoiceSpeakModeChange += Action<int>::Bind(this, &VoiceManager::SwitchListenMode);
}

VoiceManager::~VoiceManager()
{
	End();
	delete device;
	delete interpreter;
	delete resolver;
	delete registry;
	delete synth;
}

void VoiceManager::Start()
{
	if (active)
		return;

	device->Start();
	synth->Start();
	active = true;
}

void VoiceManager::Update()
{
	if (!active)
		return;

	device->SetGate(OpenMic());
	device->Update();
	
	std::string sentence;
	while (TryPopSentence(sentence))
	{
		Intent intent = interpreter->Process(sentence);
		if (!intent.IsValid())
			continue;

		resolver->Resolve(intent);
		Response response;
		if (!intent.IsUnderstood())
			response = Response{ "Sorry, I couldn't understand you." };
		else
			response = registry->Respond(intent);

		responses.emplace_back(response);
		synth->Generate(std::move(response));
	}

	synth->Speak();
}

void VoiceManager::Draw()
{
	if (!active)
		return;

	ImGui::RS::Spacing(2);
	ImGui::Text("DEBUG"); ImGui::SameLine();
	ImGui::RS::Separator(2);

	device->Draw();
	interpreter->Draw();

	ImGui::Text("Responses:");
	for (const Response& r : responses)
		ImGui::TextUnformatted(r.text.c_str());
}

void VoiceManager::End()
{
	if (!active)
		return;

	device->Stop();
	synth->Stop();
	active = false;
	sentences.clear();
	responses.clear();
}

bool VoiceManager::TryPopSentence(std::string& sentence)
{
	if (sentences.empty())
		return false;

	sentence = std::move(sentences.front());
	sentences.pop_front();

	return true;
}

void VoiceManager::SwitchListenMode(int mode)
{
	device->SetGate(mode == 1);
}

bool VoiceManager::OpenMic()
{
	bool ret = false;
	switch (config.voiceSpeakMode)
	{
	case 0:
		if (config.holdToSpeak)
			ret = config.holdKeybind == KeyCode::UNKNOWN ? Input::AnyKey() : Input::GetKey(config.holdKeybind);
		else
		{
			ret = micOpenSwitch;
			if (config.holdKeybind == KeyCode::UNKNOWN)
			{
				if (Input::AnyKeyUp())
					ret = micOpenSwitch = !micOpenSwitch;
			}
			else if (Input::GetKeyUp(config.holdKeybind))
				ret = micOpenSwitch = !micOpenSwitch;
		}
		break;

	case 1:
		ret = true;
		break;

	case 2:
		// Detect the activation word
		break;
	}

	return ret;
}
