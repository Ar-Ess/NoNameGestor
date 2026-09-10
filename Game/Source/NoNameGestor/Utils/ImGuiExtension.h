#pragma once

#include <string>
#include "Framework/Data/String.h"
#include "Framework/Time/DateTime.h"

struct ImFont;

namespace ImGui::RS
{
	extern ImFont* InputTextFont;
	extern ImFont* PlusMinusButtonFont;

	void InitializeExtension();

	// Input from 0 (smallest spacing) to whatever you need
	void Spacing(unsigned int spaces = 1);

	// Input from 1 to whatever you need
	void Separator(unsigned int separator = 1);

	// Create helper pop up
	void Helper(const char* desc, const char* title = "(?)");

	void ClearInputText(const char* name, std::string* buffer);

	void TextWithEndEllipsis(char const* aString, float aMaxWidth, bool useWordBoundaries, float aSpacing);

	void TextWithStartEllipsis(char const* aString, float aMaxWidth, bool useWordBoundaries, float aSpacing);

	/// <summary>
	/// Creates a directory browser to find a directory with a file dialog.
	/// DirectoryBrowser states are:
	/// "Browse" button is clicked (result = 0).
	/// "Ok" button is clicked in the File Dialog and thus new directory is selected (result = 1).
	/// "Cancel" button or quitting the File Dialog (result = 2).
	/// "Restore" button is clicked (result = 3).
	/// </summary>
	/// <param name="name">- Name of the browser directory.</param>
	/// <param name="browsePath">- String to retrieve the selected path.</param>
	/// <param name="result">- Output int that returns the different states in which the directory browser can be on.</param>
	/// <param name="maxWindowWidth">- Maximum width in which the selected path can be shown.</param>
	/// <param name="dialogBasePath">- To which path will the file dialog open when browsing.</param>
	/// <returns>True if any of the 4 states is triggered; otherwise false.</returns>
	bool DirectoryBrowserField(const char* label, String* browsePath, int& result, float maxWindowWidth = 0, const char* dialogBasePath = ".");

	bool IsSpace(char aCharacter);

	void SectionText(const char* text, unsigned int spacing = 1);

	bool SliderCombo(const char* label, int* value, const char* const items[], int itemsLength, float width = 0);

	void TimeDisplay(double seconds);

	bool OneOptionSelectableCombo(const char* labels[], int labelCount, int* selection, int spacing = -1);

	bool DateField(const char* label, DateTime* date);

	void CenterNextWindow();

	int PlusMinusButtonsV(const char* id, int* value, int min, int max, const char* format = "%d");
	int PlusMinusButtonsH(const char* id, int* value, int min, int max, const char* format = "%d");

	int MonthSelector(const char* id, int* value, const DateTime& base, DateTime* date, int min, int max);

}