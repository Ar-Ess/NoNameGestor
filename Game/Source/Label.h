#pragma once
#include <string>

struct Label
{
	Label(const char* name, float money, float limit = 0.0f)
	{
		this->name = name;
		this->limit = limit;
		this->tempLimit = limit;
		this->money = money;
	}

	~Label()
	{
		name.clear();
		name.shrink_to_fit();
	}

	float limit = 1.0f;
	float tempLimit = 0;
	float money = 0;
	std::string name;
};
