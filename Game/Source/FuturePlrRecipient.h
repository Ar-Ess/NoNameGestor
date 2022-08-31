#pragma once

#include "Recipient.h"
#include "Label.h"

class FuturePlrRecipient : public Recipient
{
public: // Functions

	FuturePlrRecipient(const char* name, float money, bool hidden, bool open, float* totalMoneyPtr) : Recipient(name, money, hidden, open, RecipientType::FUTURE_PLURAL)
	{
		this->totalMoneyPtr = totalMoneyPtr;
		NewLabel();
	}

	~FuturePlrRecipient() override
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
		for (Label* f : labels) money += f->money;
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
			ImGui::InputText("##FutureName", &labels[i]->name);
			ImGui::PopItemWidth(); ImGui::SameLine();

			ImGui::PushItemWidth(100.f);
			ImGui::DragFloat("##Drag", &labels[i]->money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, format.c_str());
			ImGui::PopItemWidth(); ImGui::SameLine();

			if (size > 1 && ImGui::Button("X")) DeleteLabel(i);

			if (i == 0)
			{
				if (size == 1)
				{
					ImGui::SameLine();
					ImGui::Dummy(ImVec2{15, 0});
				}
				ImGui::SameLine();
				if (ImGui::Button("+")) NewLabel();
			}

			ImGui::PopID();
		}

		if (hidden) ImGui::EndDisabled();
	}

	void NewLabel(const char* name = "New Future", float money = 0.0f)
	{
		labels.push_back(new Label(name, money));
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
};
