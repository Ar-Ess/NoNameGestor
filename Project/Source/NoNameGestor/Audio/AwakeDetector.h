#pragma once

#include "NoNameGestor/Gestor/Configuration.h"
#include "NoNameGestor/Audio/Response.h"
#include <string>
#include <chrono>

class AwakeDetector
{
public:

	enum class Result { IGNORED, WOKE, WOKE_WITH_LEFTOVER, COMMAND };

public:


	AwakeDetector(Configuration& config);

	Result Filter(std::string&);
	static size_t FindWholeWord(const std::string& text, const std::string& word);

private:

	bool enabled = false;
	Configuration& config;
	std::chrono::steady_clock::time_point awakeUntil{};
	std::chrono::seconds window{ 8 };

};

