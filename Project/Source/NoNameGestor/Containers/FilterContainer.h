#pragma once

#include "Container.h"

class FilterContainer : public Container
{
public: // Functions

	FilterContainer(std::string&& name, bool hidden, bool open, bool unified, String* format, Configuration* config);

	FilterContainer(const FileManager::FileNode& node, String* format, Configuration* config);

	~FilterContainer() override = default;

	bool Update(Aggregate& agg) override;

	void Draw() override;

	void NewLabel(std::string&& name = "New Filter", float money = 0.0f);

	const char* TypeName() const override
	{
		return "FILTER";
	}

	ContainerType TypeEnum() const override
	{
		return ContainerType::FILTER;
	}

private:

};
