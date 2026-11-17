#pragma once

#include "Framework/Data/Flag.h"
#include "Framework/Utils/Nullable.h"
#include "NoNameGestor/Audio/VoiceNode.h"

#include "NoNameGestor/External/imgui/imgui.h"
#include "NoNameGestor/External/imgui/imgui_stdlib.h"

#include <string>
#include <format>

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
	virtual std::string Describe(DetailLevel level) const override
	{
		std::string ret;
		switch (level)
		{
		case DetailLevel::BRIEF:
			ret = "Label named " + name + ", contains " + std::format("{:.2f}", money);
			break;
		case DetailLevel::NORMAL:
			//ret = std::string("We are currently in a NNG file called ") + file.Name().Data()
			//	+ ", which has " + std::to_string(gestors.size()) + " gestors in it.";
			break;
		case DetailLevel::FULL:
		{
			//ret = std::string("We are currently in NoNameGestor app, with an open NNG file called ") + file.Name().Data()
			//	+ ". It has a total amount of " + std::to_string(gestors.size()) + " gestors in it. ";
			//static std::string numerals[4] = { "The first one", "The second one", "The third one", "The forth one" };
			//int i = 0;
			//for (GestorSystem* g : gestors)
			//	ret += numerals[i++] + " is called " + g->Name().data() + ". ";

			break;
		}
		default:
			ret = "";
			break;
		}
		return ret;
	}
	// TODO: Describe better
	virtual std::string Locate(DetailLevel level) const override
	{
		return "I am in a label named " + name;
	}
};
