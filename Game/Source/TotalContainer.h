#pragma once

#include "Container.h"
#include "MovementLog.h"

class EconomyScene;

class TotalContainer : public Container
{
public: // Functions

	TotalContainer(const char* name, std::vector<Log*>* logs, bool* dateFormat, int* maxLogs) : Container(name, false, true, false, nullptr, ContainerType::TOTAL_MONEY)
	{
		this->logs = logs;
		this->dateFormat = dateFormat;
		this->maxLogs = maxLogs;
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
		ImGui::SetNextWindowPos(ImVec2((ImGui::GetWindowSize().x / 2) - 145, (ImGui::GetWindowSize().y / 2) - 125));
		ImGui::SetNextWindowSize(ImVec2(290, 250));
		if (ImGui::BeginPopupModal("Add a movement to the Log", nullptr, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Dummy({ 0, 10 });

			// Label Name
			ImGui::Dummy({ 4, 0 }); ImGui::SameLine();
			ImGui::Text("Type the Movement Name:");
			ImGui::Spacing();
			ImGui::Dummy({ 14, 0 }); ImGui::SameLine();
			ImGui::InputText("##MoveName", &movementName);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			// Date Picker
			ImGui::Dummy({ 4, 0 }); ImGui::SameLine(); ImGui::Checkbox("##date", &datePicked); ImGui::SameLine();
			if (!datePicked)
			{
				ImGui::TextDisabled("Date:"); ImGui::SameLine();
				ImGui::TextDisabled("(!)");
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::TextUnformatted("Adding a date to a movement is highly recommendable");
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}
			}
			else
			{
				int format[2] = {31, 12};
				int index[2] = { 0, 1 };
				if (!*dateFormat)
				{
					format[0] = 12;
					format[1] = 31;
					index[0] = 1;
					index[1] = 0;
				}
				ImGui::Text("Date: "); ImGui::SameLine();
				ImGui::TextDisabled("Format?");
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					*dateFormat ?
						ImGui::TextUnformatted("Actual format: day / month / year\nChange it on preferences"):
						ImGui::TextUnformatted("Actual format: month / day / year\nChange it on preferences");
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}
				
				ImGui::PushItemWidth(40);
				ImGui::Dummy({ 4, 0 }); ImGui::SameLine(); ImGui::DragInt("##Day", &date[index[0]], 0.5f, 1, format[0]); ImGui::SameLine();
				ImGui::SameLine(); ImGui::Text(" / "); ImGui::SameLine();
				ImGui::DragInt("##Month", &date[index[1]], 0.5f, 1, format[1]); ImGui::SameLine();
				ImGui::Text(" / "); ImGui::SameLine();
				ImGui::DragInt("##Year", &date[2], 0.5f, 2010, 2147483647);
				ImGui::PopItemWidth();
			}

			ImGui::Spacing();
			ImGui::Spacing();
			if (!datePicked)
			{
				ImGui::Spacing();
				ImGui::Spacing();
			}

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
			float plus = 0;
			if (!datePicked) plus = 15;
			ImGui::Dummy({ 0, 12 + plus});
			ImGui::Dummy({ 30, 0 }); ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				tempTotal = 0;
				editTotal = false;
				movementName.clear();
				movementName = "New Movement";
				datePicked = false;
				date[0] = 1;
				date[1] = 1;
				date[2] = 2010;
			}
			ImGui::SameLine(0.0f, 20.0f);
			bool disabled = (tempTotal == 0);
			if (disabled) ImGui::BeginDisabled();
			if (ImGui::Button("Done"))
			{
				negative ? tempTotal *= -1 : tempTotal *= 1;
				money += tempTotal;
				
				CreateMovement(tempTotal, movementName.c_str());
				tempTotal = 0;
				editTotal = false;
				movementName.clear();
				movementName = "New Movement";
				datePicked = false;
				date[0] = 1;
				date[1] = 1;
				date[2] = 2010;
			}
			if (disabled) ImGui::EndDisabled();
		}
		ImGui::EndPopup();
		ImGui::PopID();
	}

	void CreateMovement(float money, const char* name)
	{
		logs->emplace_back(new MovementLog(money, this->money, name, dateFormat));
		if (datePicked) logs->back()->SetDate(date[0], date[1], date[2]);
		if (logs->size() > *maxLogs)
		{
			Log* erase = logs->front();
			logs->erase(logs->begin());
			RELEASE(erase);
		}
	}

private: // Functions

private: // Variables

	std::vector<Log*>* logs = nullptr;
	bool editTotal = false;
	float tempTotal = 0.0f;
	bool negative = false;
	std::string movementName = "New Movement";
	int date[3] = { 1, 1, 2010 };
	bool datePicked = false;
	bool* dateFormat = nullptr;
	int* maxLogs = nullptr;
};

