#pragma once

#include "NoNameGestor/Audio/Intent.h"

class Interpreter
{

	friend class ActionResolver;

public:

	Intent Process(const std::string& sentence);
	void Draw();
	
private:

	static std::string Normalize(const std::string& sentence);
	static bool IsValid(const std::string& sentence);

private:

	std::string lastText;

};

