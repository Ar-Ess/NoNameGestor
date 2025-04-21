#pragma once

#include "Container.h"

class EconomyScene;


class TotalContainer : public Container
{
public: // Functions

	TotalContainer(const char* name, std::string* format) : Container(name, false, true, false, nullptr, format, ContainerType::TOTAL_MONEY)
	{
	}

	void Start(const char* currency) override
	{
	}

	void Draw() override 
	{
		ImGui::Text(name.c_str()); ImGui::SameLine();
		ImGui::PushItemWidth(150.0f);
		ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, MAX_MONEY, (*format).c_str(), ImGuiSliderFlags_AlwaysClamp);
		ImGui::PopItemWidth();
		ImGui::SameLine();
	}

private: // Functions

private: // Variables

	bool editTotal = false;
	float tempTotal = 0.0f;
	bool negative = false;
	std::string movementName = "New Movement";
	int date[3] = { 1, 1, 2010 };
	bool datePicked = false;
};

