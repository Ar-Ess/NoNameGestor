#pragma once

#include "Container.h"

class InputContainer : public Container
{
public: // Functions

	InputContainer(const char* name, float money, String* format) :
		Container(name, false, true, false, format, nullptr)
	{
		this->money = money;
	}

	bool Update(Aggregate& agg) override
	{
		agg.total += money;
		return true;
	}

	void Draw() override 
	{
		ImGui::PushID(id.Data());
		{
			ImGui::Text("Money: "); ImGui::SameLine();
			ImGui::PushItemWidth(150.0f);
			{
				ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, MAX_MONEY, format->Str(), ImGuiSliderFlags_AlwaysClamp);
			}
			ImGui::PopItemWidth();
		}
		ImGui::PopID();
	}

	const char* TypeName() const override
	{
		return "INPUT ";
	}

	ContainerType Type() const override
	{
		return ContainerType::INPUT_MONEY;
	}

};

