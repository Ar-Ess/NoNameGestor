#pragma once

#include <string>

class Movement
{
public:
	Movement(float money, const char* name);

	std::string name;
	float money = 0;
};
