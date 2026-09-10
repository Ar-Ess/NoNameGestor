#include "ImGuiExtension.h"

#include "Framework/Engine/App.h"
#include "Framework/Data/String.h"
#include "Framework/Utils/Maths.h"
#include "Framework/Time/TimeSpan.h"

#include "Framework/External/SDL/include/SDL.h"
#include "Framework/Window/Window.h"

#include "NoNameGestor/External/imgui/imgui_internal.h"
#include "NoNameGestor/External/imgui/imgui_stdlib.h"
#include "NoNameGestor/External/ImGuiFileDialog/ImGuiFileDialog.h"

ImFont* ImGui::RS::InputTextFont = nullptr;
ImFont* ImGui::RS::PlusMinusButtonFont = nullptr;

void ImGui::RS::InitializeExtension()
{
	auto& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();

	//TODO: Framework: Assets path must always be a Directory, because /Assets.pak is unreachable and non-useful internally
	String path = App::AssetsPath() + "\\Roboto-Regular.ttf";

	InputTextFont = io.Fonts->AddFontFromFileTTF(path.Str(), 20.f);
	PlusMinusButtonFont = io.Fonts->AddFontFromFileTTF(path.Str(), 20.f);

	io.Fonts->Build();
}

void ImGui::RS::Spacing(unsigned int spaces)
{
	short int plus = 0;
	if (spaces == 0) plus = 1;
	for (unsigned int i = 0; i < spaces * 2 + plus; i++) ImGui::Spacing();
}

void ImGui::RS::Separator(unsigned int separator)
{
	if (separator == 0) return;
	for (unsigned int i = 0; i < separator; i++) ImGui::Separator();
}

void ImGui::RS::Helper(const char* desc, const char* title)
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

void ImGui::RS::ClearInputText(const char* name, std::string* buffer)
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

void ImGui::RS::TextWithEndEllipsis(char const* aString, float aMaxWidth, bool useWordBoundaries, float aSpacing)
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

void ImGui::RS::TextWithStartEllipsis(char const* aString, float aMaxWidth, bool useWordBoundaries, float aSpacing)
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

bool ImGui::RS::DirectoryBrowserField(const char* label, String* browsePath, int& result, float maxWindowWidth, const char* dialogBasePath)
{
	bool ret = false;

	ImGui::BeginGroup();
	{
		if (ImGui::Button("Browse", ImVec2(50, 19)))
		{
			IGFD::FileDialogConfig config;
			config.path = dialogBasePath;
			ImGuiFileDialog::Instance()->OpenDialog(label, "Choose a Directory", nullptr, config);
			result = 0;
			ret = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("R", ImVec2(19, 19)))
		{
			result = 3;
			ret = true;
		}
		ImGui::SameLine(); ImGui::Text(label);

		float width = maxWindowWidth <= 0 ? ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - 20 : maxWindowWidth;
		ImGui::RS::TextWithStartEllipsis(browsePath->Str(), width, false, 0);

	}
	ImGui::EndGroup();

	if (!ImGuiFileDialog::Instance()->IsOpened(label))
		return ret;

	//TODO: Framework: Window doesn't provide a method with the resized size of the window.
	Point winSize = Window::WindowSize();
	ImGui::SetNextWindowSize(ImVec2(winSize.x, winSize.y), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(winSize.x / 2, winSize.y / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGuiFileDialog::Instance()->Display(label, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			*browsePath = (ImGuiFileDialog::Instance()->GetCurrentPath() + '\\').c_str();
			result = 1;
			ret = true;
		}
		else
		{
			result = 2;
			ret = true;
		}

		ImGuiFileDialog::Instance()->Close();
	}

	return ret;
}

bool ImGui::RS::IsSpace(char aCharacter)
{
	// all space characters are values 32 or less (space is 32)
	// so we can convert them to a bitmask and use a single condition
	const int mask = (1 << (' ' - 1)) | (1 << ('\f' - 1)) | (1 << ('\n' - 1)) | (1 << ('\r' - 1)) | (1 << ('\t' - 1)) | (1 << ('\v' - 1));
	return (mask & (1 << ((aCharacter && aCharacter <= 32) * (aCharacter - 1)))) != 0;
}

void ImGui::RS::SectionText(const char* text, unsigned int spacing)
{
	ImGui::Text(text);
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeight() * 0.5f);
	ImGui::Separator();
	ImGui::RS::Spacing(spacing);
}

bool ImGui::RS::SliderCombo(const char* label, int* value, const char* const items[], int itemsLength, float width)
{
	ImGui::BeginGroup();
	if (!String::StartsWith("##", label))
	{
		ImGui::Text(label);
	}
	if (width > 0) ImGui::PushItemWidth(width);
	ImGui::PushID(label);
	bool ret = ImGui::SliderInt("##sliderint", value, 0, itemsLength - 1, "", ImGuiSliderFlags_NoInput);
	ImGui::PopID();
	if (width > 0) ImGui::PopItemWidth();
	ImGui::SameLine(); ImGui::Text(items[*value]);
	ImGui::EndGroup();
	return ret;
}

