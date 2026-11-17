#pragma once

#include "Framework/Utils/Maths.h"

#include "NoNameGestor/Containers/Container.h"

class LimitContainer : public Container
{
	struct LimitLabel : public Label
	{
		LimitLabel(std::string&& name, float money, float limit);

		Flag Draw(bool unified, bool first, bool single, int textFieldSize, String* format) override;

		float limit;
	};

public: // Functions

	LimitContainer(std::string&& name, bool hidden, bool open, bool unified, String* format, Configuration* config);

	LimitContainer(const FileManager::FileNode& node, String* format, Configuration* config);

	~LimitContainer() override = default;

	bool Update(Aggregate& agg) override;

	void Draw() override;

	void Save(FileManager::FileNode node) const override;

	void NewLabel(std::string&& name = "New Limit", float money = 0.0f, float limit = 0.0f);

	std::string TypeName() const override
	{
		static std::string limitContainerTypeName = "LIMIT ";
		return limitContainerTypeName;
	}

	ContainerType TypeEnum() const override
	{
		return ContainerType::LIMIT;
	}

private:

	LimitLabel* select = nullptr;
};
