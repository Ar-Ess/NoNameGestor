#ifndef __ECONOMY_SCENE_H__
#define __ECONOMY_SCENE_H__

#include "imgui/imgui.h"
#include "Input.h"
#include "FileManager.h"
#include "Chrono.h"
#include "GestorSystem.h"

class EconomyScene
{
public:

	EconomyScene(Input* input, const char* openedFile = nullptr);
	~EconomyScene();

	bool Start();
	bool Update();
	bool Draw();

	bool CleanUp();

private: // Functions

	void NewFile();
	void SaveAs();
	void Save();
	void InternalSave(const char* path);
	void Load();
	void LoadInternal(const char* path);
	void LoadRecentPaths();
	void SaveRecentPath(const char* path);

	bool DrawMenuBar();
	bool DrawDocking();

	bool DrawPreferencesWindow(bool* open);
	bool DrawMainWindow(bool* open);

	bool DrawToolbarWindow(bool* open);

	void UpdateShortcuts();
	void UpdateFormat();

	// Input from 0 (smallest spacing) to whatever you need
	void AddSpacing(unsigned int spaces = 1)
	{
		short int plus = 0;
		if (spaces == 0) plus = 1;
		for (unsigned int i = 0; i < spaces * 2 + plus; i++) ImGui::Spacing();
	}

	// Input from 1 to whatever you need
	void AddSeparator(unsigned int separator = 1)
	{
		if (separator == 0) return;
		for (unsigned int i = 0; i < separator; i++) ImGui::Separator();
	}

	// Create helper pop up
	void AddHelper(const char* desc, const char* title = "(?)")
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

private: // Variables

	// General
	Input* input = nullptr;
	FileManager* file = nullptr;

	// Path List
	std::string rootPath;
	std::vector<std::string*> recentPaths;

	bool preferencesWindow = false;

	// Gestor
	std::vector<GestorSystem*> gestors;

	// Shortcuts
	bool ctrl = false, shft = false, n = false, 
		 p    = false, s    = false, o = false;

	// Preferences
	bool showContainerType = true;
	bool showFutureUnasigned = false;

	int currency = 0;
	const char* comboCurrency[5] = { "EUR", "USD", "COP", "ARS", "PEN"};

	// Save & Load
	bool saving = false, loading = false, savingAs = false;
	bool loadingV1_0 = false;
	bool versionError = false;

	std::string openFileName;
	std::string openFilePath;

	Chrono chrono;
	bool openToolbarPopup = false;
	int focusedGestor = 0;

	ImFont* bigFont = nullptr;

	// Open File Directly
	const char* openedFile = nullptr;

};

#endif //__ECONOMY_SCENE_H__
