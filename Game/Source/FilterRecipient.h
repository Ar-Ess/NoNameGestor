#ifndef __FILTER_RECIPIENT_H__
#define __FILTER_RECIPIENT_H__

#include "Recipient.h"
#include "imgui/imgui_stdlib.h"

class FilterRecipient : public Recipient
{
public: // Functions

	FilterRecipient(const char* name, float money) : Recipient(name, money, RecipientType::FILTER)
	{
	}

	virtual ~FilterRecipient() {}

	void Start() override {}

	void Update() override {}

	void Draw() override
	{
		ImGui::PushItemWidth(150.0f);
		ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, "%.2f EUR");
		ImGui::PopItemWidth();
	}

private: // Functions

protected: // Variables
};

#endif // !__FILTER_RECIPIENT_H__
