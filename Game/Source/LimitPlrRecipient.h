#pragma once

#include "Recipient.h"
#include "Label.h"
#include <vector>

class LimitPlrRecipient : public Recipient
{
public: // Functions

	LimitPlrRecipient(const char* name, float money, bool hidden, bool open, float* totalMoneyPtr) : Recipient(name, money, hidden, open, RecipientType::LIMIT_PLURAL)
	{
		this->totalMoneyPtr = totalMoneyPtr;
		NewLabel();
	}

	~LimitPlrRecipient() override
	{
		ClearLabels();
	}

	void Start(const char* currency) override
	{
		SetFormat("%.2f ", currency);
	}

	void Update() override 
	{
		money = 0;
		for (Label* l : labels) money += l->money;
	}

	void Draw() override
	{
		if (hidden) ImGui::BeginDisabled();

		size_t size = labels.size();

		for (suint i = 0; i < size; ++i)
		{
			ImGui::PushID(id * -1 * i);

			if (ImGui::Button(" -> "))
			{
				*totalMoneyPtr += GetLabelMoney(i);
				DeleteLabel(i);
				if (labels.empty()) NewLabel();
				ImGui::PopID();
				break;
			}
			ImGui::SameLine();

			ImGui::PushItemWidth(textFieldSize);
			ImGui::InputText("##LimitName", &labels[i]->name);
			ImGui::PopItemWidth(); ImGui::SameLine();

			if (ImGui::BeginTable(name.c_str(), 1))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::PushItemWidth(100.f);
				ImGui::DragFloat("##Drag", &labels[i]->money, 1.0f, 0.0f, labels[i]->limit, format.c_str());
				ImVec2 itemSize = ImGui::GetItemRectSize();
				itemSize.y -= 15;
				ImGui::PopItemWidth();ImGui::SameLine();

				ImGui::Text("/"); ImGui::SameLine();
				ImGui::Text(format.c_str(), labels[i]->limit); ImGui::SameLine();
				if (ImGui::Button("Edit"))
				{
					editLimit = true;
					editLimitIndex = i;
				}

				ImGui::SameLine();

				if (size > 1 && ImGui::Button("X")) DeleteLabel(i);

				if (i == 0)
				{
					if (size == 1)
					{
						ImGui::SameLine();
						ImGui::Dummy(ImVec2{ 15, 0 });
					}
					ImGui::SameLine();
					if (ImGui::Button("+")) NewLabel();
				}

				ImGui::TableNextColumn();

				ImGui::ProgressBar(labels[i]->money / labels[i]->limit, itemSize, "");
			}
			ImGui::EndTable();

			ImGui::PopID();
		}

		if (hidden) ImGui::EndDisabled();

		if (!editLimit)
		{
			if (hidden) ImGui::EndDisabled();
			return;
		}

		ImGui::PushID(id);
		ImGui::OpenPopup("Edit limit");
		ImGui::SetNextWindowPos(ImVec2((ImGui::GetWindowSize().x / 2) - 70, (ImGui::GetWindowSize().y / 2) - 50));
		ImGui::SetNextWindowSize(ImVec2(140, 100));
		if (ImGui::BeginPopupModal("Edit limit", nullptr, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::DragFloat("##Drag1", &labels[editLimitIndex]->tempLimit, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, format.c_str());
			if (ImGui::Button("Done"))
			{
				labels[editLimitIndex]->limit = labels[editLimitIndex]->tempLimit;
				editLimit = false;
			}
		}
		ImGui::EndPopup();
		ImGui::PopID();
	}

	void NewLabel(const char* name = "New Limit", float money = 0.0f, float limit = 1.0f)
	{
		labels.push_back(new Label(name, money, limit));
	}

	void ClearLabels()
	{
		for (Label* l : labels) RELEASE(l);
		labels.clear();
		labels.shrink_to_fit();
	}

	int GetSize() const
	{
		return labels.size();
	}

	float GetLabelMoney(int i) const
	{
		return labels[i]->money;
	}
	
	const char* GetLabelName(int i) const
	{
		return labels[i]->name.c_str();
	}

	float GetLabelLimit(int i) const
	{
		return labels[i]->limit;
	}

private:

	void DeleteLabel(int index)
	{
		labels[index]->name.clear();
		labels[index]->name.shrink_to_fit();
		labels.erase(labels.begin() + index);
	}

private:

	std::vector<Label*> labels;
	float* totalMoneyPtr = nullptr;
	bool editLimit = false;
	int editLimitIndex = 0;
};
