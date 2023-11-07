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
		ImGui::Text("Total Money"); ImGui::SameLine();
		ImGui::PushItemWidth(150.0f);
		ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, format.c_str(), ImGuiSliderFlags_AlwaysClamp);
		ImGui::PopItemWidth();
		ImGui::SameLine();
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

