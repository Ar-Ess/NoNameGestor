#pragma once

#include "Container.h"

class TotalContainer : public Container
{
public: // Functions

	TotalContainer(const char* name, String* format, Configuration* config);

	bool Update(Aggregate& agg) override;

	void Draw() override;

	std::string TypeName() const override
	{
		static std::string totalContainerTypeName = "TOTAL ";
		return totalContainerTypeName;
	}

	ContainerType TypeEnum() const override
	{
		return ContainerType::TOTAL_MONEY;
	}

private: // Variables

	float futureMoney = 0.0f;
	float leftMoney = 0.0f;

};