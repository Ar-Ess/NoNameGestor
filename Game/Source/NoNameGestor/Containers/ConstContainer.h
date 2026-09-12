#pragma once

#include "NoNameGestor/Containers/Container.h"

class ConstContainer : public Container
{
	struct ConstLabel : public Label
	{
		ConstLabel(const std::string& name, float money, float perMonth, const DateTime& expectedRunoutDate, float expectedRunoutMoney, int offsetMonths);

		bool CanSetExpectedRunout() const;
		bool HasExpectedRunout() const;
		void CalculateBaseMoneyState();

		DateTime expectedRunoutDate;
		float expectedRunoutMoney;
		float perMonth;
		int offsetMonths;
		
		int maxPlannedMonths;
		float availableMoney;
		int finalPlannedMonths;
	};

public: // Functions

	ConstContainer(const std::string& name, bool hidden, bool open, bool unified, String* format, Configuration* config);

	ConstContainer(const FileManager::FileNode& node, String* format, Configuration* config);

	~ConstContainer() override = default;

	void Awake() override;

	bool Update(Aggregate& agg) override;

	void Draw() override;

	void DrawCashFlow(float widthRatio, float initX) const override;

	void Save(FileManager::FileNode node) const override;

	void NewLabel(const std::string& name = "New Const", float money = 0.0f, float perMonth = 0.0f, const DateTime& expectedRunoutDate = DateTime::BaseEpoch, float expectedRunoutMoney = 0, int offsetMonths = 0);

	const char* TypeName() const override;

	ContainerType Type() const override;

private:

	ConstLabel* select = nullptr;
	DateTime currentTime;

	bool editExpectedRunoutEnabled = false;
	int editRunoutMonths = 0;
	float editExpectedRunoutMoney = 0.0f;

};

