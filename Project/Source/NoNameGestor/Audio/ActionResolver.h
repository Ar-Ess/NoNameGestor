#pragma once

#include "NoNameGestor/Audio/Interpreter.h"

#include <vector>
#include <unordered_map>

class ActionResolver
{
public:

	void AddInstruction(const std::string& action, const std::vector<std::string>& phrases);
	void Resolve(Intent& intent);

private:

	std::unordered_map<std::string, std::string> actions;

};

