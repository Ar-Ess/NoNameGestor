#pragma once

#include "NoNameGestor/Audio/ProgramState.h"
#include "NoNameGestor/Audio/Response.h"
#include "NoNameGestor/Audio/Intent.h"
#include "NoNameGestor/Audio/CommandContext.h"
#include <functional>
#include <map>

class ActionRegistry
{
public:

	void Register(std::string&& name, std::function<Response(const CommandContext&)>&& handler);
	Response Respond(const Intent& intent, VoiceNode* node, ProgramState state, bool resumable);

private:

	std::map<std::string, std::function<Response(const CommandContext&)>> registry;

};

