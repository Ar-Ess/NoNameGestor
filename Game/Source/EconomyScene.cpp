#include "EconomyScene.h"
#include <windows.h>
#include <iostream>

#define VERSION "v1.2"
#define EXTENSION ".nng"
#include "External/ImGuiFileDialog/ImGuiFileDialog.h"
#include "imgui/imgui_internal.h"
#include "ContainerHeader.h"
#include "ContainerEnum.h"


EconomyScene::EconomyScene(Input* input)
{
	this->input = input;
	this->file = new FileManager(EXTENSION);
	
	gestors.emplace_back(new GestorSystem(&showFutureUnasigned, &allowFutureCovering, &showArrearUnasigned, &allowArrearsFill, &showConstantTotal, &showContainerType));

	openFileName = "New_File";
	openFileName += EXTENSION;
	openFilePath.clear();
}

EconomyScene::~EconomyScene()
{
}

bool EconomyScene::Start()
{
	UpdateFormat();

	return true;
}

bool EconomyScene::Update()
{
	UpdateShortcuts();

	for (GestorSystem* gestor : gestors)
		gestor->Update();

	return true;
}

bool EconomyScene::Draw()
{
	bool ret = true;

	ret = DrawDocking();
	ret = DrawMenuBar();

	ret = DrawPreferencesWindow(&preferencesWindow);
	ret = DrawMainWindow(&ret);
	ret = DrawToolbarWindow(&ret);

	//ImGui::ShowDemoWindow();

	if (saving) Save();
	if (loading) Load();
	if (savingAs) SaveAs();

	return ret;
}

bool EconomyScene::CleanUp()
{
	return true;
}

void EconomyScene::NewFile()
{
	openFileName = "New_File";
	openFileName += EXTENSION;
	openFilePath.clear();

	for (GestorSystem* gestor : gestors)
		RELEASE(gestor);
	gestors.clear();

	gestors.emplace_back(new GestorSystem(&showFutureUnasigned, &allowFutureCovering, &showArrearUnasigned, &allowArrearsFill, &showConstantTotal, &showContainerType));
}

void EconomyScene::SaveAs()
{
	if (!savingAs)
	{
		savingAs = true;
		return;
	}

	// open Dialog Simple
	ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose a path", ".nng", ".");
	std::string path;
	std::string name;
	size_t format = 0;
	//display
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			path = ImGuiFileDialog::Instance()->GetCurrentPath() + "\\";
			name = ImGuiFileDialog::Instance()->GetCurrentFileName();
			format = ImGuiFileDialog::Instance()->GetCurrentFilter().size();
			ImGuiFileDialog::Instance()->Close();
		}
		else
		{
			ImGuiFileDialog::Instance()->Close();
			savingAs = false;
			return;
		}
	}
	else
	{
		return;
	}

	savingAs = false;

	openFilePath = path;

	if (name.empty())
		path += openFileName;
	else
	{
		openFileName.clear();
		openFileName.shrink_to_fit();
		openFileName = name.c_str();
		name.erase(name.end() - format, name.end());
		path += name;
	}

	InternalSave(path.c_str());
}

void EconomyScene::Save()
{
	if (!saving)
	{
		if (openFilePath.empty()) savingAs = true;
		else
			saving = true;

		return;
	}

	saving = false;

	std::string savePath = openFilePath + openFileName;

	InternalSave(savePath.c_str());

}

void EconomyScene::InternalSave(const char* path)
{
	file->OpenFile(path).
		// Preferences
		Write("version").String(VERSION).
		Write("cnfSRT").Bool(showContainerType).
		Write("cnfSFU").Bool(showFutureUnasigned).
		Write("cnfAFC").Bool(allowFutureCovering).
		Write("cnfSAU").Bool(showArrearUnasigned).
		Write("cnfAAF").Bool(allowArrearsFill).
		Write("cnfSCT").Bool(showConstantTotal).
		Write("cnfCCU").Bool(createContainerUnified).
		Write("cnfTFS").Number(textFieldSize).
		Write("currency").Number(currency);

	for (GestorSystem* gestor : gestors)
		gestor->Save(file, path);

}

