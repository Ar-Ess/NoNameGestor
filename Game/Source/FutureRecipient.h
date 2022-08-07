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

	FutureRecipient(const char* name, float money, float* totalMoneyPtr) : Recipient(name, money, RecipientType::FUTURE)
	{
		this->totalMoneyPtr = totalMoneyPtr;
		NewFuture();
	}

	~FutureRecipient() override
	{
		ClearFutures();
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
			ImGui::PushID(id * -1 * i);

			if (ImGui::Button(" -> "))
			{
				*totalMoneyPtr += GetFutureMoney(i);
				DeleteFuture(i);
				if (futures.empty()) NewFuture();
				ImGui::PopID();
				break;
			}
			ImGui::SameLine();

			ImGui::PushItemWidth(textFieldSize);
			ImGui::InputText("##FutureName", &futures[i].name);
			ImGui::PopItemWidth(); ImGui::SameLine();

			ImGui::PushItemWidth(100.f);
			ImGui::DragFloat("##Drag", &futures[i].money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, "%.2f EUR"); 
			ImGui::PopItemWidth(); ImGui::SameLine();

			if (size > 1 && ImGui::Button("X")) DeleteFuture(i);

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

	void ClearFutures()
	{
		for (Future f : futures)
		{
			f.name.clear();
			f.name.shrink_to_fit();
		}
		futures.clear();
		futures.shrink_to_fit();
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

	void DeleteFuture(int index)
	{
		futures[index].name.clear();
		futures[index].name.shrink_to_fit();
		futures.erase(futures.begin() + index);
	}

private:

	std::vector<Future> futures;
	float* totalMoneyPtr = nullptr;
};
