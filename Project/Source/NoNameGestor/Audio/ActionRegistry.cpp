#include "ActionRegistry.h"

void ActionRegistry::Register(std::string&& name, std::function<Response(const CommandContext&)>&& handler)
{
    registry[name] = std::move(handler);
}

Response ActionRegistry::Respond(const Intent& intent, VoiceNode* node, ProgramState state, bool resumable)
{
    auto it = registry.find(intent.action);

    if (it == registry.end())
        return Response{ "Unrecognized command." };

    return it->second({ intent , node, state, resumable });
}
