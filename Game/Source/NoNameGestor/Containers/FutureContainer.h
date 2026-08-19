#pragma once

#include "Container.h"

class FutureContainer : public Container
{
public: // Functions

	FutureContainer(const char* name, bool hidden, bool open, bool unified, std::string* format, float* textFieldSize) : Container(name, hidden, open, unified, format, textFieldSize, ContainerType::FUTURE)
	{
		NewLabel("New Future");
	}

	~FutureContainer() override
	{
		ClearLabels();
	}

	void Start(const char* currency) override
	{
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

			if (i == 0) { if (ImGui::Button("+")) NewLabel("New Future"); }
			else ImGui::Dummy({ 33, 0 });

			ImGui::SameLine();

			float width = 100.0f;
			if (!unified)
			{
				if (size > 1) { if (ImGui::Button("X")) DeleteLabel(i); }
				else ImGui::Dummy({ 15, 0 });

				ImGui::SameLine();

				ImGui::PushItemWidth(*textFieldSize);
				ImGui::InputText("##FutureName", &labels[i]->name);
				ImGui::PopItemWidth(); ImGui::SameLine();
			}
			else width += 50;

			ImGui::PushItemWidth(width);
			ImGui::DragFloat("##Drag", &labels[i]->money, 1.0f, 0.0f, MAX_MONEY, (*format).c_str(), ImGuiSliderFlags_AlwaysClamp);
			ImGui::PopItemWidth();

			ImGui::PopID();
		}

		if (hidden) ImGui::EndDisabled();
	}

private:

};
