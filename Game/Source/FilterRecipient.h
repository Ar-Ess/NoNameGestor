#pragma once

#include "Recipient.h"

class FilterRecipient : public Recipient
{
public: // Functions

	FilterRecipient(const char* name, float money, bool hidden, bool open) : Recipient(name, money, hidden, open, RecipientType::FILTER_SINGULAR)
	{
	}

	~FilterRecipient()
	{

	}

	void Start(const char* currency) override
	{
		SetFormat("%.2f ", currency);
	}

	void Draw() override
	{
		if (hidden) ImGui::BeginDisabled();

		ImGui::PushID(id);
		ImGui::PushItemWidth(150.0f);
		ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, format.c_str());
		ImGui::PopItemWidth();
		ImGui::PopID();

		if (hidden) ImGui::EndDisabled();
	}

private: // Functions

protected: // Variables
};
