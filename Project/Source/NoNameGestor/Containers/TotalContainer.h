#pragma once

#include "Container.h"

class TotalContainer : public Container
{
public: // Functions

	TotalContainer(const char* name, String* format, Configuration* config);

	bool Update(Aggregate& agg) override;

	void Draw() override;

	const char* TypeName() const override
	{
		return "TOTAL ";
	}

	ContainerType Type() const override
	{
		return ContainerType::TOTAL_MONEY;
	}

private: // Variables

	float futureMoney = 0.0f;
	float leftMoney = 0.0f;

};