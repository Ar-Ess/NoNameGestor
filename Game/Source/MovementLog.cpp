#include "MovementLog.h"
#include "imgui/imgui.h"

MovementLog::MovementLog(float money, float totalInstance, const char* name, bool* dateFormat) : Log(name, totalInstance, LogType::MOVEMENT_LOG, dateFormat)
{
	this->money = money;
	money >= 0 ? color[1] = 255 : color[0] = 255;
}

void MovementLog::Draw(const char* currency)
{
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Text(name.c_str());
	if (date[0] != 0)
	{
		int index[2] = { 0, 1 };
		if (!*dateFormat)
		{
			index[0] = 1;
			index[1] = 0;
		}
		ImGui::SameLine(200);
		ImGui::Text("%d / %d / %d", date[index[0]], date[index[1]], date[2]);
	}
	ImGui::TextColored(TO_COLOR(color), "%.2f %s", money, currency);
	ImGui::SameLine(200); ImGui::Text("%.2f %s", totalInstance, currency);
	ImGui::Spacing();
	ImGui::Separator();
}

float MovementLog::GetOldInstance() const
{
	return totalInstance - money;
}

float MovementLog::GetNewInstance() const
{
	return totalInstance;
}
