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

	AddCommand("describe_project_brief", { "Briefly describe the project", "Shortly describe the project", "Summarize the project" },
		describe(DetailLevel::BRIEF));

	AddCommand("describe_project_normal", { "Describe the project" },
		describe(DetailLevel::NORMAL));

	AddCommand("describe_project_full", { "Fully describe the project", "Describe the project with detail" },
		describe(DetailLevel::FULL));

	AddCommand("continue", { "Continue", "Resume" },
		[this](const CommandContext& ctx)
		{
			if (!ctx.resumable)
				return Response{ "There is nothing to continue." };

			if (!ctx.node)
				return Response{ "There is no project open." };

			context->reading = true;
			return Response{ "Okay. " + ctx.node->Describe(context->readDetail)};
		});

	AddCommand("stop", { "Stop", "Wait", "That's enough", "Hold on" },
		[this](const CommandContext& c)
		{
			context->resumable = (c.state == ProgramState::RUNNING);
			return Response{ c.state == ProgramState::IDLE ? "Nothing is running." : "Okay." };
		});

	AddCommand("locate_current", { "Where are you?", "Where am I?", "Where are we?" },
		[this](const CommandContext& ctx)
		{
			if (!ctx.node)
				return Response{ "There is no project open." };

			return Response{ ctx.node->Locate(context->readDetail) };
		});

	AddCommand("describe_current", { "Describe current", "Describe" },
		[this](const CommandContext& ctx)
		{
			if (!ctx.node)
				return Response{ "There is no project open." };

			return Response{ ctx.node->Describe(context->readDetail) };
		});

	AddCommand("next", { "Next", "Go next", "Advance one", "Skip to the next one" },
		[this](const CommandContext&) { return MoveAndLocate(Advance(), "That is the end."); });

	AddCommand("previous", { "Previous", "Go back one", "Go to the previous one" },
		[this](const CommandContext&) { return MoveAndLocate(Retreat(), "You are at the start."); });

	AddCommand("go_start", { "Go to the start", "Reset", "Go to the root" },
		[this](const CommandContext&)
		{
			if (root) context->Path({ root->Id().Data() });
			return MoveAndLocate(root != nullptr, "");
		});

	AddCommand("go_up", { "Go up", "Go to the parent" },
		[this](const CommandContext&)
		{
			auto chain = ResolveChain();
			const bool moved = chain.size() > 1;
			if (moved) context->Truncate(chain.size() - 1);
			return MoveAndLocate(moved, "You are at the top.");
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
	delete context;
	delete awake;
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

		const ProgramState state = GetState();

		if (config.voiceSpeakMode == 2 && !PassesWakeWord(intent, state))
			continue;

		resolver->Resolve(intent);

		if (intent.IsUnderstood())
			HandleKnown(intent, state);
		else
			HandleUnknown(state);
	}

	if (context->reading && !synth->IsBusy())
	{
		VoiceNode* n = Advance(context->scopeId) ? ResolveCurrent() : nullptr;
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

bool VoiceManager::Cancel()
{
	//TODO: Review later, add if context is waiting or not.
	const bool wasActive = context->reading || synth->IsBusy() /* || context->waiting, later */;
	context->reading = false;
	synth->Clear();
	return wasActive;
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
	context->Clear();
	context->scopeId = node ? node->Id().Data() : 0;
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

bool VoiceManager::Advance(uint64_t scope)
{
	auto chain = ResolveChain();
	if (chain.empty()) return false;

	// 1. First child, if any
	auto kids = chain.back()->Children();
	if (!kids.empty()) { context->Push(kids[0]->Id().Data()); return true; }

	// 2. Otherwise next sibling, going up as many levels as needed
	for (size_t i = chain.size() - 1; i > 0; --i)
	{
		if (chain[i]->Id().Data() == scope)
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

bool VoiceManager::Retreat()
{
	auto chain = ResolveChain();
	if (chain.size() < 2)
		return false;                                    // at the root

	auto siblings = chain[chain.size() - 2]->Children();
	size_t idx = 0;
	while (idx < siblings.size() && siblings[idx]->Id() != chain.back()->Id())
		++idx;

	context->Truncate(chain.size() - 1);                 // go up to the parent

	if (idx == 0 || idx >= siblings.size())
		return true;                                     // first child: the parent is the previous node

	VoiceNode* n = siblings[idx - 1];                    // previous sibling...
	context->Push(n->Id().Data());
	for (auto kids = n->Children(); !kids.empty(); kids = n->Children())
	{
		n = kids.back();                                 // ...then its last descendant
		context->Push(n->Id().Data());
	}
	return true;
}

Response VoiceManager::MoveAndLocate(bool moved, const char* edgeMessage)
{
	if (!root)
		return Response{ "There is no project open." };

	if (!moved)
		return Response{ edgeMessage };

	VoiceNode* n = ResolveCurrent(); // c.node is stale after a move
	return Response{ n ? n->Locate(DetailLevel::BRIEF) : "There is no project open." };
}

bool VoiceManager::PassesWakeWord(Intent& intent, ProgramState state)
{
	switch (awake->Filter(intent.normalized))
	{
	case AwakeDetector::Result::IGNORED:
		return false;

	case AwakeDetector::Result::WOKE:
		if (state == ProgramState::IDLE) Respond("Tell me!"); // chart: ignored while busy
		return false;

	case AwakeDetector::Result::WOKE_WITH_LEFTOVER:
		if (state == ProgramState::IDLE)
			Respond("Sorry, I didn't catch that. Say the command after my name.");
		return false;

	case AwakeDetector::Result::COMMAND:
		return true;
	}
	return false;
}

void VoiceManager::HandleKnown(const Intent& intent, ProgramState state)
{
	// WAITING (later): first try the question's answer parser; if it fits, consume it
	// as the answer and return. Otherwise fall through: the question is abandoned.
	const bool wasResumable = context->resumable;   // sampled before anything clears it
	context->resumable = false;
	Cancel();
	Respond(registry->Respond(intent, ResolveCurrent(), state, wasResumable));
}

void VoiceManager::HandleUnknown(ProgramState state)
{
	switch (state)
	{
	case ProgramState::IDLE:
		Respond("Sorry, I couldn't understand you.");
		break;

	case ProgramState::RUNNING:
	case ProgramState::MID_SENTENCE:
		break;   // ignored on purpose; goes to the conversation log later

	case ProgramState::WAITING:
		// TODO: re-ask the question + "Say stop to cancel."
		break;
	}
}

ProgramState VoiceManager::GetState() const
{
	if (context->waiting)   return ProgramState::WAITING;
	if (context->reading)   return ProgramState::RUNNING;
	if (synth->IsBusy())    return ProgramState::MID_SENTENCE;
	return ProgramState::IDLE;
}
