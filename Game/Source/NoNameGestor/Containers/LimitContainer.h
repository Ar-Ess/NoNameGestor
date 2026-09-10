#pragma once

#include "Framework/Utils/Maths.h"

#include "NoNameGestor/Containers/Container.h"

class LimitContainer : public Container
{
	struct LimitLabel : public Label
	{
		LimitLabel(const std::string& name, float money, float limit);

		float limit;
	};

public: // Functions

	LimitContainer(const std::string& name, bool hidden, bool open, bool unified, String* format, Configuration* config);

	LimitContainer(const FileManager::FileNode& node, String* format, Configuration* config);

	~LimitContainer() override = default;

	bool Update(Aggregate& agg) override;

	void Draw() override;

	void Save(FileManager::FileNode node) const override;

	void NewLabel(const std::string& name = "New Limit", float money = 0.0f, float limit = 0.0f);

	const char* TypeName() const override
	{
		return "LIMIT ";
	}

	ContainerType Type() const override
	{
		return ContainerType::LIMIT;
	}

private:

	LimitLabel* select = nullptr;
};
