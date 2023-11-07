#pragma once
#include <string>

#define EXTENSION ".nng"

class Page
{
public:

	Page()
	{
		name = "New_File";
		name += EXTENSION;
	}

	void Start(const char* currency)
	{
		SetFormat("%.2f ", currency);
	}

	void Update()
	{
		actualMoney = money;

		containers.Iterate<float*, float*, float*, float*>
		(
			&actualMoney,
			&futureMoney,
			&arrearMoney,
			&constMoney,
			[](Container* c, float* totalMoney, float* futureMoney, float* arrearMoney, float* constMoney)
			{
				if (c->hidden) return;

				c->Update();

				switch (c->GetType())
				{

				case ContainerType::FUTURE: *futureMoney += c->GetMoney(); break;
				case ContainerType::ARREAR: *arrearMoney -= c->GetMoney(); break;
				case ContainerType::CONSTANT:
					for (int i = 0; i < ((ConstContainer*)c)->GetSize(); ++i) *constMoney += ((ConstContainer*)c)->GetLabelLimit(i);
				case ContainerType::FILTER:
				case ContainerType::LIMIT:
				case ContainerType::SAVING:
				default: *totalMoney -= c->GetMoney(); break;
				}
			}
		);

		money = actualMoney + futureMoney + arrearMoney;
	}

	void Draw()
	{

		ImGui::Text("Total Money"); ImGui::SameLine();
		ImGui::PushItemWidth(150.0f);
		ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, format.c_str(), ImGuiSliderFlags_AlwaysClamp);
		ImGui::PopItemWidth();

		ImGui::Text("Balance:"); ImGui::SameLine();
		ImGui::Text(format.c_str(), money);
	}

private:


	void SetFormat(const char* form, const char* currency)
	{
		format.clear();
		format += form;
		format += currency;
	}

public:

	std::string name;

private:

	float money = 0;
	float futureMoney = 0.0f;
	float actualMoney = 0.0f;
	float arrearMoney = 0.0f;
	float constMoney = 0.0f;
	std::string format;
	Array<Container*> containers;

};