#include "ImGuiExtension.h"

#include "Framework/Engine/App.h"
#include "Framework/Data/String.h"

#include "NoNameGestor/External/imgui/imgui_internal.h"
#include "NoNameGestor/External/imgui/imgui_stdlib.h"

ImFont* InputTextFont = nullptr;

void ImGui::InitializeExtension()
{
	ImFontConfig fontConfig;
	fontConfig.SizePixels = 18.0f;
	auto& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();

	//TODO: Framework: Assets path must always be a Directory, because /Assets.pak is unreachable and non-useful internally
	String path = App::AssetsPath() + "\\Roboto-Regular.ttf";

	InputTextFont = io.Fonts->AddFontFromFileTTF(path.Str(), 20.f);
	io.Fonts->Build();
}

void ImGui::AddSpacing(unsigned int spaces)
{
	short int plus = 0;
	if (spaces == 0) plus = 1;
	for (unsigned int i = 0; i < spaces * 2 + plus; i++) ImGui::Spacing();
}

void ImGui::AddSeparator(unsigned int separator)
{
	if (separator == 0) return;
	for (unsigned int i = 0; i < separator; i++) ImGui::Separator();
}

void ImGui::AddHelper(const char* desc, const char* title)
{
	ImGui::TextDisabled(title);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}


void ImGui::AddClearInputText(const char* name, std::string* buffer)
{
	// Guarda l'estat de l'estil actual
	ImGui::PushFont(InputTextFont);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);  // Sense vora
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));  // Opcional: ajusta padding

	// Colors transparents
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));          // Fons transparent
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));   // Quan el cursor està a sobre
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));    // Quan està actiu (editant)

	ImGui::InputText(name, buffer);

	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	float maxTextX = ImGui::CalcTextSize((*buffer).c_str()).x + min.x + 10;
	float y = max.y + 1;
	ImU32 color = ImGui::IsItemActive() ? IM_COL32(80, 140, 255, 255) : IM_COL32(160, 160, 160, 100);
	ImGui::GetWindowDrawList()->AddLine(ImVec2(min.x, y), ImVec2(maxTextX, y), color, 1.5f);

	// Torna a l'estil normal
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);
	ImGui::PopFont();
}