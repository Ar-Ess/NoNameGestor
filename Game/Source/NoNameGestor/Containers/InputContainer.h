#pragma once

#include "Container.h"

class InputContainer : public Container
{
public: // Functions

	InputContainer(const char* name, std::string* format) : Container(name, false, true, false, format, nullptr, ContainerType::INPUT_MONEY)
	{
	}

	void Draw() override 
	{
		ImGui::PushID(id);
		ImGui::Text(name.c_str()); ImGui::SameLine();
		ImGui::PushItemWidth(150.0f);
		ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, MAX_MONEY, (*format).c_str(), ImGuiSliderFlags_AlwaysClamp);
		ImGui::PopItemWidth();
		ImGui::PopID();
	}

};

