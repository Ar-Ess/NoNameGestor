#pragma once

#include "Container.h"

class UnasignedContainer : public Container
{
public: // Functions

	UnasignedContainer(const char* name, float money, bool* showFutureMoney, bool* allowFutureCover, bool* showArrearMoney, bool* allowArrearsFill) : Container(name, money, false, true, ContainerType::UNASIGNED_MONEY)
	{
		this->showFutureMoney = showFutureMoney;
		this->allowFutureCover = allowFutureCover;
		this->showArrearMoney = showArrearMoney;
		this->allowArrearsFill = allowArrearsFill;
	}

	void Start(const char* currency) override
	{
		SetFormat("%.2f ", currency);
	}

	void Update() override
	{
		if (*showFutureMoney && *allowFutureCover && futureMoney > 0 && actualMoney < 0)
		{
			float debt = -actualMoney;
			if (debt <= futureMoney)
			{
				futureMoney -= debt;
				actualMoney = 0;
			}
			else
			{
				actualMoney += futureMoney;
				futureMoney = 0;
			}
		}

		if (*showArrearMoney && *allowArrearsFill && arrearMoney < 0 && actualMoney > 0)
		{
			float debt = -arrearMoney;
			if (debt <= actualMoney)
			{
				actualMoney -= debt;
				arrearMoney = 0;
			}
			else
			{
				arrearMoney += actualMoney;
				actualMoney = 0;
			}
		}
	}

	void Draw() override
	{
		if (!*showFutureMoney && !*showArrearMoney)
		{
			ImGui::Text(name.c_str()); ImGui::SameLine();
			ImGui::Text(": "); ImGui::SameLine();
			ImGui::Text(format.c_str(), money);
		}
		else
		{
			ImGui::Text("Unasigned Actual Money"); ImGui::SameLine();
			ImGui::Text(": "); ImGui::SameLine();

			float add = 0;
			if (*showFutureMoney && !*showArrearMoney) add = arrearMoney;
			if (!*showFutureMoney && *showArrearMoney) add = futureMoney;

			ImGui::Text(format.c_str(), double(actualMoney) + add);
			if (*showFutureMoney)
			{
				ImGui::Text("Unasigned Future Money"); ImGui::SameLine();
				ImGui::Text(": "); ImGui::SameLine();
				ImGui::Text(format.c_str(), futureMoney);
			}
			if (*showArrearMoney)
			{
				ImGui::Text("Unasigned Arrear Money"); ImGui::SameLine();
				ImGui::Text(": "); ImGui::SameLine();
				ImGui::Text(format.c_str(), arrearMoney);
			}
		}
	}

	void SetMoney(float money, float actualMoney, float futureMoney, float arrearMoney)
	{
		this->money = money;
		this->actualMoney = actualMoney;
		this->futureMoney = futureMoney;
		this->arrearMoney = arrearMoney;
	}

private: // Functions

private: // Variables

	float futureMoney = 0.0f;
	float actualMoney = 0.0f;
	float arrearMoney = 0.0f;
	bool* showFutureMoney = nullptr;
	bool* allowFutureCover = nullptr;
	bool* showArrearMoney = nullptr;
	bool* allowArrearsFill = nullptr;
};