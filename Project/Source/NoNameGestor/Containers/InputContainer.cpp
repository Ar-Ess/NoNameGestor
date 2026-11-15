#include "InputContainer.h"

#include "NoNameGestor/Gestor/Aggregate.h"
#include "NoNameGestor/Gestor/Configuration.h"

#include "NoNameGestor/External/imgui/imgui.h"

InputContainer::InputContainer(const char* name, float money, String* format) :
	Container(name, false, true, false, format, nullptr)
{
	this->money = money;
}

bool InputContainer::Update(Aggregate& agg)
{
	agg.total += money;
	return true;
}

void InputContainer::Draw()
{
	ImGui::PushID(id.Data());
	{
		ImGui::Text("Money: "); ImGui::SameLine();
		ImGui::PushItemWidth(150.0f);
		{
			ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, MAX_MONEY, format->Str(), ImGuiSliderFlags_AlwaysClamp);
		}
		ImGui::PopItemWidth();
	}
	ImGui::PopID();
}
