#pragma once

#include "Framework/Data/Flag.h"
#include "Framework/Utils/Nullable.h"
#include "NoNameGestor/Audio/VoiceNode.h"

#include "NoNameGestor/External/imgui/imgui.h"
#include "NoNameGestor/External/imgui/imgui_stdlib.h"

#include <string>

struct Label : public VoiceNode
{
	Label(std::string&& name, float money) :
		VoiceNode(std::move(name), "Label"), money(money)
	{
	}

	virtual Flag Draw(bool unified, bool first, bool single, int textFieldSize, String* format)
	{
		Flag ret = Flag::AllFalse;

		ImGui::PushID(id.Data());

		if (first) { if (ImGui::Button("+")) ret.Set(0, true); }
		else ImGui::Dummy({ 33, 0 });

		ImGui::SameLine();

		float width = 100.0f;
		if (!unified)
		{
			if (!single)
			{
				if (ImGui::Button("X"))
					ret.Set(1, true);
			}
			else ImGui::Dummy({ 15, 0 });

			ImGui::SameLine();

			ImGui::PushItemWidth(textFieldSize);
			{
				ImGui::InputText("##InputName", &name);
			}
			ImGui::PopItemWidth(); ImGui::SameLine();
		}
		else width += 50;


		ImGui::PushItemWidth(width);
		{
			ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, std::numeric_limits<float>::max(), format->Str(), ImGuiSliderFlags_AlwaysClamp);
		}
		ImGui::PopItemWidth();

		ImGui::PopID();

		return ret;
	}

	float money = 0;

	std::vector<VoiceNode*> Children() const override { return std::vector<VoiceNode*>(); }
	virtual std::string Describe(DetailLevel level) const override { return "Label named " + name + " contains " + std::to_string(money); }
	// TODO: Describe better
};
