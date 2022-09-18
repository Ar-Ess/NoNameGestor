#include "InformativeLog.h"
#include "imgui/imgui.h"

InformativeLog::InformativeLog(float oldInstance, float newInstance, const char* name) : Log(name, oldInstance, LogType::INFORMATIVE_LOG, nullptr)
{
	this->newInstance = newInstance;
}

void InformativeLog::Draw(const char* currency)
{
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Text(name.c_str());
	ImGui::TextColored({ 1, 1, 0.4, 1 }, "%.2f %s", totalInstance, currency);
	ImGui::SameLine(130); ImGui::TextColored({ 1, 1, 0.4, 1 }, "->");
	ImGui::SameLine(200); ImGui::TextColored({ 1, 1, 0.4, 1 }, "%.2f %s", newInstance, currency);
	ImGui::Spacing();
	ImGui::Separator();
}