#include "TotalContainer.h"

#include "NoNameGestor/Gestor/Aggregate.h"
#include "NoNameGestor/Gestor/Configuration.h"

#include "NoNameGestor/External/imgui/imgui.h"
//#include "NoNameGestor/External/imgui/imgui_stdlib.h"
//#include "NoNameGestor/External/imgui/imgui_internal.h"

TotalContainer::TotalContainer(const char* name, String* format, Configuration* config) :
	Container(name, false, true, false, format, config)
{
}

bool TotalContainer::Update(Aggregate& agg)
{
	money = agg.total - agg.assigned + agg.future;
	leftMoney = agg.total - agg.assigned;
	futureMoney = agg.future;

	return true;
}

void TotalContainer::Draw()
{
	ImGui::PushID(id.Data());
	{
		if (!config->showFutureUnassigned)
		{
			ImGui::Text("Total: "); ImGui::SameLine();
			ImGui::Text(format->Str(), money);
		}
		else
		{
			ImGui::Text("Actual Total: "); ImGui::SameLine();
			ImGui::Text(format->Str(), double(leftMoney));

			ImGui::Text("Future Total: "); ImGui::SameLine();
			ImGui::Text(format->Str(), futureMoney);
		}
	}
	ImGui::PopID();
}
