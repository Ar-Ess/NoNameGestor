#pragma once

#include "Container.h"

class InputContainer : public Container
{
public: // Functions

	InputContainer(const char* name, float money, String* format);

	~InputContainer() override = default;

	bool Update(Aggregate& agg) override;

	void Draw() override;

	const char* TypeName() const override
	{
		return "INPUT ";
	}

	ContainerType Type() const override
	{
		return ContainerType::INPUT_MONEY;
	}

};

