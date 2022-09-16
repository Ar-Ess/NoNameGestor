#ifndef __ECONOMY_SCENE_H__
#define __ECONOMY_SCENE_H__

#include <vector>
#include "imgui/imgui.h"
#include "Input.h"
#include "ContainerHeader.h"
#include "FileManager.h"
#include "MethodsEnum.h"
#include "Chrono.h"

struct File
{
	File(const char* fileName = nullptr, const char* filePath = nullptr)
	{
		openFileName.clear();
		openFilePath.clear();
		if (fileName) openFileName = fileName;
		if (filePath) openFilePath = filePath;
	}

	std::string openFileName;
	std::string openFilePath;
};

class EconomyScene
{
public:

	EconomyScene(Input* input);
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
	void Loadv1_0();

	bool DrawMenuBar();
	bool DrawDocking();

	bool DrawPreferencesWindow(bool* open);
	bool DrawMainWindow(bool* open);
		void DrawGestorSystem();
		void DrawLogSystem();

	bool DrawToolbarWindow(bool* open);

	void UpdateShortcuts();
	void UpdateCurrency();

	void CreateContainer(ContainerType container, const char* name = "New Container", bool hidden = false, bool open = false);

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

	void DeleteContainer(suint index)
	{
		assert(index >= 0 && index < containers.size());

		Container* r = containers[index];
		containers.erase(containers.begin() + index);
		RELEASE(r);
	}

	void DeleteAllContainer()
	{
		for (Container* r : containers) RELEASE(r);
		containers.clear();
		containers.shrink_to_fit();
	}

	void MoveContainer(suint index, suint position)
	{
		if (index == position) return;
		suint size = containers.size();
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

	void SetMethod();

private: // Variables

	TotalContainer* totalContainer = nullptr;
	UnasignedContainer* unasignedContainer = nullptr;
	std::vector<Container*> containers;

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
	bool createContainerUnified = true;

	int currency = 0;
	const char* comboCurrency[5] = { "EUR", "USD", "COP", "ARS", "PEN"};

	// Save & Load
	bool saving = false, loading = false, savingAs = false;
	bool loadingV1_0 = false;
	bool versionError = false;

	std::vector<File> openFiles;

	std::string openFileName;
	std::string openFilePath;

	// Others
	Method method = Method::MTHD_NO;

	Chrono chrono;
	bool openToolbarPopup = false;

};

#endif //__ECONOMY_SCENE_H__
