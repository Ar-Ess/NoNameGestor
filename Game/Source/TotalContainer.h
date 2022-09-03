#pragma once

#include "Container.h"

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
		ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, format.c_str());
		ImGui::PopItemWidth();
	}

private: // Functions

private: // Variables
};

