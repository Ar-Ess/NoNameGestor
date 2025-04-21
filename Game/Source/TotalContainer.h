#pragma once

#include "Container.h"

class TotalContainer : public Container
{
public: // Functions

	TotalContainer(const char* name, std::string* format, bool* showFutureMoney) : Container(name, false, true, false, nullptr, format, ContainerType::TOTAL_MONEY)
	{
		this->showFutureMoney = showFutureMoney;
	}

	void Draw() override
	{
		ImGui::PushID(id);
		if (!*showFutureMoney)
		{
			ImGui::Text(name.c_str()); ImGui::SameLine();
			ImGui::Text((*format).c_str(), money);
		}
		else
		{
			ImGui::Text("Actual Total"); ImGui::SameLine();
			ImGui::Text((*format).c_str(), double(actualMoney));

			ImGui::Text("Future Total: "); ImGui::SameLine();
			ImGui::Text((*format).c_str(), futureMoney);
		}
		ImGui::PopID();
	}

	void SetMoney(float actualMoney, float futureMoney)
	{
		this->money = actualMoney + futureMoney;
		this->actualMoney = actualMoney;
		this->futureMoney = futureMoney;
	}

private: // Variables

	float futureMoney = 0.0f;
	float actualMoney = 0.0f;
	bool* showFutureMoney = nullptr;

};