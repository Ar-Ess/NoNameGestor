#pragma once

#include "Log.h"
#define TO_COLOR(c) (const ImVec4(c[0]/255, c[1]/255, c[2]/255, c[3]/255))

class MovementLog : public Log
{
public:

	MovementLog(float money, float totalMoneyInstance, const char* name, bool* dateFormat);

	~MovementLog()
	{
		name.clear();
		name.shrink_to_fit();
	}

	void Draw(const char* currency, int index) override;

	virtual float GetOldInstance() const override;

	virtual float GetNewInstance() const override;

public:

	float color[4] = {100, 100, 100, 255};
	float money = 0;

};
