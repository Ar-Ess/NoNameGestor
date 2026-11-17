#pragma once

#include "NoNameGestor/Audio/Response.h"
#include "NoNameGestor/Audio/Intent.h"
#include "NoNameGestor/Audio/CommandContext.h"
#include <functional>
#include <map>

class ActionRegistry
{
public:

	void Register(std::string&& name, std::function<Response(const CommandContext&)>&& handler);
	Response Respond(const Intent& intent, VoiceNode* node);

private:

	std::map<std::string, std::function<Response(const CommandContext&)>> registry;

};

