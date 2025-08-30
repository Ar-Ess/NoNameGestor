#include "EconomyScene.h"
#include <windows.h>
#include <iostream>
#include <filesystem>

#define VERSION "v1.2"
#define EXTENSION ".nng"
#define RECENT_PATHS "/Config"
#define EXTENSION_CONFIG ".cnfg"
#include "External/ImGuiFileDialog/ImGuiFileDialog.h"
#include "imgui/imgui_internal.h"
#include "ContainerHeader.h"
#include "ContainerEnum.h"


EconomyScene::EconomyScene(Input* input, const char* rootPath, const char* openedFile)
{
	this->input = input;
	this->file = new FileManager(EXTENSION);
	this->openedFile = openedFile;
	this->rootPath = rootPath;
}

EconomyScene::~EconomyScene()
{
}

bool EconomyScene::Start()
{
	LoadFonts(!(openedFile == nullptr || IsDebuggerPresent()));

	if (openedFile == nullptr)
		NewFile();
	else
	{
		std::string openFile(openedFile);
		size_t a = openFile.find_last_of('\\') + 1;
		openFileName = openFile.substr(a, openFile.length());
		openFilePath = openFile.substr(0, a);
		LoadInternal(openedFile);
	}
	
	LoadRecentPaths();
	
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

	DrawDocking(ret);
	DrawMenuBar(ret);

	DrawPreferencesWindow(ret);
	DrawMainWindow(ret);
	DrawToolbarWindow(ret);

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

	gestors.emplace_back(new GestorSystem("New Gestor", &showFutureUnasigned, &showContainerType, &openFileName, &openFilePath, bigFont, &textFieldSize, &errorMessage));

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

void EconomyScene::Backup()
{
	std::string savePath(rootPath);
	savePath += "Backups\\";

	if (!std::filesystem::exists(savePath) && !std::filesystem::create_directories(savePath))
	{
		errorMessage = std::string("Error: it was not possible to create Backups folder in: ") + rootPath + strerror(errno);
		return;
	}
	else if (!std::filesystem::is_directory(savePath))
	{
		errorMessage = "Error: " + savePath + " is not a folder." + strerror(errno);
		return;
	}

	savePath += openFileName;
	time_t now = time(0);
	tm* ltm = localtime(&now);
	std::string backupText("_Backup_");

	backupText += std::to_string(ltm->tm_year + 1900) + "-";
	int month = ltm->tm_mon + 1;
	int day = ltm->tm_mday;
	backupText += month < 10 ? "0" + std::to_string(month) + "-" : std::to_string(month) + "-";
	backupText += day < 10 ? "0" + std::to_string(day) : std::to_string(day);
	savePath.insert(savePath.length() - 4, backupText.c_str());


	file->OpenFile(savePath.c_str()).
		// Preferences
		Write("version").String(VERSION).
		Write("cnfSRT").Bool(showContainerType).
		Write("cnfSFU").Bool(showFutureUnasigned).
		Write("cnfTFS").Number(textFieldSize).
		Write("currency").Number(currency).
		Write("gestors").Number((int)gestors.size());

	for (GestorSystem* gestor : gestors)
		gestor->Save(file, savePath.c_str());
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
		if (ImGuiFileDialog::Instance()->IsOk() == true && file->Exists(ImGuiFileDialog::Instance()->GetFilePathName().c_str()))
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

	LoadInternal(path.c_str());
}

void EconomyScene::LoadInternal(const char* path)
{
	unsigned int size = 0;
	CleanUp();

	SaveRecentPath(path);

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

		GestorSystem* g = new GestorSystem(name.c_str(), &showFutureUnasigned, &showContainerType, &openFileName, &openFilePath, bigFont, &textFieldSize, &errorMessage);
		gestors.emplace_back(g);

		g->Load(file, path, jumplines);
	}

	UpdateFormat();
}

void EconomyScene::LoadRecentPaths()
{
	std::string path(rootPath);
	path += RECENT_PATHS;

	file->SetExtension(EXTENSION_CONFIG);

	if (!file->Exists(path.c_str(), true))
	{
		file->OpenFile(path.c_str()).Write("count").Number(0);
		return;
	}

	int count = 0;
	file->ViewFile(path.c_str()).Read("count").AsInt(count);

	for (unsigned int i = 0; i < count; ++i)
	{
		std::string variableName = "path_";
		variableName += std::to_string(i);
		std::string result;
		file->ViewFile(path.c_str(), i + 1).Read(variableName.c_str()).AsString(result);
		recentPaths.push_back(new std::string(result));
	}

	file->SetExtension(EXTENSION);
}

