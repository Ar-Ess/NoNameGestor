#pragma once

#include "Framework/Utils/Nullable.h"
#include "Framework/Engine/ID.h"

#include <string>

struct Label
{
	Label(const std::string& name, float money) :
		id(ID::New()), name(name), money(money)
	{ }

	~Label() {}

	// Internal
	ID id = ID::Empty;

	// Common
	std::string name;
	float money = 0;

};
