#ifndef __UNASIGNED_MONEY_RECIPIENT_H__
#define __UNASIGNED_MONEY_RECIPIENT_H__

#include "Recipient.h"

class UnasignedMoneyRecipient : public Recipient
{
public: // Functions

	UnasignedMoneyRecipient(const char* name, float money) : Recipient(name, money, RecipientType::UNASIGNED_MONEY)
	{
	}

	~UnasignedMoneyRecipient() {}

	void Start() override
	{
		money = 0.0f;
	}

	void Update() override {}

	void Draw() override
	{
		ImGui::Text(name.c_str()); ImGui::SameLine();
		ImGui::Text(": %.2f EUR", money);
	}

	void SetMoney(float money)
	{
		this->money = money;
	}

private: // Functions

protected: // Variables
};

#endif // !__UNASIGNED_MONEY_RECIPIENT_H__