void EconomyScene::Load()
{
	// Load Logic
	if (!loading)
	{
		loading = true;
		return;
	}

	// Draw File Dialog
	std::string path, name;
	size_t format = 0;
	bool closed = false;
	if (!DrawFileDialog(&versionError, VERSION, &path, &name, &format, &closed)) return;
	else
	{
		if (closed)
		{
			loading = false;
			return; // Return true
		}
	}

	// Load
	loading = false;

	openFileName = name;
	openFilePath = path;

	path += name;
	path.erase(path.end() - format, path.end());

	LoadInternal(path.c_str());
}

void EconomyScene::LoadInternal(const char* path)
{
	// Y aspects
	file->ViewFile(path, 1).
		// Preferences
		//Read("version") // 0
		Read("cnfSRT").AsBool(showContainerType). // 1
		Read("cnfSFU").AsBool(showFutureUnasigned). // 2
		Read("cnfAFC").AsBool(allowFutureCovering). // 3
		Read("cnfSAU").AsBool(showArrearUnasigned). // 4
		Read("cnfAAF").AsBool(allowArrearsFill). // 5
		Read("cnfSCT").AsBool(showConstantTotal). // 6
		Read("cnfCCU").AsBool(createContainerUnified). // 7
		Read("cnfTFS").AsFloat(textFieldSize). // 8
		Read("currency").AsInt(currency); // 9

	// The following line to read is 10
	int jumplines = 10; // Update if more preferences added on top /\ 

	for (GestorSystem* gestor : gestors)
		gestor->Load(file, path, jumplines);

	UpdateFormat();
}

bool EconomyScene::DrawFileDialog(bool* vError, const char* v, std::string* path, std::string* name, size_t* format, bool* closed)
{
	if (!*vError)
	{
		std::string version;
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose a path", ".nng", ".");
		if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
		{
			// action if OK
			if (ImGuiFileDialog::Instance()->IsOk() == true && file->Exists(ImGuiFileDialog::Instance()->GetFilePathName().c_str(), false))
			{
				*path = ImGuiFileDialog::Instance()->GetCurrentPath() + "\\";
				*name = ImGuiFileDialog::Instance()->GetCurrentFileName();
				*format = ImGuiFileDialog::Instance()->GetCurrentFilter().size();
				ImGuiFileDialog::Instance()->Close();

				// Check if the version file is the same as the program version
				std::string checkPath = *path;
				checkPath += *name;
				checkPath.erase(checkPath.end() - *format, checkPath.end());
				file->ViewFile(checkPath.c_str()).
					Read("version").AsString(version);
				*vError = !SameString(v, version);
				return true;
			}
			else
			{
				ImGuiFileDialog::Instance()->Close();
				*closed = true;
				return true; // Return true
			}
		}
		else return false; // Return false
	}
}

void EconomyScene::ExportGestor(std::vector<Container*>* exporting)
{
	std::fstream file;
	std::string filePath = openFilePath;
	if (filePath.empty()) filePath = "Exports\\";
	filePath += openFileName;
	filePath.erase(filePath.end() - 4, filePath.end());
	filePath += "_Gestor.txt";

	file.open(filePath, std::ios::out);

	assert(file.is_open()); // File is not open

	std::vector<Container*>::const_iterator it = exporting->begin();
	for (it; it != exporting->end(); ++it)
	{
		file << (*it)->GetName() << ":";
		if ((*it)->unified)
		{
			file << " " << (*it)->GetMoney() << " " << comboCurrency[currency] << std::endl << std::endl;
		}
		else
		{
			file << std::endl;
			unsigned int size = (*it)->GetSize();
			for (unsigned int i = 0; i < size; ++i)
			{
				file << " - " << (*it)->GetLabelName(i) << ": " << (*it)->GetLabelMoney(i) << " " << comboCurrency[currency] << std::endl << std::endl;
			}
		}
	}

	file.close();
}

