#pragma once

#include "Log.h"

class InformativeLog : public Log
{
public:

	InformativeLog(float oldInstance, float newInstance, const char* name);

	~InformativeLog()
	{
		name.clear();
		name.shrink_to_fit();
	}

	void Draw(const char* currency, int index) override;

	virtual float GetOldInstance() const override;

	virtual float GetNewInstance() const override;

public:

	float newInstance = 0;

};
