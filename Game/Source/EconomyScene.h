#pragma once

#include "Framework/Scene.h"
#include "Framework/Defs.h"
#include "Framework/Array.h"

#include "imgui/imgui.h"

class Page;

//--------
#include "FileManager.h"
//-------

class EconomyScene : public Scene
{
public:

	EconomyScene();
	~EconomyScene();

	bool Start() override;
	bool Update(float dt) override;
	bool Draw(float dt) override;

	bool CleanUp();

private:

	Array<Page*> pages;
	unsigned int index = 0;

// ===============================================================
private: // Functions --------------------------------------------

	void NewFile();
	bool DrawFileDialog(bool* vError, const char* v, std::string* path, std::string* name, size_t* format, bool* closed);
	bool ErrorPopup(bool* open, const char* title, const char* description);
	//void ExportGestor(std::vector<Container*>* exporting);
//	void ExportLog s(unsigned int start, unsigned int end);

	bool DrawMenuBar();
	bool DrawDocking();

	bool DrawPreferencesWindow(bool* open);
	bool DrawMainWindow(bool* open);
		void DrawGestorSystem();
		void DrawLogSystem(bool checkMismatch);

	bool DrawToolbarWindow(bool* open);

	void UpdateCurrency();

	//void CreateContainer(ContainerType container, const char* name = "New Container", bool hidden = false, bool open = true);

private: // Variables

	// General
	Input* input = nullptr;
	FileManager* file = nullptr;

	bool demoWindow = false;
	bool preferencesWindow = false;

	// Shortcuts
	bool ctrl = false, shft = false, d = false, 
		 p    = false, s    = false, l = false,
		 n    = false;

	// Preferences
	bool showContainerType = true;
	bool showFutureUnasigned = false;
	bool allowFutureCovering = false;
	bool showArrearUnasigned = false;
	bool allowArrearsFill = false;
	bool showConstantTotal = false;
	bool createContainerUnified = true;
	bool dateFormatType = true;

	int currency = 0;
	const char* comboCurrency[5] = { "EUR", "USD", "COP", "ARS", "PEN"};

	int maxLogs = 30;

	// Save & Load
	bool saving = false, loading = false, savingAs = false;
	bool loadingV1_0 = false;
	bool versionError = false;

	std::string openFileName;
	std::string openFilePath;

	bool openToolbarPopup = false;

};
