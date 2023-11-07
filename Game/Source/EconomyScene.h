#pragma once

#include "Framework/Scene.h"
#include "Framework/Defs.h"
#include "Framework/Array.h"

#include "imgui/imgui.h"

class Page;

//--------
#include <vector>
#include "FileManager.h"
#include "Chrono.h"

#include "ContainerHeader.h"
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
	void SaveAs();
	void Save();
	void InternalSave(const char* path);
	void Load();
	void LoadInternal(const char* path);
	void Loadv1_0();
	bool DrawFileDialog(bool* vError, const char* v, std::string* path, std::string* name, size_t* format, bool* closed);
	bool ErrorPopup(bool* open, const char* title, const char* description);
	void ExportGestor(std::vector<Container*>* exporting);
	void ExportLogs(unsigned int start, unsigned int end);

	bool DrawMenuBar();
	bool DrawDocking();

	bool DrawPreferencesWindow(bool* open);
	bool DrawMainWindow(bool* open);
		void DrawGestorSystem();
		void DrawLogSystem(bool checkMismatch);

	bool DrawToolbarWindow(bool* open);

	void UpdateCurrency();

	void CreateContainer(ContainerType container, const char* name = "New Container", bool hidden = false, bool open = true);

	void CreateInformative(float oldInstance, const char* information)
	{
		//logs.emplace_back(new InformativeLog(oldInstance, totalContainer->GetMoney(), information));
		CheckLogLeaking();
	}

	void CheckLogLeaking()
	{
		if (logs.size() > maxLogs)
		{
			Log* erase = logs.front();
			logs.erase(logs.begin());
			RELEASE(erase);
		}
	}

	void CheckLogMismatch();

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

	void DeleteContainer(unsigned int index)
	{
		assert(index >= 0 && index < containers.size());

		Container* r = containers[index];
		containers.erase(containers.begin() + index);
		RELEASE(r);
		SwitchLoadOpen();
	}

	void DeleteAllContainer()
	{
		for (Container* r : containers) RELEASE(r);
		containers.clear();
	}

	void DeleteAllLogs()
	{
		for (Log* l : logs) RELEASE(l);
		logs.clear();
	}

	void MoveContainer(unsigned int index, unsigned int position)
	{
		if (index == position) return;
		unsigned int size = containers.size();
		assert(index >= 0 && index < size);
		assert(position >= 0 && position < size);

		Container* r = containers[index];
		containers.erase(containers.begin() + index);

		containers.insert(containers.begin() + position, r);
	}

	int ReturnContainerIndex(intptr_t id)
	{
		size_t size = containers.size();
		int i = 0;
		for (std::vector<Container*>::const_iterator it = containers.begin(); it != containers.end(); ++it)
		{
			Container* rTarget = (*it);
			if (id == rTarget->GetId()) return i;
			++i;
		}

		i = -1;
		assert(i != -1); // There is not a container like "r"

		return i;
	}

	void SwitchLoadOpen()
	{
		for (Container* c : containers) c->loadOpen = true;
	}

private: // Variables

	// General
	Input* input = nullptr;
	FileManager* file = nullptr;

	bool demoWindow = false;
	bool preferencesWindow = false;

	// Gestor
	std::vector<Container*> containers;

	//  Logs
	std::vector<Log*> logs;

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

	Chrono chrono;
	bool openToolbarPopup = false;

};
