#pragma once

#include "Container.h"

class FilterContainer : public Container
{
public: // Functions

	FilterContainer(const char* name, bool hidden, bool open, bool unified, float* totalMoneyPtr) : Container(name, hidden, open, unified, totalMoneyPtr, ContainerType::FILTER)
	{
		NewLabel("New Filter");
	}

	~FilterContainer() override
	{
		ClearLabels();
	}

	void Start(const char* currency) override
	{
		SetFormat("%.2f ", currency);
	}

	void Update() override 
	{
		money = 0;
		for (Label* l : labels) money += l->money;
	}

	void Draw() override
	{
		if (hidden) ImGui::BeginDisabled();

		size_t size = labels.size();

		for (suint i = 0; i < size; ++i)
		{
			ImGui::PushID(-id / ( i + 1 ));

			if (i == 0) { if (ImGui::Button("+")) NewLabel("New Filter"); }
			else ImGui::Dummy({ 38, 0 });

			ImGui::SameLine();

			float width = 100.0f;
			if (!unified)
			{
				if (size > 1) { if (ImGui::Button("X")) DeleteLabel(i); }
				else ImGui::Dummy({ 15, 0 });

				ImGui::SameLine();

				if (ImGui::Button(" > "))
				{
					*totalMoneyPtr -= GetLabelMoney(i);
					DeleteLabel(i);
					if (labels.empty()) NewLabel("New Filter");
					ImGui::PopID();
					break;
				}
				ImGui::SameLine();

				ImGui::PushItemWidth(textFieldSize);
				ImGui::InputText("##FilterName", &labels[i]->name);
				ImGui::PopItemWidth(); ImGui::SameLine();
			}
			else width += 50;


			ImGui::PushItemWidth(width);
			ImGui::DragFloat("##Drag", &labels[i]->money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, format.c_str(), ImGuiSliderFlags_AlwaysClamp);
			ImGui::PopItemWidth();

			ImGui::PopID();
		}

		if (hidden) ImGui::EndDisabled();
	}

private:

};