bool EconomyScene::DrawMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New File", "Ctrl + N"))
				NewFile();

			ImGui::Separator();

			if (ImGui::MenuItem("Save", "Ctrl + S"))
				Save();

			if (ImGui::MenuItem("Save As", "Ctrl + Shft + S"))
				SaveAs();

			if (ImGui::MenuItem("Load", "Ctrl + L"))
				Load();

			ImGui::Separator();

			if (ImGui::BeginMenu("Export"))
			{
				// TODO: adapt this to accept different amounts of gestors
				/*
				bool empty = containers.empty();
				bool selected = false;
				if (!empty)
				{
					ImGui::Text("Select the containers:");
					AddSpacing(1);
					for (Container* c : containers)
					{
						ImGui::Text("  - ");
						ImGui::SameLine();
						ImGui::PushID(c->GetId());
						ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
						ImGui::MenuItem(c->GetName(), "", &c->exporting);
						ImGui::PopItemFlag();
						ImGui::PopID();
						if (!selected && c->exporting) selected = true;
					}
				}
				else
				{
					ImGui::TextDisabled("No containers yet:");
					AddSpacing(3);
				}
				AddSpacing(1);
				AddSeparator();
				if (!selected || empty) ImGui::BeginDisabled();

				if (ImGui::Selectable("  Export", false, ImGuiSelectableFlags_None, { 70, 14 }))
				{
					std::vector<Container*> toExport;
					for (Container* c : containers)
					{
						if (!c->exporting) continue;
						toExport.emplace_back(c);
					}
					ExportGestor(&toExport);
				}
				if (!selected && !empty) ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::Text("|");
				ImGui::SameLine();
				if (ImGui::Selectable("Export All", false, ImGuiSelectableFlags_None, { 70, 14 }))
					ExportGestor(&containers);
				if (empty) ImGui::EndDisabled();
				*/
				ImGui::EndMenu();
			}

			ImGui::Separator();

			ImGui::MenuItem("Preferences", "Ctrl + Shft + P", &preferencesWindow);

			ImGui::EndMenu();
		}
		/*if (ImGui::BeginMenu("Edit"))
		{
			ImGui::Text("Undo/Redo Future Implementation");
			//ImGui::MenuItem("Undo", "Ctrl + Z");
			//ImGui::MenuItem("Redo", "Ctrl + Shft + Z");
			ImGui::Separator();
			ImGui::Text("Copy/Paste/Cut/Duplicate Future Implementation");
			//ImGui::MenuItem("Copy", "Ctrl + C");
			//ImGui::MenuItem("Paste", "Ctrl + V");
			//ImGui::MenuItem("Cut", "Ctrl + X");
			//ImGui::MenuItem("Duplicate", "Ctrl + D");
			ImGui::EndMenu();
		}*/
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Filter"))
				gestors[0]->CreateContainer(ContainerType::FILTER);

			if (ImGui::MenuItem("Limit"))
				gestors[0]->CreateContainer(ContainerType::LIMIT);

			if (ImGui::MenuItem("Future"))
				gestors[0]->CreateContainer(ContainerType::FUTURE);

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("About"))
		{
			ImGui::Text("No Name Gestor %s", VERSION); ImGui::SameLine();

			if (ImGui::Selectable(">")) 
				ShellExecute(NULL, NULL, "https://github.com/Ar-Ess/NoNameGestor", NULL, NULL, SW_SHOWNORMAL);

			if (ImGui::BeginMenu("Third Parties"))
			{
				if (ImGui::MenuItem("SDL - App Loop"))
					ShellExecute(NULL, NULL, "https://www.libsdl.org/", NULL, NULL, SW_SHOWNORMAL);

				if (ImGui::MenuItem("ImGui - Graphic UI"))
					ShellExecute(NULL, NULL, "https://github.com/ocornut/imgui", NULL, NULL, SW_SHOWNORMAL);

				if (ImGui::MenuItem("ImGuiFileDialog - FileDialog"))
					ShellExecute(NULL, NULL, "https://github.com/aiekick/ImGuiFileDialog", NULL, NULL, SW_SHOWNORMAL);

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}
	}
	ImGui::EndMainMenuBar();

	return true;
}

bool EconomyScene::DrawDocking()
{
	bool ret = true;

	ImGuiDockNodeFlags dockspace_flags = (ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoResize);
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("Docking", (bool*)0, (ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus));

	ImGui::PopStyleVar();

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & (ImGuiConfigFlags_DockingEnable))
	{
		ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	ImGui::End();

	return ret;
}

