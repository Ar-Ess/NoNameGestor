#ifndef __UNASIGNED_MONEY_RECIPIENT_H__
#define __UNASIGNED_MONEY_RECIPIENT_H__

#include "Recipient.h"

class UnasignedMoneyRecipient : public Recipient
{
public: // Functions

	UnasignedMoneyRecipient(const char* name, float money, bool* showFutureMoney, bool* allowFutureCover) : Recipient(name, money, RecipientType::UNASIGNED_MONEY)
	{
		this->showFutureMoney = showFutureMoney;
		this->allowFutureCover = allowFutureCover;
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
	}

	void Draw() override
	{
		if (!*showFutureMoney)
		{
			ImGui::Text(name.c_str()); ImGui::SameLine();
			ImGui::Text(": %.2f EUR", money);
		}
		else
		{
			ImGui::Text("Unasigned Actual Money"); ImGui::SameLine();
			ImGui::Text(": %.2f EUR", actualMoney);
			ImGui::Text("Unasigned Future Money"); ImGui::SameLine();
			ImGui::Text(": %.2f EUR", futureMoney);
		}
	}

	void SetMoney(float money, float actualMoney, float futureMoney)
	{
		this->money = money;
		this->actualMoney = actualMoney;
		this->futureMoney = futureMoney;
	}

private: // Functions

private: // Variables
	float futureMoney = 0.0f;
	float actualMoney = 0.0f;
	bool* showFutureMoney = nullptr;
	bool* allowFutureCover = nullptr;
};

#endif // !__UNASIGNED_MONEY_RECIPIENT_H__