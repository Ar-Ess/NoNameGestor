#pragma once

#include "Recipient.h"
#include "imgui/imgui_stdlib.h"
#include "Rect.h"

class LimitRecipient : public Recipient
{
public: // Functions

	LimitRecipient(const char* name, float money, float limit = 0) : Recipient(name, money, RecipientType::LIMIT)
	{
		this->limit = limit;
		this->tempLimit = limit;
	}

	void Draw() override
	{
		if (ImGui::BeginTable(name.c_str(), 1))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::PushItemWidth(150.0f);
			ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, limit, "%.2f EUR");
			ImVec2 size = ImGui::GetItemRectSize();
			size.y -= 15;
			ImGui::PopItemWidth();

			ImGui::SameLine();
			ImGui::Text(" / %.2f", limit); ImGui::SameLine();
			if (ImGui::Button("Edit")) editLimit = true;

			ImGui::TableNextColumn();

			ImGui::ProgressBar(money / limit, size, "");

			ImGui::EndTable();
		}

		if (!editLimit) return;

		ImGui::OpenPopup("Edit limit");
		ImGui::SetNextWindowPos(ImVec2((ImGui::GetWindowSize().x / 2) - 70, (ImGui::GetWindowSize().y / 2) - 50));
		ImGui::SetNextWindowSize(ImVec2(140, 100));
		if (ImGui::BeginPopupModal("Edit limit", nullptr, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::DragFloat("##Drag1", &tempLimit, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, "%.2f EUR");
			if (ImGui::Button("Done"))
			{
				limit = tempLimit;
				editLimit = false;
			}
			ImGui::EndPopup();
		}
	}

	float GetLimit() const
	{
		return limit;
	}

private: // Functions

private: // Variables

	float limit = 1.0f;
	float tempLimit = 0.0f;
	bool editLimit = false;
};
