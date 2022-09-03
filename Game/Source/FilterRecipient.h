#pragma once

#include "Recipient.h"
#include "Label.h"
#include <vector>

class FilterRecipient : public Recipient
{
public: // Functions

	FilterRecipient(const char* name, float money, bool hidden, bool open, float* totalMoneyPtr) : Recipient(name, money, hidden, open, RecipientType::FILTER)
	{
		this->totalMoneyPtr = totalMoneyPtr;
		NewLabel();
	}

	~FilterRecipient() override
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
			ImGui::PushID(-id / ( i + 1 ));

			if (i == 0) { if (ImGui::Button("+")) NewLabel(); }
			else ImGui::Dummy({ 38, 0 });

			ImGui::SameLine();

			if (size > 1) { if (ImGui::Button("X")) DeleteLabel(i); }
			else ImGui::Dummy({ 15, 0 });

			ImGui::SameLine();

			if (ImGui::Button(" > "))
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
			ImGui::PopItemWidth();

			ImGui::PopID();
		}

		if (hidden) ImGui::EndDisabled();
	}

	void NewLabel(const char* name = "New Filter", float money = 0.0f)
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