void ImGui::RS::TimeDisplay(double seconds)
{
	int s = static_cast<int>(std::floor(seconds));

	int h = s / 3600;
	int m = (s % 3600) / 60;
	s = s % 60;

	ImGui::Text("%02d:%02d:%02d", h, m, s);
}

bool ImGui::RS::OneOptionSelectableCombo(const char* labels[], int labelCount, int* selection, int spacing)
{
	if (labels == nullptr)
		return false;

	bool ret = false;
	for (int i = 0; i < labelCount; i++)
	{
		bool s = i == *selection;
		
		ImGui::PushID(labels[i]);
		if (ImGui::Checkbox("", &s))
		{
			*selection = i;
			ret = true;
		}
		ImGui::PopID();
		ImGui::SameLine(0, 6);
		ImGui::Text(labels[i]);

		if (i < labelCount - 1)
			ImGui::SameLine(0, spacing);
	}

	return ret;
}

bool ImGui::RS::DateField(const char* label, DateTime* date)
{
	if (date == nullptr)
		return false;

	int y, m, d;
	date->Date(y, m, d);

	bool ret = false;
	if (!String::StartsWith("##", label)) { ImGui::Text(label); ImGui::SameLine(); }
	ImGui::PushItemWidth(20);
	ImGui::PushID(label);
	ret |= ImGui::DragInt("##daydatefield", &d, 0.35, 1, 31, "%d", ImGuiSliderFlags_ClampOnInput);
	ImGui::SameLine(0, 4); ImGui::Text("/"); ImGui::SameLine(0, 4);
	ret |= ImGui::DragInt("##monthdatefield", &m, 0.2, 1, 12, "%d", ImGuiSliderFlags_ClampOnInput);
	ImGui::SameLine(0, 4); ImGui::Text("/"); ImGui::SameLine(0, 4);
	ImGui::PopItemWidth();
	ImGui::PushItemWidth(36);
	ret |= ImGui::DragInt("##yeardatefield", &y, 0.7, 0, INT_MAX, "%d", ImGuiSliderFlags_ClampOnInput);
	ImGui::PopItemWidth();
	ImGui::PopID();

	if (ret)
	{
		uint64_t maxDay = DateTime::DaysInMonth(y, m);
		d = Maths::Clamp(d, 1, maxDay);
		*date = DateTime::From::Date(y, m, d);
	}

	return ret;
}

void ImGui::RS::CenterNextWindow()
{
	ImVec2 size = ImGui::GetIO().DisplaySize;
	ImGui::SetNextWindowPos(ImVec2(size.x / 2, size.y / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
}

int ImGui::RS::PlusMinusButtonsV(const char* id, int* value, int min, int max, const char* format)
{
	if (value == nullptr)
		return 0;

	int ret = 0;
	ImGui::PushID(id);
	{
		ImGui::BeginGroup();
		{
			ImVec2 pos = ImGui::GetCursorPos();

			ImGui::SetCursorPosY(pos.y - 1);
			ImGui::BeginDisabled(*value == max);
			if (ImGui::Button("##+", ImVec2(9, 9)))
			{
				*value += 1;
				ret = 1;
			}
			ImGui::SetCursorPos(ImVec2(pos.x + 1, pos.y - 4));
			ImGui::Text("+");
			ImGui::EndDisabled();
			ImGui::SetCursorPos(ImVec2(pos.x, pos.y + 11));
			ImGui::BeginDisabled(*value == min);
			if (ImGui::Button("##-", ImVec2(9, 9)))
			{
				*value -= 1;
				ret = 2;
			}
			ImGui::SetCursorPos(ImVec2(pos.x + 1, pos.y + 8));
			ImGui::Text("-");
			ImGui::EndDisabled();

			ImGui::SetCursorPos(pos);
		}
		ImGui::EndGroup();
	}
	ImGui::PopID();

	return ret;
}

int ImGui::RS::PlusMinusButtonsH(const char* id, int* value, int min, int max, const char* format)
{
	if (value == nullptr)
		return 0;

	int ret = 0;
	ImGui::PushID(id);
	{
		ImGui::BeginDisabled(*value == min);
		if (ImGui::Button("-##PlusMinusButtonsH"))
		{
			*value -= 1;
			ret = 2;
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(*value == max);
		if (ImGui::Button("+##PlusMinusButtonsH"))
		{
			*value += 1;
			ret = 1;
		}
		ImGui::EndDisabled();
	}
	ImGui::PopID();

	return ret;
}

int ImGui::RS::MonthSelector(const char* id, int* value, const DateTime& base, DateTime* date, int min, int max)
{
	if (date == nullptr)
		return 0;

	int ret = PlusMinusButtonsH(id, value, min, max, "");

	if (ret > 0)
		*date = base + TimeSpan<>::From::Months(*value);

	ImGui::SameLine();
	ImGui::Text("%s %d", date->MonthName(), date->Year());

	return ret;
}
