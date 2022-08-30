#pragma once

#include "Recipient.h"
#include <vector>

class ArrearPlrRecipient : public Recipient
{
private: //structs

	struct Arrear
	{
		Arrear(const char* name, float money)
		{
			this->name = name;
			this->money = money;
		}

		float money = 0;
		std::string name;
	};

public: // Functions

	ArrearPlrRecipient(const char* name, float money, bool hidden, bool open, float* totalMoneyPtr) : Recipient(name, money, hidden, open, RecipientType::ARREAR_PLURAL)
	{
		this->totalMoneyPtr = totalMoneyPtr;
		NewArrear();
	}

	~ArrearPlrRecipient() override
	{
		ClearArrears();
	}

	void Update() override
	{
		money = 0;
		for (Arrear f : arrears) money += f.money;
	}

	void Draw() override
	{
		if (hidden) ImGui::BeginDisabled();

		size_t size = arrears.size();

		for (unsigned short int i = 0; i < size; ++i)
		{
			ImGui::PushID(id * -1 * i);

			if (ImGui::Button(" -> "))
			{
				*totalMoneyPtr -= GetArrearMoney(i);
				DeleteFuture(i);
				if (arrears.empty()) NewArrear();
				ImGui::PopID();
				break;
			}
			ImGui::SameLine();

			ImGui::PushItemWidth(textFieldSize);
			ImGui::InputText("##FutureName", &arrears[i].name);
			ImGui::PopItemWidth(); ImGui::SameLine();

			ImGui::PushItemWidth(100.f);
			ImGui::DragFloat("##Drag", &arrears[i].money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, "%.2f EUR");
			ImGui::PopItemWidth(); ImGui::SameLine();

			if (size > 1 && ImGui::Button("X")) DeleteFuture(i);

			if (i == 0)
			{
				if (size == 1)
				{
					ImGui::SameLine();
					ImGui::Dummy(ImVec2{ 15, 0 });
				}
				ImGui::SameLine();
				if (ImGui::Button("+")) NewArrear();
			}

			ImGui::PopID();
		}

		if (hidden) ImGui::EndDisabled();
	}

	void NewArrear(const char* name = "New Arrear", float money = 0.0f)
	{
		arrears.push_back(Arrear(name, money));
	}

	void ClearArrears()
	{
		for (Arrear f : arrears)
		{
			f.name.clear();
			f.name.shrink_to_fit();
		}
		arrears.clear();
		arrears.shrink_to_fit();
	}

	int GetSize() const
	{
		return arrears.size();
	}

	float GetArrearMoney(int i) const
	{
		return arrears[i].money;
	}

	const char* GetArrearName(int i) const
	{
		return arrears[i].name.c_str();
	}

private:

	void DeleteFuture(int index)
	{
		arrears[index].name.clear();
		arrears[index].name.shrink_to_fit();
		arrears.erase(arrears.begin() + index);
	}

private:

	std::vector<Arrear> arrears;
	float* totalMoneyPtr = nullptr;
};
