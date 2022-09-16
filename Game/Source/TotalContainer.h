#pragma once

#include "Container.h"

class EconomyScene;

class TotalContainer : public Container
{
public: // Functions

	TotalContainer(const char* name) : Container(name, false, true, false, nullptr, ContainerType::TOTAL_MONEY)
	{
	}

	void Start(const char* currency) override
	{
		SetFormat("%.2f ", currency);
	}

	void Draw() override
	{
		ImGui::Text(name.c_str()); ImGui::SameLine();
		ImGui::PushItemWidth(150.0f);
		ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, format.c_str(), ImGuiSliderFlags_AlwaysClamp);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Make Movement")) editTotal = true;

		if (!editTotal) return;

		ImGui::PushID(id);
		ImGui::OpenPopup("Add a movement to the Log");
		ImGui::SetNextWindowPos(ImVec2((ImGui::GetWindowSize().x / 2) - 145, (ImGui::GetWindowSize().y / 2) - 98));
		ImGui::SetNextWindowSize(ImVec2(290, 196));
		if (ImGui::BeginPopupModal("Add a movement to the Log", nullptr, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Dummy({ 0, 6 });

			// Label Name
			ImGui::Dummy({ 4, 0 }); ImGui::SameLine();
			ImGui::Text("Type the Movement Name:");
			ImGui::Spacing();
			ImGui::Dummy({ 14, 0 }); ImGui::SameLine();
			ImGui::InputText("##MoveName", &movementName);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			// DragFloat
			ImGui::Dummy({ 4, 0 }); ImGui::SameLine();
			ImGui::Text("Type the movement's amount:");
			ImGui::Spacing();
			ImGui::Dummy({ 14, 0 }); ImGui::SameLine();
			if (!negative && ImGui::Button("+")) negative = true;
			else if (negative && ImGui::Button("-")) negative = false;
			ImGui::SameLine();
			ImGui::DragFloat("##Drag1", &tempTotal, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, format.c_str(), ImGuiSliderFlags_AlwaysClamp);
			
			// Done Button
			ImGui::Dummy({ 0, 12 });
			ImGui::Dummy({ 30, 0 }); ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				tempTotal = 0;
				editTotal = false;
				movementName.clear();
				movementName = "New Movement";
			}
			ImGui::SameLine(0.0f, 20.0f);
			if (tempTotal == 0) ImGui::BeginDisabled();
			if (ImGui::Button("Done"))
			{
				negative ? tempTotal * -1 : tempTotal * 1;
				money += tempTotal;

				tempTotal = 0;
				editTotal = false;
				movementName.clear();
				movementName = "New Movement";
			}
			if (tempTotal == 0) ImGui::EndDisabled();
		}
		ImGui::EndPopup();
		ImGui::PopID();
	}

private: // Functions

private: // Variables

	bool editTotal = false;
	float tempTotal = 0.0f;
	bool negative = false;
	std::string movementName = "New Movement";
};

