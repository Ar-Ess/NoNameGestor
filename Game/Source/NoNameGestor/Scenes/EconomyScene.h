#pragma once

#include "Framework/Scenes/Scene.h"

#include "NoNameGestor/Gestor/GestorSystem.h"
#include "NoNameGestor/Gestor/Configuration.h"
#include "NoNameGestor/Utils/FileManager.h"
#include "NoNameGestor/Utils/Chrono.h"

class FileManager;
struct ImFont;
struct SDL_Renderer;

class EconomyScene : public Scene
{
public:

	bool Awake() override;
	bool Start() override;
	bool Update(float dt) override;
	bool Draw(float dt) override;
	bool CleanUp() override;

private: // Functions

	void NewFile();

	void SaveAs();
	void Save();
	void InternalSave(StringView path, FileManager::File& file);
	void Backup();
	void UpdateAutomaticBackup();
	void Load();
	void LoadInternal(StringView path);
	bool OldLoadInternal(StringView path);
	void LoadConfiguration();
	void SaveRecentPath(StringView path);
	void NewGestor();

	void DrawMenuBar(bool& ret);
	void DrawDocking(bool& ret);

	void DrawPreferencesWindow(bool& ret);
	void DrawMainWindow(bool& ret);

	void DrawToolbarWindow(bool& ret);

	void UpdateShortcuts();

private: // Variables

	// Files
	FileManager::File file;
	FileManager::File configFile;

	// Config
	Configuration config;

	// Gestor
	Vector<GestorSystem*, true> gestors;
	int focusedGestor = 0;

	// Shortcuts
	bool ctrl = false, shft = false, n = false, 
		 p    = false, s    = false, o = false;

	// Save & Load
	bool saving = false, loading = false, savingAs = false;

	// Error Handling
	String errorMessage;
	String warningMessage;

	// Internals
	SDL_Renderer* renderer = nullptr;
	bool preferencesWindow = false;
	Chrono backupChrono;

};
