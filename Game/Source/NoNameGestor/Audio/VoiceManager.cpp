#include "VoiceManager.h"
#include "NoNameGestor/External/imgui/imgui_internal.h"

VoiceManager::VoiceManager() :
	device(new CaptureDevice(sentences)), interpreter(new Interpreter), resolver(new ActionResolver), registry(new ActionRegistry), synth(new VoiceSynth)
{
	resolver->AddInstruction("describe_project", { "Describe the project", "What is the project state?" });
	registry->Register("describe_project", [](const Intent& intent) { return Response{ "This is my description of your project." }; });
}

VoiceManager::~VoiceManager()
{
	delete device;
	delete interpreter;
	delete resolver;
	delete registry;
	delete synth;
}

void VoiceManager::Update()
{
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
	device->Draw();
	interpreter->Draw();

	ImGui::Text("Responses:");
	for (const Response& r : responses)
		ImGui::TextUnformatted(r.text.c_str());
}

bool VoiceManager::TryPopSentence(std::string& sentence)
{
	if (sentences.empty())
		return false;

	sentence = std::move(sentences.front());
	sentences.pop_front();

	return true;
}
