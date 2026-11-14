#include "VoiceManager.h"
#include "NoNameGestor/External/imgui/imgui_internal.h"

VoiceManager::VoiceManager() :
	device(new CaptureDevice(sentences)),
	interpreter(new Interpreter),
	resolver(new ActionResolver),
	registry(new ActionRegistry),
	synth(new VoiceSynth)
{
	resolver->AddInstruction("describe_project", { "Describe the project", "What is the project state?" });
	registry->Register("describe_project", [](const Intent& intent) { return Response{ "This is my description of your project." }; });
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
