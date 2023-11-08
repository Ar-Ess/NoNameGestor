#pragma once

#include "Framework/Scene.h"
#include "Framework/Defs.h"
#include "Framework/Array.h"

class Page;

class EconomyScene : public Scene
{
public:

	EconomyScene();
	~EconomyScene();

	bool Start() override;
	bool Update(float dt) override;
	bool Draw(float dt) override;
	bool CleanUp();

	bool DrawMenuBar();
	bool DrawDocking();
	bool DrawPreferencesWindow(bool* open);
	bool DrawMainWindow(bool* open);
	bool DrawToolbarWindow(bool* open);
	bool DrawFileDialog(bool* vError, const char* v, std::string* path, std::string* name, size_t* format, bool* closed);
	
	void NewFile();

	bool ErrorPopup(bool* open, const char* title, const char* description);

private:

	Input* input = nullptr;
	Array<Page*> pages;
	unsigned int index = 0;

	bool demoWindow = false;
	bool preferencesWindow = false;

	// Shortcuts
	bool ctrl = false, shft = false, d = false,
		p = false, s = false, l = false,
		n = false;

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
	const char* comboCurrency[5] = { "EUR", "USD", "COP", "ARS", "PEN" };

	bool openToolbarPopup = false;

};
