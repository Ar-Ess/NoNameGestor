#pragma once

#include "Container.h"

class ArrearContainer : public Container
{
public: // Functions

	ArrearContainer(const char* name, bool hidden, bool open, bool unified, float* totalMoneyPtr) : Container(name, hidden, open, unified, totalMoneyPtr, ContainerType::ARREAR)
	{
		NewLabel("New Arrear");
	}

	~ArrearContainer() override
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

		for (unsigned short int i = 0; i < size; ++i)
		{
			ImGui::PushID(id->Value() * -1 * i);

			if (i == 0) { if (ImGui::Button("+")) NewLabel("New Arrear"); }
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
					if (labels.empty()) NewLabel("New Arrear");
					ImGui::PopID();
					break;
				}
				ImGui::SameLine();

				ImGui::PushItemWidth(textFieldSize);
				ImGui::InputText("##ArrearName", &labels[i]->name);
				ImGui::PopItemWidth(); ImGui::SameLine();
			}
			else width += 50;

			ImGui::PushItemWidth(width);
			ImGui::DragFloat("##Drag", &labels[i]->money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, "%.2f EUR", ImGuiSliderFlags_AlwaysClamp);
			ImGui::PopItemWidth();

			ImGui::PopID();
		}

		if (hidden) ImGui::EndDisabled();
	}

private:

};
