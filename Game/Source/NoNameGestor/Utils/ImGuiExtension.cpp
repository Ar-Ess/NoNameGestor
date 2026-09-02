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

void ImGui::TextWithEndEllipsis(char const* aString, float aMaxWidth, bool useWordBoundaries, float aSpacing)
{
	char const* partStart = aString;
	char const* partEnd = aString;
	ImWchar elipsisChar = ImGui::GetFont()->EllipsisChar;
	char elipsisText[8];
	ImTextStrToUtf8(elipsisText, sizeof(elipsisText), &elipsisChar, (&elipsisChar) + 1);

	if (aSpacing < 0.0f) aSpacing = ImGui::GetStyle().ItemSpacing.x;

	float const ellipsisWidth = ImGui::CalcTextSize(elipsisText).x + aSpacing;
	float width = 0;
	bool addElipsis = false;

	while (*partStart != 0)
	{
		// Add space to next segment

		while (IsSpace(*partEnd)) partEnd++;

		if (useWordBoundaries)
		{
			// get next 'word' by looking for space after non-space
			while (*partEnd != 0 && !IsSpace(*partEnd)) ++partEnd;
		}
		else
		{
			if (*partEnd != 0) ++partEnd;
		}

		ImVec2 const wordSize = ImGui::CalcTextSize(partStart, partEnd);

		// Clearly we have space for this word so just add it
		if (wordSize.x + width + ellipsisWidth < aMaxWidth)
		{
			width += wordSize.x;
			partStart = partEnd;
		}
		// If we're just at the end of the word and we just fit then we can commit here
		else if (*partEnd == 0 && wordSize.x + width < aMaxWidth)
		{
			width += wordSize.x;
			partStart = partEnd;
		}
		// we're done so add elipsis where the current segment starts
		else
		{
			addElipsis = true;
			break;
		}
	}

	ImGui::TextUnformatted(aString, partStart);
	if (addElipsis)
	{
		ImGui::SameLine(0.0f, aSpacing);
		ImGui::TextUnformatted(elipsisText);
	}
}

void ImGui::TextWithStartEllipsis(char const* aString, float aMaxWidth, bool useWordBoundaries, float aSpacing)
{
	ImWchar elipsisChar = ImGui::GetFont()->EllipsisChar;
	char elipsisText[8];
	ImTextStrToUtf8(elipsisText, sizeof(elipsisText), &elipsisChar, (&elipsisChar) + 1);

	if (aSpacing < 0.0f)
		aSpacing = ImGui::GetStyle().ItemSpacing.x;

	float const ellipsisWidth = ImGui::CalcTextSize(elipsisText).x + aSpacing;

	char const* textStart = aString;
	float width = ImGui::CalcTextSize(aString).x;
	bool addElipsis = false;

	while (width + ellipsisWidth > aMaxWidth && *textStart != 0)
	{
		char const* next = textStart;

		if (useWordBoundaries)
		{
			while (*next != 0 && !IsSpace(*next))
			{
				unsigned int c;
				int bytes = ImTextCharFromUtf8(&c, next, NULL);
				next += bytes > 0 ? bytes : 1;
			}

			while (*next != 0 && IsSpace(*next))
			{
				unsigned int c;
				int bytes = ImTextCharFromUtf8(&c, next, NULL);
				next += bytes > 0 ? bytes : 1;
			}
		}
		else
		{
			unsigned int c;
			int bytes = ImTextCharFromUtf8(&c, next, NULL);
			next += bytes > 0 ? bytes : 1;
		}

		width -= ImGui::CalcTextSize(textStart, next).x;

		textStart = next;
		addElipsis = true;
	}

	if (addElipsis)
	{
		ImGui::TextUnformatted(elipsisText);
		ImGui::SameLine(0.0f, aSpacing);
	}

	ImGui::TextUnformatted(textStart);
}

bool ImGui::IsSpace(char aCharacter)
{
	// all space characters are values 32 or less (space is 32)
	// so we can convert them to a bitmask and use a single condition
	const int mask = (1 << (' ' - 1)) | (1 << ('\f' - 1)) | (1 << ('\n' - 1)) | (1 << ('\r' - 1)) | (1 << ('\t' - 1)) | (1 << ('\v' - 1));
	return (mask & (1 << ((aCharacter && aCharacter <= 32) * (aCharacter - 1)))) != 0;
}
