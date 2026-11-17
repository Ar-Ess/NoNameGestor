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
	awake(new AwakeDetector(config)),
	context(new DialogueContext),
	config(config)
{
	AddCommand(
		"describe_project_brief",
		{ "Briefly describe the project", "Shortly describe the project", "Summarize the project"},
		[this](const CommandContext& ctx)
		{
			if (!ctx.node)
				return Response{ "There is no project open." };

			return Response{ DescribeTree(root, DetailLevel::BRIEF) };
		}
	);

	AddCommand(
		"describe_project_normal",
		{ "Describe the project" },
		[this](const CommandContext& ctx)
		{
			if (!ctx.node)
				return Response{ "There is no project open." };

			return Response{ DescribeTree(root, DetailLevel::NORMAL) };
		}
	);

	AddCommand(
		"describe_project_full",
		{ "Fully describe the project", "Describe the project with detail"},
		[this](const CommandContext& ctx)
		{
			if (!ctx.node)
				return Response{ "There is no project open." };

			return Response{ DescribeTree(root, DetailLevel::FULL) };
		}
	);

	// TODO: Això utilitza directament config. No pot ser això, ja que necesitem que sigui algo independent de NoNameGestor!!
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

	device->SetGate(OpenMicLogic());
	device->Update();
	
	std::string sentence;
	while (TryPopSentence(sentence))
	{
		Intent intent = interpreter->Process(sentence);
		if (!intent.IsValid())
			continue;

		if (config.voiceSpeakMode == 2)
		{
			switch (awake->Filter(intent.normalized))
			{
			case AwakeDetector::Result::IGNORED:
				continue;

			case AwakeDetector::Result::WOKE:
				Respond("Tell me!");
				continue;

			case AwakeDetector::Result::WOKE_WITH_LEFTOVER:
				Respond("Sorry, I didn't catch that. Say the command after my name.");
				continue;

			case AwakeDetector::Result::COMMAND:
				break;
			}
		}

		resolver->Resolve(intent);

		Respond(intent.IsUnderstood() ?
			registry->Respond(intent, ResolveCurrent()) :
			Response{ "Sorry, I couldn't understand you." }
		);
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

void VoiceManager::AddCommand(std::string commandId, std::vector<std::string> activationCalls, std::function<Response(const CommandContext&)> handler)
{
	resolver->AddInstruction(commandId, activationCalls);
	registry->Register(std::move(commandId), std::move(handler));
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

bool VoiceManager::OpenMicLogic()
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
	case 2:
		ret = true;
		break;
	}

	return ret;
}

void VoiceManager::Respond(Response&& response)
{
	responses.emplace_back(response);
	synth->Generate(std::move(response));
}

void VoiceManager::Respond(const char* response)
{
	Respond(Response{ response });
}

bool VoiceManager::FindPath(VoiceNode* from, uint64_t id, std::vector<VoiceNode*>& path)
{
	path.push_back(from);
	if (from->Id().Data() == id)
		return true;

	for (VoiceNode* c : from->Children())
		if (FindPath(c, id, path))
			return true;

	path.pop_back();
	return false;
}

void VoiceManager::SetRoot(VoiceNode* node)
{
	root = node;
}

VoiceNode* VoiceManager::ResolveCurrent()
{
	// Si no hi ha root, no es pot resoldre res.
	if (root == nullptr)
		return nullptr;

	// Agafar la referència del path
	auto& path = context->Path();

	// Si el path és buit o no té com a node inicial la root, és un path invàlid
	// Per tant, s'ha d'eliminar i tornar a construir.
	if (path.empty() || path[0] != root->Id().Data())
	{
		context->Path({ root->Id().Data() });
		return root;
	}

	// Variables necesàries
	VoiceNode* node = root;
	size_t i = 1;

	// Iterar cada path
	for (; i < path.size(); ++i)
	{
		uint64_t id = path[i];
		bool found = false;

		// Mirar els children the cada node, i buscar si trobem el següent child
		for (VoiceNode* n : node->Children())
		{
			// Si entra aquí és que l'hem trobat!
			if (n->Id().Data() == id)
			{
				found = true;
				node = n;
				break;
			}
		}

		// Si no l'hem trobat, ens quedem amb l'últim node existent.
		if (!found) break;
	}

	// Si ha sigut el cas de que no s'ha trobat un node, hem de retallar el path fins a aquell node.
	if (i < path.size())
	{
		auto p = path;
		p.resize(i);
		context->Path(std::move(p));
	}

	return node;
}

std::string VoiceManager::DescribeTree(VoiceNode* node, DetailLevel level)
{
	std::string out = node->Describe(level);

	const auto& children = node->Children();

	if (children.empty())
		return out;

	for (VoiceNode* child : children)
		out += " " + DescribeTree(child, level);

	return out;
}
