#pragma once

#include "Framework/Scenes/Scene.h"
#include "NoNameGestor/Utils/Chrono.h"
#include <string>

class FileManager;
class GestorSystem;
struct ImFont;

class EconomyScene : public Scene
{
public:

	EconomyScene();
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
	void Backup();
	void Load();
	void LoadInternal(const char* path);
	void LoadRecentPaths();
	void SaveRecentPath(const char* path);

	void DrawMenuBar(bool& ret);
	void DrawDocking(bool& ret);

	void DrawPreferencesWindow(bool& ret);
	void DrawMainWindow(bool& ret);

	void DrawToolbarWindow(bool& ret);

	void UpdateShortcuts();
	void UpdateFormat();

	void LoadFonts(bool addFullPath);

	// Input from 0 (smallest spacing) to whatever you need
	void AddSpacing(unsigned int spaces = 1);

	// Input from 1 to whatever you need
	void AddSeparator(unsigned int separator = 1);

	// Create helper pop up
	void AddHelper(const char* desc, const char* title = "(?)");

private: // Variables

	// General
	FileManager* file = nullptr;

	// Path List
	const char* rootPath = nullptr;
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
	float textFieldSize = 150.f;

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

	std::string errorMessage;

};