void EconomyScene::SaveRecentPath(const char* filePath)
{
	std::string* str = new std::string(filePath);

	// Si el path ja existeix a la llista
	for (std::vector<std::string*>::iterator it = recentPaths.begin(); it < recentPaths.end(); ++it)
	{
		if (SameString(*(*it), *str))
			recentPaths.erase(it);
	}

	recentPaths.insert(recentPaths.begin(), str);

	if (recentPaths.size() >= 20)
	{
		delete recentPaths.back();
		recentPaths.pop_back();
	}
	
	std::string path(rootPath);
	path += RECENT_PATHS;

	file->SetExtension(EXTENSION_CONFIG);

	file->OpenFile(path.c_str()).Write("count").Number((int)recentPaths.size());

	int i = 0; 
	for (std::vector<std::string*>::iterator it = recentPaths.begin(); it < recentPaths.end(); ++it)
	{
		std::string newVariableName = "path_";
		newVariableName += std::to_string(i);

		file->EditFile(path.c_str()).Write(newVariableName.c_str()).String((*it)->c_str());
		++i;
	}

	file->SetExtension(EXTENSION);
}

void EconomyScene::DrawDocking(bool& ret)
{
	if (!ret) return;
	ret = true;

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
}

void EconomyScene::DrawMenuBar(bool& ret)
{
	if (!ret) return;
	ret = true;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New File", "Ctrl + N"))
				NewFile();

			ImGui::Separator();

			if (ImGui::MenuItem("Open", "Ctrl + O"))
				Load();

			if (ImGui::BeginMenu("Open Recent"))
			{
				bool thereIsRecentPaths = false;
				for (std::vector<std::string*>::iterator it = recentPaths.begin(); it < recentPaths.end(); ++it)
				{
					thereIsRecentPaths = true;
					if (!ImGui::MenuItem((*it)->c_str())) continue;

					size_t a = (*it)->find_last_of('\\') + 1;
					openFileName = (*it)->substr(a, (*it)->length());
					openFilePath = (*it)->substr(0, a);
					LoadInternal((*it)->c_str());
					break;
				}

				if (!thereIsRecentPaths)
					ImGui::Text("There is no recent files...");

				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Save", "Ctrl + S"))
				Save();

			if (ImGui::MenuItem("Save As", "Ctrl + Shft + S"))
				SaveAs();

			if (ImGui::MenuItem("Backup"))
				Backup();


			ImGui::Separator();

			if (ImGui::BeginMenu("Export"))
			{
				ImGui::Text("Select the gestor: ");
				ImGui::Separator();
				ImGui::Spacing();

				for (GestorSystem* gestor : gestors)
					gestor->DrawExport();

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

			if (ImGui::MenuItem("New Gestor") && gestors.size() < 4)
			{
				gestors.emplace_back(new GestorSystem("New Gestor", &showFutureUnasigned, &showContainerType, &openFileName, &openFilePath, bigFont, &textFieldSize, &errorMessage));
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
}

void EconomyScene::DrawPreferencesWindow(bool& ret)
{
	if (!ret) return;
	ret = true;

	if (!preferencesWindow) return;

	if (ImGui::Begin("Preferences", &preferencesWindow, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
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
}

void EconomyScene::DrawMainWindow(bool& ret)
{
	if (!ret) return;
	ret = true;

	if (ImGui::Begin("##MainWindow", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::Text(openFileName.c_str());
		unsigned int size = gestors.size();

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 0.0f));
		if (ImGui::BeginTable("##systemtable", size, ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable))
		{
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
			ImGui::PopStyleVar();
		}

		ImGui::TextColored(ImVec4(1, 0, 0, 1), errorMessage.c_str());
	}
	ImGui::End();
}

void EconomyScene::DrawToolbarWindow(bool& ret)
{
	if (!ret) return;
	ret = true;

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

void EconomyScene::LoadFonts(bool addFullPath)
{
	ImFontConfig fontConfig;
	fontConfig.SizePixels = 18.0f;
	auto io = ImGui::GetIO();
	io.Fonts->AddFontDefault();

	std::string fontPath(addFullPath ? rootPath : "Assets/Roboto-Regular.ttf");
	if (addFullPath) fontPath += "Assets/Roboto-Regular.ttf";

	bigFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 20.f);
	io.Fonts->Build();
}
