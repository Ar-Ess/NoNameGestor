#pragma once

#include "Container.h"

class TotalContainer : public Container
{
public: // Functions

	TotalContainer(const char* name, String* format, Configuration* config) :
		Container(name, false, true, false, format, config)
	{ }

	bool Update(Aggregate& agg) override
	{
		money = agg.total - agg.assigned + agg.future;
		leftMoney = agg.total - agg.assigned;
		futureMoney = agg.future;

		return true;
	}

	void Draw() override
	{
		ImGui::PushID(id.Data());
		{
			if (!config->showFutureUnasigned)
			{
				ImGui::Text("Total: "); ImGui::SameLine();
				ImGui::Text(format->Str(), money);
			}
			else
			{
				ImGui::Text("Actual Total: "); ImGui::SameLine();
				ImGui::Text(format->Str(), double(leftMoney));

				ImGui::Text("Future Total: "); ImGui::SameLine();
				ImGui::Text(format->Str(), futureMoney);
			}
		}
		ImGui::PopID();
	}

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