bool EconomyScene::DrawPreferencesWindow(bool* open)
{
	bool ret = true;
	if (!(*open)) return ret;

	if (ImGui::Begin("Preferences", open, ImGuiWindowFlags_NoDocking))
	{
		ImGui::Spacing();

		ImGui::Text("Currency:");
		if (ImGui::Combo("##Currency", &currency, comboCurrency, 5))
			UpdateFormat();

		ImGui::Spacing();

		if (ImGui::BeginTabBar("##PreferencesTabBar"))
		{
			if (ImGui::BeginTabItem("Gestor"))
			{
				AddHelper("Shows, at the side of each container,\na text noting it's type.", "?"); ImGui::SameLine();
				ImGui::Checkbox("Show Container Typology Name", &showContainerType);

				AddHelper("Shows the unsigned money in terms\nof future income.", "?"); ImGui::SameLine();
				ImGui::Checkbox("Show Unasigned Future Money ", &showFutureUnasigned);

				if (showFutureUnasigned)
				{
					ImGui::Dummy(ImVec2{ 6, 0 }); ImGui::SameLine();
					AddHelper("Allows future money to cover\nactual money whenever it goes\nin negative numbers.\nIMPORTANT:\nUse this option if you know for sure\nyou'll receive the future income.", "?"); ImGui::SameLine();
					ImGui::Checkbox("Allow Future Money Covering ", &allowFutureCovering);
				}

				AddHelper("Shows the unsigned money in terms\nof arrears outcome.", "?"); ImGui::SameLine();
				ImGui::Checkbox("Show Unasigned Arrears Money ", &showArrearUnasigned);

				if (showArrearUnasigned)
				{
					ImGui::Dummy(ImVec2{ 6, 0 }); ImGui::SameLine();
					AddHelper("Allows unasigned actual money to fill\nunasigned arrears whenever they exist.", "?"); ImGui::SameLine();
					ImGui::Checkbox("Allow Arrears Money Filling", &allowArrearsFill);
				}

				AddHelper("Shows the total money in terms\nof constant outcome.", "?"); ImGui::SameLine();
				ImGui::Checkbox("Show Total Constrant Money ", &showConstantTotal);

				AddHelper("Enlarges the size of the text\nlabels of each container.", "?"); ImGui::SameLine();
				ImGui::PushItemWidth(textFieldSize);
				ImGui::DragFloat("Text Fiend Size", &textFieldSize, 0.1f, 1.0f, 1000.0f, "%f pts", ImGuiSliderFlags_AlwaysClamp);
				ImGui::PopItemWidth();

				AddHelper("Creates all containers unified by default", "?"); ImGui::SameLine();
				ImGui::Checkbox("Create Container Unified Default", &createContainerUnified);

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

	}
	ImGui::End();

	return ret;
}

bool EconomyScene::DrawMainWindow(bool* open)
{
	bool ret = true;
	if (!(*open)) return ret;

	if (ImGui::Begin("##MainWindow", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::Text(openFileName.c_str());

		if (ImGui::BeginTable("##systemtable", 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable))
		{
			for (GestorSystem* gestor : gestors)
			{
				ImGui::TableNextColumn();
				gestor->Draw();
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();

	return ret;
}

bool EconomyScene::DrawToolbarWindow(bool* open)
{
	bool ret = true;
	if (!(*open)) return ret;

	if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		bool action = false;

		if (ImGui::Button("FILTER"))
		{
			gestors[0]->CreateContainer(ContainerType::FILTER);
			action = true;
		}

		if (ImGui::Button("LIMIT "))
		{
			gestors[0]->CreateContainer(ContainerType::LIMIT);
			action = true;
		}

		if (ImGui::Button("FUTURE"))
		{
			gestors[0]->CreateContainer(ContainerType::FUTURE);
			action = true;
		}

		if (action) gestors[0]->SwitchLoadOpen();
	}
	ImGui::End();

	return ret;
}

void EconomyScene::UpdateShortcuts()
{
	ctrl = input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_REPEAT;
	shft = input->GetKey(SDL_SCANCODE_LSHIFT) == KeyState::KEY_REPEAT;

	p    = input->GetKey(SDL_SCANCODE_P) == KeyState::KEY_DOWN;
	s    = input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_DOWN;
	l    = input->GetKey(SDL_SCANCODE_L) == KeyState::KEY_DOWN;
	n    = input->GetKey(SDL_SCANCODE_N) == KeyState::KEY_DOWN;

	if (ctrl)
	{
		if (shft)
		{
			if (p) preferencesWindow = !preferencesWindow;
			if (s) SaveAs();
		}
		else
		{
			if (s) Save();
			if (l) Load();
			if (n) NewFile();
		}
	}
}

void EconomyScene::UpdateFormat()
{
	for (GestorSystem* system : gestors)
		system->SetFormat("%.2f ", comboCurrency[currency]);

	//totalContainer->SetCurrency(comboCurrency[currency]);
	//unasignedContainer->SetCurrency(comboCurrency[currency]);
	//for (Container* r : containers) r->SetCurrency(comboCurrency[currency]);
}

// Old Loads -------------------------------------------
//void EconomyScene::Loadv1_0()
//{
//	// Load Logic
//	if (!loadingV1_0)
//	{
//		loadingV1_0 = true;
//		return;
//	}
//
//	// Draw File Dialog
//	std::string path, name;
//	size_t format = 0;
//	bool closed = false;
//	if (!DrawFileDialog(&versionError, "v1.0", &path, &name, &format, &closed)) return;
//	else
//	{
//		if (closed)
//		{
//			loadingV1_0 = false;
//			return; // Return true
//		}
//	}
//
//	// Load
//	loadingV1_0 = false;
//
//	openFileName = name;
//	openFilePath = path;
//
//	path += name;
//	path.erase(path.end() - format, path.end());
//
//	float total = 0;
//	int size = 0;
//
//	DeleteAllContainer();
//
//	// Y aspects
//	file->ViewFile(path.c_str(), 1).
//		// Preferences
//		Read("cnfSRT").AsBool(showContainerType).
//		Read("cnfSFU").AsBool(showFutureUnasigned).
//		Read("cnfAFC").AsBool(allowFutureCovering).
//		Read("cnfTFS").AsFloat(textFieldSize).
//		Read("currency").AsInt(currency).
//		// General Project
//		Read("total").AsFloat(total).
//		Read("size").AsInt(size);
//
//	int added = 0;
//
//	for (unsigned int i = 0; i < size; ++i)
//	{ //Change depednig X aspects \/   \/ Change it depending on how many Y aspects
//		int positionToRead = (i * 5) + 8 + added;
//		int type = -1;
//		float money = 0;
//		bool hidden = false, open = false;
//		std::string name;
//
//		// X aspects
//		file->ViewFile(path.c_str(), positionToRead).
//			Read("name").AsString(name).
//			Read("type").AsInt(type).
//			Read("money").AsFloat(money).
//			Read("hide").AsBool(hidden).
//			Read("open").AsBool(open);
//
//
//		switch ((ContainerType)type)
//		{
//		case ContainerType::FILTER:
//		{
//			FilterContainer* fR = (FilterContainer*)gestors[0]->CreateContainer((ContainerType)type, "Filter Name", hidden, open, createContainerUnified);
//			// Change: v1.1 -> v1.0 (Filters are plural forever)
//			fR->ClearLabels();
//
//			fR->NewLabel(name.c_str(), money);
//
//			break;
//		}
//
//		case ContainerType::LIMIT:
//		{
//			float limit = 1;
//			file->ViewFile(path.c_str(), positionToRead + 4).
//				Read("limit").AsFloat(limit);
//
//			LimitContainer* lR = (LimitContainer*)gestors[0]->CreateContainer((ContainerType)type, "Limit Name", hidden, open, createContainerUnified);
//			// Change: v1.1 -> v1.0 (Limits are plural forever)
//			lR->ClearLabels();
//
//			lR->NewLabel(name.c_str(), money, limit);
//			added++;
//
//			break;
//		}
//
//		case ContainerType::FUTURE:
//		{
//			FutureContainer* fR = (FutureContainer*)gestors[0]->CreateContainer((ContainerType)type, name.c_str(), hidden, open, createContainerUnified);
//			fR->ClearLabels();
//
//			int fSize = 0; //                           \/ Change depending on X aspects amount
//			int futurePositionToRead = positionToRead + 5;
//
//			file->ViewFile(path.c_str(), futurePositionToRead).
//				Read("size").AsInt(fSize);
//
//			added++;
//
//			for (suint i = 0; i < fSize; ++i)
//			{
//				std::string fName;
//				float fMoney;
//				file->ViewFile(path.c_str(), (futurePositionToRead + 1) + (i * 2)).
//					Read("name").AsString(fName).
//					Read("money").AsFloat(fMoney);
//
//				fR->NewLabel(fName.c_str(), fMoney);
//
//				added += 2;
//			}
//
//			break;
//		}
//
//		default: break;
//		}
//
//		containers.back()->loadOpen = true;
//	}
//
//	totalContainer->SetMoney(total);
//	UpdateFormat();
//
//}
