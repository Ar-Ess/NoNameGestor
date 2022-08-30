#pragma once

#include "Recipient.h"

class LimitRecipient : public Recipient
{
public: // Functions

	LimitRecipient(const char* name, float money, bool hidden, bool open) : Recipient(name, money, hidden, open, RecipientType::LIMIT_SINGULAR)
	{
		this->limit = limit;
		this->tempLimit = limit;
	}

	void Start(const char* currency) override
	{
		SetFormat("%.2f ", currency);
	}

	void Draw() override
	{
		if (hidden) ImGui::BeginDisabled();

		ImGui::PushID(id);
		if (ImGui::BeginTable(name.c_str(), 1))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::PushItemWidth(150.f);
			ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, limit, format.c_str());
			ImVec2 size = ImGui::GetItemRectSize();
			size.y -= 15;
			ImGui::PopItemWidth();

			ImGui::SameLine();
			ImGui::Text(" / "); ImGui::SameLine();
			ImGui::Text(format.c_str(), limit); ImGui::SameLine();
			if (ImGui::Button("Edit")) editLimit = true;

			ImGui::TableNextColumn();

			ImGui::ProgressBar(money / limit, size, "");
		}
		ImGui::EndTable();

		if (!editLimit)
		{
			ImGui::PopID();
			if (hidden) ImGui::EndDisabled();
			return;
		}

		ImGui::OpenPopup("Edit limit");
		ImGui::SetNextWindowPos(ImVec2((ImGui::GetWindowSize().x / 2) - 70, (ImGui::GetWindowSize().y / 2) - 50));
		ImGui::SetNextWindowSize(ImVec2(140, 100));
		if (ImGui::BeginPopupModal("Edit limit", nullptr, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::DragFloat("##Drag1", &tempLimit, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, format.c_str());
			if (ImGui::Button("Done"))
			{
				limit = tempLimit;
				editLimit = false;
			}
		}
		ImGui::EndPopup();
		ImGui::PopID();
	}

	float GetLimit() const
	{
		return limit;
	}

	void SetLimit(float limit)
	{
		this->limit = limit;
		this->tempLimit = limit;
	}

private: // Functions

private: // Variables

	float limit = 1.0f;
	float tempLimit = 0.0f;
	bool editLimit = false;
};
