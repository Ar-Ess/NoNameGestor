#ifndef __TOTAL_MONEY_RECIPIENT_H__
#define __TOTAL_MONEY_RECIPIENT_H__

#include "Recipient.h"

class TotalMoneyRecipient : public Recipient
{
public: // Functions

	TotalMoneyRecipient(const char* name, float money) : Recipient(name, money, false, RecipientType::TOTAL_MONEY)
	{
	}

	void Draw() override
	{
		ImGui::Text(name.c_str()); ImGui::SameLine();
		ImGui::PushItemWidth(150.0f);
		ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, "%.2f EUR");
		ImGui::PopItemWidth();
	}

private: // Functions

protected: // Variables
};

#endif // !__TOTAL_MONEY_RECIPIENT_H__

