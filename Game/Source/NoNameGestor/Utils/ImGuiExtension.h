#pragma once

#include <string>

struct ImFont;

namespace ImGui
{
	static ImFont* InputTextFont;

	void InitializeExtension();
	
	// Input from 0 (smallest spacing) to whatever you need
	void AddSpacing(unsigned int spaces = 1);

	// Input from 1 to whatever you need
	void AddSeparator(unsigned int separator = 1);

	// Create helper pop up
	void AddHelper(const char* desc, const char* title = "(?)");

	void AddClearInputText(const char* name, std::string* buffer);

	void TextWithEndEllipsis(char const* aString, float aMaxWidth, bool useWordBoundaries, float aSpacing);

	void TextWithStartEllipsis(char const* aString, float aMaxWidth, bool useWordBoundaries, float aSpacing);

	bool IsSpace(char aCharacter);

}