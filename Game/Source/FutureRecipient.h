#pragma once

#include "Recipient.h"
#include "imgui/imgui_stdlib.h"



class FutureRecipient : public Recipient
{
private: //structs

	struct Future
	{
		Future(const char* name, float money)
		{
			this->name = name;
			this->money = money;
		}

		float money;
		std::string name;
	};

public: // Functions

	FutureRecipient(const char* name, float money) : Recipient(name, money, RecipientType::FUTURE)
	{
		NewFuture();
	}

	~FutureRecipient() override
	{
		for (Future f : futures)
		{
			f.name.clear();
			f.name.shrink_to_fit();
		}
		futures.clear();
		futures.shrink_to_fit();
	}

	void Update() override 
	{
		money = 0;
		for (Future f : futures) money += f.money;
	}

	void Draw() override
	{
		size_t size = futures.size();

		for (suint i = 0; i < size; ++i)
		{
			ImGui::PushID(i * -1);

			ImGui::Text(" -  ");
			ImGui::SameLine();

			ImGui::PushItemWidth(150.0f);
			ImGui::InputText("##FutureName", &futures[i].name);
			ImGui::PopItemWidth(); ImGui::SameLine();

			ImGui::PushItemWidth(100.0f);
			ImGui::DragFloat("##Drag", &futures[i].money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, "%.2f EUR"); 
			ImGui::PopItemWidth(); ImGui::SameLine();

			if (size > 1 && ImGui::Button("X")) futures.erase(futures.begin() + i);

			if (i == 0)
			{
				if (size == 1)
				{
					ImGui::SameLine();
					ImGui::Dummy(ImVec2{15, 0});
				}
				ImGui::SameLine();
				if (ImGui::Button("+")) NewFuture();
			}

			ImGui::PopID();
		}
	}

	void NewFuture(const char* name = "New Future", float money = 0.0f)
	{
		futures.push_back(Future(name, money));
	}

	int GetSize() const
	{
		return futures.size();
	}

	float GetFutureMoney(int i) const
	{
		return futures[i].money;
	}
	
	const char* GetFutureName(int i) const
	{
		return futures[i].name.c_str();
	}

private:

	std::vector<Future> futures;
};
