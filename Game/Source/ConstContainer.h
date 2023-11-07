#pragma once

#include "Container.h"

class ConstContainer : public Container
{
public: // Functions

	ConstContainer(const char* name, bool hidden, bool open, bool unified, float* totalMoneyPtr) : Container(name, hidden, open, unified, totalMoneyPtr, ContainerType::CONSTANT)
	{
		NewLabel("New Const");
	}

	~ConstContainer() override
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

		for (unsigned int i = 0; i < size; ++i)
		{
			ImGui::PushID(-id->Value() / ( i + 1 ));

			if (i == 0) { if (ImGui::Button("+")) NewLabel("New Const"); }
			else ImGui::Dummy({ 38, 0 });

			ImGui::SameLine();

			float width = 100.0f;
			if (!unified)
			{
				if (size > 1) { if (ImGui::Button("X")) DeleteLabel(i); }
				else ImGui::Dummy({ 15, 0 });

				ImGui::SameLine();

				if (ImGui::Button(" O ")) *totalMoneyPtr -= GetLabelMoney(i);
				ImGui::SameLine();

				ImGui::PushItemWidth(textFieldSize);
				ImGui::InputText("##ConstName", &labels[i]->name);
				ImGui::PopItemWidth(); ImGui::SameLine();
			}
			else
			{
				ImGui::Dummy({ 15, 0 });

				ImGui::SameLine();

				if (ImGui::Button(" O "))
				{
					*totalMoneyPtr -= GetLabelMoney(i);
					ImGui::PopID();
					break;
				}

				ImGui::SameLine();
				width += 50;
			}

			ImGui::BeginGroup();
			ImGui::PushItemWidth(width);
			ImGui::DragFloat("##Drag", &labels[i]->money, 1.0f, 0.0f, labels[i]->limit, format.c_str(), ImGuiSliderFlags_AlwaysClamp);
			ImVec2 itemSize = ImGui::GetItemRectSize();
			itemSize.y -= 15;
			ImGui::PopItemWidth();

			ImGui::ProgressBar(labels[i]->money / labels[i]->limit, itemSize, "");
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::Text("/"); ImGui::SameLine();
			ImGui::Text(format.c_str(), labels[i]->limit); ImGui::SameLine();
			if (ImGui::Button("Edit"))
			{
				editLimit = true;
				editLimitIndex = i;
			}

			ImGui::PopID();
		}

		if (hidden) ImGui::EndDisabled();

		if (!editLimit)
		{
			if (hidden) ImGui::EndDisabled();
			return;
		}

		ImGui::PushID(id);
		ImGui::OpenPopup("Edit const");
		ImGui::SetNextWindowPos(ImVec2((ImGui::GetWindowSize().x / 2) - 70, (ImGui::GetWindowSize().y / 2) - 50));
		ImGui::SetNextWindowSize(ImVec2(140, 100));
		if (ImGui::BeginPopupModal("Edit const", nullptr, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::DragFloat("##Drag1", &labels[editLimitIndex]->tempLimit, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, format.c_str(), ImGuiSliderFlags_AlwaysClamp);
			if (ImGui::Button("Done"))
			{
				labels[editLimitIndex]->limit = labels[editLimitIndex]->tempLimit;
				editLimit = false;
			}
		}
		ImGui::EndPopup();
		ImGui::PopID();
	}

	float GetLabelLimit(int i) const
	{
		return labels[i]->limit;
	}

private:

	bool editLimit = false;
	int editLimitIndex = 0;

};
