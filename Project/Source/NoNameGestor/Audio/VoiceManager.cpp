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
	auto describe = [this](DetailLevel d) { return [this, d](const CommandContext&) { return StartReading(d); }; };

	AddCommand("describe_project_brief",
		{ "Briefly describe the project", "Shortly describe the project", "Summarize the project" },
		describe(DetailLevel::BRIEF));

	AddCommand("describe_project_normal",
		{ "Describe the project" },
		describe(DetailLevel::NORMAL));

	AddCommand("describe_project_full",
		{ "Fully describe the project", "Describe the project with detail" },
		describe(DetailLevel::FULL));

	AddCommand("continue", { "Continue" },
		[this](const CommandContext& ctx)
		{
			if (!ctx.node)
				return Response{ "There is no project open." };

			context->reading = true;
			return Response{ "Okay. " + ctx.node->Describe(context->readDetail)};
		});

	AddCommand("stop", { "Stop", "Wait", "That's enough", "Hold on" },
		[this](const CommandContext&)
		{
			Cancel();
			return Response{ "Okay." };
		});

	AddCommand("locate_current", { "Where are you?" },
		[this](const CommandContext& ctx)
		{
			if (!ctx.node)
				return Response{ "There is no project open." };

			return Response{ ctx.node->Locate(context->readDetail) };
		});

	AddCommand("describe_current", { "Describe current" },
		[this](const CommandContext& ctx)
		{
			if (!ctx.node)
				return Response{ "There is no project open." };

			return Response{ ctx.node->Describe(context->readDetail) };
		});

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

	const bool micOpen = OpenMicLogic();
	device->SetGate(micOpen);
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

	if (context->reading && !synth->IsBusy())
	{
		VoiceNode* n = Advance() ? ResolveCurrent() : nullptr;
		if (n) Respond(Response{ n->Describe(context->readDetail) });
		else { context->reading = false; Respond("That's all."); }
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

Response VoiceManager::StartReading(DetailLevel detail)
{
	if (root == nullptr)
		return Response{ "There is no project open." };

	synth->Clear(); // cut anything still being said

	context->Path({ root->Id().Data() }); // cursor = root
	context->scopeId = root->Id().Data(); // stop when we leave this subtree
	context->readDetail = detail;
	context->reading = true;

	return Response{ root->Describe(detail) }; // first node; the tick does the rest
}

void VoiceManager::Cancel()
{
	context->reading = false;
	synth->Clear();
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
	auto chain = ResolveChain();
	return chain.empty() ? nullptr : chain.back();
}

std::vector<VoiceNode*> VoiceManager::ResolveChain()
{
	// Si no hi ha root, no es pot resoldre res.
	if (root == nullptr)
		return {};

	// Agafar la referència del path
	auto& path = context->Path();

	// Si el path és buit o no té com a node inicial la root, és un path invàlid
	// Per tant, s'ha d'eliminar i tornar a construir.
	if (path.empty() || path[0] != root->Id().Data())
	{
		context->Path({ root->Id().Data() });
		return { root };
	}

	// Variables necesàries
	VoiceNode* node = root;
	std::vector<VoiceNode*> chain { root };
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
				chain.push_back(n);
				break;
			}
		}

		// Si no l'hem trobat, ens quedem amb l'últim node existent.
		if (!found) break;
	}

	// Si ha sigut el cas de que no s'ha trobat un node, hem de retallar el path fins a aquell node.
	if (i < path.size())
		context->Truncate(i);

	return chain;
}

bool VoiceManager::Advance()
{
	auto chain = ResolveChain();
	if (chain.empty()) return false;

	// 1. First child, if any
	auto kids = chain.back()->Children();
	if (!kids.empty()) { context->Push(kids[0]->Id().Data()); return true; }

	// 2. Otherwise next sibling, going up as many levels as needed
	for (size_t i = chain.size() - 1; i > 0; --i)
	{
		if (chain[i]->Id().Data() == context->scopeId)
			return false;                       // we'd leave the scope: done

		auto siblings = chain[i - 1]->Children();
		for (size_t s = 0; s + 1 < siblings.size(); ++s)
		{
			if (siblings[s]->Id() == chain[i]->Id())
			{
				context->Truncate(i);               // drop this node and below
				context->Push(siblings[s + 1]->Id().Data());
				return true;
			}
		}
	}
	return false;                                   // nothing left
}
