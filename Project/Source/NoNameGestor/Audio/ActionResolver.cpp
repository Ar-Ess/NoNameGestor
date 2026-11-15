#include "ActionResolver.h"

void ActionResolver::AddInstruction(const std::string& action, const std::vector<std::string>& phrases)
{
    for (const std::string& p : phrases)
    {
        std::string normalized = Interpreter::Normalize(p);

        if (!Interpreter::IsValid(normalized))
            continue;

        actions[normalized] = action;
    }
}

void ActionResolver::Resolve(Intent& intent)
{
    auto it = actions.find(intent.normalized);

    if (it != actions.end())
        intent.action = it->second;
}
