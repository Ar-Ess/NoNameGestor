#pragma once

#include <string>
#include <map>

struct Intent
{
	std::string original;
	std::string normalized;
	std::string action;
	std::map<std::string, std::string> parameters;

	bool IsValid() const { return !normalized.empty(); }
	bool IsUnderstood() const { return !action.empty(); }

};

