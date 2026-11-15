#pragma once

#include "NoNameGestor/Audio/Response.h"
#include "NoNameGestor/Audio/Intent.h"
#include <functional>

class ActionRegistry
{
public:

	void Register(std::string name, std::function<Response(const Intent&)> handler);
	Response Respond(const Intent& intent);

private:

	std::map<std::string, std::function<Response(const Intent&)>> registry;

};

