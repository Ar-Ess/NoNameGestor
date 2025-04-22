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
}

EconomyScene::~EconomyScene()
{
}

bool EconomyScene::Start()
{
	ImFontConfig fontConfig;
	fontConfig.SizePixels = 18.0f;
	auto io = ImGui::GetIO();

	io.Fonts->AddFontDefault();
	bigFont = io.Fonts->AddFontFromFileTTF("Assets/Roboto-Regular.ttf", 20.f);
	io.Fonts->Build();

	NewFile();

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
	for (GestorSystem* gestor : gestors)
		RELEASE(gestor);
	gestors.clear();

	return true;
}

void EconomyScene::NewFile()
{
	openFileName = "New_File";
	openFileName += EXTENSION;
	openFilePath.clear();

	CleanUp();

	gestors.emplace_back(new GestorSystem("NewGestor", &showFutureUnasigned, &showContainerType, bigFont));

	UpdateFormat();
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
		Write("cnfTFS").Number(textFieldSize).
		Write("currency").Number(currency).
		Write("gestors").Number((int)gestors.size());

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
	std::string path, name, version;
	size_t format = 0;
	bool closed = false;
	ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose a path", ".nng", ".");
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk() == true && file->Exists(ImGuiFileDialog::Instance()->GetFilePathName().c_str(), false))
		{
			path = ImGuiFileDialog::Instance()->GetCurrentPath() + "\\";
			name = ImGuiFileDialog::Instance()->GetCurrentFileName();
			format = ImGuiFileDialog::Instance()->GetCurrentFilter().size();
			ImGuiFileDialog::Instance()->Close();

			// Check if the version file is the same as the program version
			std::string checkPath = path;
			checkPath += name;
			checkPath.erase(checkPath.end() - format, checkPath.end());
			file->ViewFile(checkPath.c_str()).
				Read("version").AsString(version);
			bool errorMissmatch = !SameString(VERSION, version);
		}
		// windows closed
		else
		{
			loading = false;
			return;
		}
	}
	else return;

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
	unsigned int size = 0;
	CleanUp();

	// Y aspects
	file->ViewFile(path, 1).
		// Preferences
		//Read("version") // 0
		Read("cnfSRT").AsBool(showContainerType). // 1
		Read("cnfSFU").AsBool(showFutureUnasigned). // 2
		Read("cnfTFS").AsFloat(textFieldSize). // 3
		Read("currency").AsInt(currency). // 4
		Read("gestors").AsInt(size); // 5

	// The following line to read is 6
	int jumplines = 6; // Update if more preferences added on top /\

	for (unsigned int i = 0; i < size; ++i)
	{
		std::string name;
		file->ViewFile(path, jumplines).
			Read("name").AsString(name);

		jumplines++;

		GestorSystem* g = new GestorSystem(name.c_str(), &showFutureUnasigned, &showContainerType, bigFont);
		gestors.emplace_back(g);

		g->Load(file, path, jumplines);
	}

	UpdateFormat();
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

			if (ImGui::MenuItem("Open", "Ctrl + O"))
				Load();

			if (ImGui::MenuItem("Save", "Ctrl + S"))
				Save();

			if (ImGui::MenuItem("Save As", "Ctrl + Shft + S"))
				SaveAs();


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
			if (ImGui::MenuItem("New Filter"))
				gestors[focusedGestor]->CreateContainer(ContainerType::FILTER);

			if (ImGui::MenuItem("New Limit"))
				gestors[focusedGestor]->CreateContainer(ContainerType::LIMIT);

			if (ImGui::MenuItem("New Future"))
				gestors[focusedGestor]->CreateContainer(ContainerType::FUTURE);

			AddSeparator(1);

			if (ImGui::MenuItem("New Gestor"))
			{
				gestors.emplace_back(new GestorSystem("New Gestor", &showFutureUnasigned, &showContainerType, bigFont));
				UpdateFormat();
			}

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

				AddHelper("Enlarges the size of the text\nlabels of each container.", "?"); ImGui::SameLine();
				ImGui::PushItemWidth(textFieldSize);
				ImGui::DragFloat("Text Fiend Size", &textFieldSize, 0.1f, 1.0f, 1000.0f, "%f pts", ImGuiSliderFlags_AlwaysClamp);
				ImGui::PopItemWidth();

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
		unsigned int size = gestors.size();

		if (ImGui::BeginTable("##systemtable", size, ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable))
		{
			//for (int i = 0; i < size; ++i)
			//	ImGui::TableSetupColumn(std::to_string(i));
			//ImGui::TableHeadersRow();

			// Captura la posició Y de l'inici de la taula
			float tableHeight = (ImGui::GetWindowPos().y + ImGui::GetWindowHeight()) - ImGui::GetCursorScreenPos().y;
			ImVec2 mousePos = ImGui::GetMousePos();

			for (unsigned int i = 0; i < size; ++i)
			{
				GestorSystem* gestor = gestors[i];
				ImGui::TableNextColumn();

				ImVec2 cellMin = ImGui::GetCursorScreenPos();
				ImVec2 cellMax = ImVec2(cellMin.x + ImGui::GetColumnWidth(), cellMin.y + tableHeight);

				// Detectem si el clic ha estat dins la zona
				bool mouseInside =
					mousePos.x >= cellMin.x && mousePos.x <= cellMax.x &&
					mousePos.y >= cellMin.y && mousePos.y <= cellMax.y;

				if (mouseInside && ImGui::IsMouseClicked(0))
					focusedGestor = i;

				if (focusedGestor == i) {
					ImGui::GetWindowDrawList()->AddLine(
						ImVec2(cellMin.x, cellMin.y - 2),
						ImVec2(cellMax.x, cellMin.y - 2),
						IM_COL32(80, 140, 255, 200),
						2.0f
					);
				}

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
			gestors[focusedGestor]->CreateContainer(ContainerType::FILTER);
			action = true;
		}

		if (ImGui::Button("LIMIT "))
		{
			gestors[focusedGestor]->CreateContainer(ContainerType::LIMIT);
			action = true;
		}

		if (ImGui::Button("FUTURE"))
		{
			gestors[focusedGestor]->CreateContainer(ContainerType::FUTURE);
			action = true;
		}

		if (action) gestors[focusedGestor]->SwitchLoadOpen();
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
	o    = input->GetKey(SDL_SCANCODE_O) == KeyState::KEY_DOWN;
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
			if (o) Load();
			if (n) NewFile();
		}
	}
}

void EconomyScene::UpdateFormat()
{
	for (GestorSystem* system : gestors)
		system->SetFormat("%.2f ", comboCurrency[currency]);

	//inputContainer->SetCurrency(comboCurrency[currency]);
	//totalContainer->SetCurrency(comboCurrency[currency]);
	//for (Container* r : containers) r->SetCurrency(comboCurrency[currency]);
}
