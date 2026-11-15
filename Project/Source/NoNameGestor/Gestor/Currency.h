#pragma once

class Currency
{
public:

	Currency(const char* name)
	{
		this->name = name;
	}

	const char* name = nullptr;

};