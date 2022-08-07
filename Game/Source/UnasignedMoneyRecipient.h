#ifndef __UNASIGNED_MONEY_RECIPIENT_H__
#define __UNASIGNED_MONEY_RECIPIENT_H__

#include "Recipient.h"

class UnasignedMoneyRecipient : public Recipient
{
public: // Functions

	UnasignedMoneyRecipient(const char* name, float money) : Recipient(name, money, RecipientType::UNASIGNED_MONEY)
	{
	}

	void Update() override
	{
		if (showFutureMoney && futureMoney > 0 && actualMoney < 0)
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
		if (!showFutureMoney)
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

	void SetMoney(float money, float actualMoney, float futureMoney, bool showFuture)
	{
		this->money = money;
		this->actualMoney = actualMoney;
		this->showFutureMoney = showFuture;
		this->futureMoney = futureMoney;
	}

private: // Functions

private: // Variables
	float futureMoney = 0.0f;
	float actualMoney = 0.0f;
	bool showFutureMoney = false;
};

#endif // !__UNASIGNED_MONEY_RECIPIENT_H__