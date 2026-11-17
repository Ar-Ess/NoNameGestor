#pragma once

#include "Container.h"

class FutureContainer : public Container
{
public: // Functions

	FutureContainer(std::string&& name, bool hidden, bool open, bool unified, String* format, Configuration* config);

	FutureContainer(const FileManager::FileNode& node, String* format, Configuration* config);

	~FutureContainer() override = default;

	bool Update(Aggregate& agg) override;

	void Draw() override;

	void NewLabel(std::string&& name = "New Future", float money = 0.0f);

	const char* TypeName() const override
	{
		return "FUTURE";
	}

	ContainerType TypeEnum() const override
	{
		return ContainerType::FUTURE;
	}

private:

};
