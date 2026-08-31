#include "EconomyScene.h"

#include "Framework/Scenes/Startup.h"
#include "Framework/Utils/Maths.h"
#include "Framework/External/SDL/include/SDL_events.h"
#include "Framework/External/SDL/include/SDL_render.h"

#include "NoNameGestor/Gestor/GestorSystem.h"
#include "NoNameGestor/Containers/ContainerHeader.h"
#include "NoNameGestor/Containers/ContainerEnum.h"
#include "NoNameGestor/Utils/ImGuiExtension.h"

#include "NoNameGestor/External/imgui/imgui_internal.h"
#include "NoNameGestor/External/ImGuiFileDialog/ImGuiFileDialog.h"
#include "NoNameGestor/External/imgui/imgui_impl_sdl.h"
#include "NoNameGestor/External/imgui/imgui_impl_sdlrenderer.h"

#define NOMINMAX
#include <windows.h>

#define VERSION 1.4f
#define EXTENSION ".nng"
#define NEW_FILE "New_File.nng"

REGISTER_STARTUP_SCENE(EconomyScene);

bool EconomyScene::Awake()
{
	// Create the renderer
	renderer = SDL_CreateRenderer(Window::window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (renderer == nullptr)
	{
		Debug::Log<const char*>("SDL_CreateRenderer failed: %s", SDL_GetError());
		return false;
	}

	// Check correct ImGui version
	IMGUI_CHECKVERSION();

	// Create ImGui context
	ImGui::CreateContext();

	// Configurate io flags
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

	// Set Dark Mode
	ImGui::StyleColorsDark();

	// Initialize ImGui with SDL_Renderer
	if (!ImGui_ImplSDL2_InitForSDLRenderer(Window::window, renderer))
	{
		Debug::Log("ImGui SDL2 backend initialization failed");
		return false;
	}

	// Initialize ImGui Renderer
	if (!ImGui_ImplSDLRenderer_Init(renderer))
	{
		Debug::Log("ImGui SDL Renderer backend initialization failed");
		return false;
	}

	return true;
}

bool EconomyScene::Start()
{
	ImGui::InitializeExtension();

	LoadConfiguration();

	if (!App::IsAppLaunchedWithFile()) // This is without "!"
		LoadInternal(App::OpenedFilePath());
	else
		NewFile();

	return true;
}

bool EconomyScene::Update(float dt)
{
	bool ret = true;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);

		if (event.type == SDL_QUIT)
			ret = false;
	}

	// Generate ImGui Renderer New Frame 
	ImGui_ImplSDLRenderer_NewFrame();
	// Generate SDL2 New Frame
	ImGui_ImplSDL2_NewFrame(Window::window);
	// Generate ImGui New Frame
	ImGui::NewFrame();

	UpdateShortcuts();

	gestors.Iterate([](GestorSystem& g) { g.Update(); });

	return ret;
}

bool EconomyScene::Draw(float dt)
{
	bool ret = true;
	
	// Clear Screen
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	// Draw ImGui
	DrawDocking(ret);
	DrawMenuBar(ret);

	DrawPreferencesWindow(ret);
	DrawMainWindow(ret);
	DrawToolbarWindow(ret);

	//ImGui::ShowDemoWindow();

	if (saving) Save();
	if (loading) Load();
	if (savingAs) SaveAs();

	// ImGui Render
	ImGui::Render();
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderPresent(renderer);

	return true;
}

bool EconomyScene::CleanUp()
{
	gestors.Clear();

	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();

	ImGui::DestroyContext();

	if (renderer != nullptr)
	{
		SDL_DestroyRenderer(renderer);
		renderer = nullptr;
	}

	return true;
}

void EconomyScene::NewFile()
{
	// Clear Gestors
	gestors.Reset();

	// Generate New File
	file.New(NEW_FILE);

	// Generate at least 1 gestor
	NewGestor();
}

void EconomyScene::SaveAs()
{
	if (!savingAs)
	{
		savingAs = true;
		return;
	}
	else
	{
		//TODO: Framework: Add trim function (opposite of Substring, it returns what is not selected by index & count)
		//TODO: Framework: Solve the problem of similarity between Substring static and non-static
		String path = file.IsNew() ? file.Name().Substring(0u, file.Name().Length() - 4) : file.Path().Substring(0u, file.Path().Length() - 4);
		ImGuiFileDialog::Instance()->OpenDialog("SaveAs", "Choose a path", ".nng", path.Str(), 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_DisableCreateDirectoryButton);
	}

	String path;
	//TODO: Framework: Window doesn't provide a method with the resized size of the window.
	ImVec2 size = ImGui::GetIO().DisplaySize;
	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowPos(ImVec2(size.x / 2, size.y / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	//display
	if (!ImGuiFileDialog::Instance()->Display("SaveAs", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
		return; // File dialogue returns false if no action is performed

	// if action is not "Ok"
	if (!ImGuiFileDialog::Instance()->IsOk())
	{
		ImGuiFileDialog::Instance()->Close();
		savingAs = false;
		return;
	}

	// IMPORTANT: Independently of the user's choice to write .nng in the text name or not, it will always appear the extension (.nng) once
	path = ImGuiFileDialog::Instance()->GetFilePathName().c_str();
	//auto a = ImGuiFileDialog::Instance()->GetFilePathName(); // Full path fron C:// to the file name with .nng
	//auto b = ImGuiFileDialog::Instance()->GetCurrentFileName(); // File Name with .nng
	//auto c = ImGuiFileDialog::Instance()->GetCurrentFilter(); // Just extension .nng
	//auto d = ImGuiFileDialog::Instance()->GetCurrentPath(); // Directory without '\\' at the end
	//auto f = ImGuiFileDialog::Instance()->GetOpenedKey(); // Dialogue Key (SaveAs)

	// Disable Save As
	savingAs = false;

	// Close Dialogue
	ImGuiFileDialog::Instance()->Close();

	// Save file in path
	InternalSave(path);
}

void EconomyScene::Save()
{
	if (!saving)
	{
		if (file.IsNew())
			savingAs = true;
		else
			saving = true;

		return;
	}

	saving = false;

	InternalSave(nullptr);
}

void EconomyScene::Backup()
{
	//String backupsPath = App::DataDirectory() + "\\Backups\\";

	//if (!std::filesystem::exists(backupsPath.Str()) && !std::filesystem::create_directories(backupsPath.Str()))
	//{
	//	errorMessage = std::string("Error: it was not possible to create Backups folder in: ") + backupsPath.Str() + strerror(errno);
	//	return;
	//}
	//else if (!std::filesystem::is_directory(backupsPath.Str()))
	//{
	//	errorMessage = std::string("Error: ") + backupsPath.Str() + " is not a folder." + strerror(errno);
	//	return;
	//}

	//backupsPath += openFileName;
	//time_t now = time(0);
	//tm* ltm = localtime(&now);
	//std::string backupText("_Backup_");

	//backupText += std::to_string(ltm->tm_year + 1900) + "-";
	//int month = ltm->tm_mon + 1;
	//int day = ltm->tm_mday;
	//backupText += month < 10 ? "0" + std::to_string(month) + "-" : std::to_string(month) + "-";
	//backupText += day < 10 ? "0" + std::to_string(day) : std::to_string(day);
	//savePath.insert(savePath.length() - 4, backupText.c_str());


	//file->OpenFile(savePath.c_str()).
	//	// Preferences
	//	Write("version").String(VERSION).
	//	Write("cnfSRT").Bool(showContainerType).
	//	Write("cnfSFU").Bool(showFutureUnasigned).
	//	Write("cnfTFS").Number(textFieldSize).
	//	Write("currency").Number(currency).
	//	Write("gestors").Number((int)gestors.size());

	//for (GestorSystem* gestor : gestors)
	//	gestor->Save(file, savePath.c_str());
}

void EconomyScene::InternalSave(StringView path)
{
	bool saveAs = !path.IsNullOrEmpty();

	// If it is a save as
	if (saveAs)
	{
		// Check if it has the correct extension
		if (!FileManager::FileHasExtension(path, EXTENSION))
		{
			errorMessage = "Path to save has not a valid file type. It must be a .nng file: " + path;
			return;
		}

		FileManager::File f = FileManager::OpenFile(path, true);

		// Check file validity
		if (!f.IsValid())
		{
			errorMessage = "Path to SaveAs could not be opened: " + path;
			return;
		}

		file = std::move(f);
	}
	else
	{
		// Check file validity
		if (!file.IsValid())
		{
			file.New(NEW_FILE);
			errorMessage = "Internal Error: Restarting file. Internal File was not valid.";
			return;
		}
	}

	// Clear file json internal data to start over from scratch
	file.Clear();

	// Write to file
	file.Write("version", VERSION);
	file.Write("gestors", FileManager::File::Array);
	auto gnode = file.Access("gestors");

	gestors.Iterate([&](const GestorSystem& g, int i) 
		{ 
			// Push an object representing a new gestor
			int index = gnode.Push(FileManager::File::Object);
			// Assure it is correctly pushed
			Debug::Assert(index != -1, "Internal Save Error: Pushing New Gestor returned an error.");
			// Access to that gestor and send it to be written
			g.Save(i, gnode.Access(index));
		});

	saveAs ? file.SaveAs(path) : file.Save();
}

void EconomyScene::Load()
{
	// Load Logic
	if (!loading)
	{
		loading = true;
		return;
	}
	else
	{
		ImGuiFileDialog::Instance()->OpenDialog("OpenFile", "Choose a file", ".nng", "", 1, nullptr, ImGuiFileDialogFlags_DisableCreateDirectoryButton);
	}

	String path;
	//TODO: Framework: Window doesn't provide a method with the resized size of the window.
	ImVec2 size = ImGui::GetIO().DisplaySize;
	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowPos(ImVec2(size.x / 2, size.y / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	//display
	if (!ImGuiFileDialog::Instance()->Display("OpenFile", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
		return; // File dialogue returns false if no action is performed

	// if action is not "Ok"
	if (!ImGuiFileDialog::Instance()->IsOk())
	{
		ImGuiFileDialog::Instance()->Close();
		loading = false;
		return;
	}

	// IMPORTANT: Independently of the user's choice to write .nng in the text name or not, it will always appear the extension (.nng) once
	path = ImGuiFileDialog::Instance()->GetFilePathName().c_str();
	//auto a = ImGuiFileDialog::Instance()->GetFilePathName(); // Full path fron C:// to the file name with .nng
	//auto b = ImGuiFileDialog::Instance()->GetCurrentFileName(); // File Name with .nng
	//auto c = ImGuiFileDialog::Instance()->GetCurrentFilter(); // Just extension .nng
	//auto d = ImGuiFileDialog::Instance()->GetCurrentPath(); // Directory without '\\' at the end
	//auto f = ImGuiFileDialog::Instance()->GetOpenedKey(); // Dialogue Key (SaveAs)

	// Disable Save As
	loading = false;

	// Close Dialogue
	ImGuiFileDialog::Instance()->Close();

	// Load path
	LoadInternal(path);
}

void EconomyScene::LoadInternal(StringView path)
{
	if (!FileManager::FileExists(path))
	{
		errorMessage = "Invalid path to load: " + path;
		return;
	}

	if (!FileManager::FileHasExtension(path, EXTENSION))
	{
		errorMessage = "Path to load has not a valid file type. It must be a .nng file: " + path;
		return;
	}

	// Open File
	FileManager::File f = FileManager::OpenFile(path);

	// Check file validity
	if (!f.IsValid())
	{
		// Try if it is an old .nng file
		if (OldLoadInternal(path))
			return;

		// If not, then show an error
		errorMessage = "Path to load could not be opened: " + path;
		return;
	}

	// Move valid file
	file = std::move(f);

	// Clear gestors
	gestors.Reset();

	// Check Version
	// Version 1.4f can't load any lower or higher versions
	float version = file.Read<float>("version");
	if (!Maths::Approximately(version, 1.4f))
	{
		errorMessage = String::Format("Invalid version. Program version v1.4 can't load version: %.1f", version);
		return;
	}

	// Load Gestors
	FileManager::FileNode gestorsNode = file.Access("gestors");
	int size = gestorsNode.Length();

	for (unsigned int i = 0; i < size; ++i)
	{
		const FileManager::FileNode node = gestorsNode.Access(i);
		gestors.EmplaceBack(
			i,
			node,
			&file,
			&config,
			&errorMessage
		);
	}

	// Save Path to recentFiles
	SaveRecentPath(path);
}

bool EconomyScene::OldLoadInternal(StringView path)
{
	// Open the file
	std::ifstream f(path.Data());

	// Check if it is correctly opened
	if (!f.is_open())
		return false;

	std::string line;
	
	// Check if the current file is an old .nng format
	std::getline(f, line);
	if (!line.starts_with("version v1.") || !line.ends_with(','))
		return false;

	bool ret = true;

	// Retrieve version - "version v1.x,"
	float version = std::stof(line.substr(9, 3));

	// Check for each version and create a json file for it
	if (Maths::Approximately(version, 1.0f))
	{
		Debug::Log<float>("OldLoader Error: Load for old version %.1f not implemented yet.", version);
		ret = false;
	}
	else if (Maths::Approximately(version, 1.1f))
	{
		Debug::Log<float>("OldLoader Error: Load for old version %.1f not implemented yet.", version);
		ret = false;
	}
	else if (Maths::Approximately(version, 1.2f))
	{
		Debug::Log<float>("OldLoader Error: Load for old version %.1f not implemented yet.", version);
		ret = false;
	}
	else if (Maths::Approximately(version, 1.3f))
	{
		constexpr auto m = std::numeric_limits<std::streamsize>::max();

		// Ignore cnfSRT, cnfSFU and cnfTFS
		for (int i = 0; i < 3; ++i) f.ignore(m, '\n');

		// Retrieve currency - "currency X,"
		std::getline(f, line);
		int currency = std::stoi(line.substr(9, line.length() - 10));

		// Retrieve gestors size - "gestors X,"
		std::getline(f, line);
		int gestorsSize = std::stoi(line.substr(8, line.length() - 9));

		// Reset the gestors vector and reserve the new size
		gestors.Reset();
		gestors.Reserve(gestorsSize);

		// Configure currency
		for (int i = 0; i < 4; ++i)
			config.currency[i] = currency;

		for (int i = 0; i < gestorsSize; ++i)
		{
			// Retrieve name - "name XXXX,"
			std::getline(f, line);
			std::string name(line.substr(5, line.length() - 6));

			// Retrieve input money - "containers X.XX,"
			std::getline(f, line);
			float money = std::stof(line.substr(11, line.length() - 12));

			// Retrieve container size - "size X,"
			std::getline(f, line);
			int containersSize = std::stoi(line.substr(5, line.length() - 6));

			// Construct gestor
			gestors.EmplaceBack(i, name.c_str(), money, &file, &config, &errorMessage);
			GestorSystem& system = gestors.Back();

			for (int j = 0; j < containersSize; ++j)
			{
				// Retrieve name - "name XXXX,"
				std::getline(f, line);
				std::string name(line.substr(5, line.length() - 6));

				// Retrieve type - "type XXXX,"
				std::getline(f, line);
				ContainerType type = (ContainerType)std::stoi(line.substr(5, line.length() - 6));

				// Ignore container money
				f.ignore(m, '\n');

				// Retrieve hide - "hide X,"
				std::getline(f, line);
				bool hide = (bool)std::stoi(line.substr(5, line.length() - 6));

				// Retrieve open - "open X,"
				std::getline(f, line);
				bool open = (bool)std::stoi(line.substr(5, line.length() - 6));

				// Retrieve unified - "unfd X,"
				std::getline(f, line);
				bool unified = (bool)std::stoi(line.substr(5, line.length() - 6));

				// Create container
				Container* c = system.CreateContainer(type, name, hide, open, unified);

				// Retrieve labels size - "size X,"
				std::getline(f, line);
				int labelsSize = std::stoi(line.substr(5, line.length() - 6));

				for (int k = 0; k < labelsSize; ++k)
				{
					// Retrieve label name - "name XXXX,"
					std::getline(f, line);
					std::string labelName(line.substr(5, line.length() - 6));

					// Retrieve label limit - "limit X,"
					float labelLimit = 0;
					if (type == ContainerType::LIMIT)
					{
						std::getline(f, line);
						labelLimit = std::stof(line.substr(6, line.length() - 7));
					}

					// Retrieve label money - "money X,"
					std::getline(f, line);
					float labelMoney = std::stof(line.substr(6, line.length() - 7));

					// Create Label
					switch (type)
					{
					case ContainerType::FILTER:
						((FilterContainer*)c)->NewLabel(labelName.c_str(), labelMoney);
						break;
					case ContainerType::FUTURE:
						((FutureContainer*)c)->NewLabel(labelName.c_str(), labelMoney);
						break;
					case ContainerType::LIMIT:
						((LimitContainer*)c)->NewLabel(labelName.c_str(), labelMoney, labelLimit);
						break;
					}
				}

			}
		}

		file.New(path.Substring(path.FindLast('\\') + 1));
	}
	else
	{
		Debug::Log<float>("OldLoader Error: Unable to load version %.1f", version);
		ret = false;
	}

	return ret;
}

void EconomyScene::LoadConfiguration()
{
	//TODO: Framework Change: Directories must have "\\" at the end
	String path = App::DataDirectory() + "\\config.nng";

	configFile = FileManager::OpenFile(path, true);

	Debug::Assert(configFile.IsValid(), "NONAMEGESTOR ERROR: Can't open or create configFile!");

	config = Configuration();
	recentFiles = Vector<String>(20);

	if (configFile.Length() == 0) // just created
	{
		// User Preferences
		configFile.Write("UP_SCT", config.showContainerType);
		configFile.Write("UP_SFU", config.showFutureUnasigned);
		configFile.Write("UP_TFS", config.textFieldSize);
		configFile.Write("RecentFiles", FileManager::File::Array);
	}
	else
	{
		// User Preferences
		configFile.Read("UP_SCT", config.showContainerType);
		configFile.Read("UP_SFU", config.showFutureUnasigned);
		configFile.Read("UP_TFS", config.textFieldSize);

		// Recent Paths
		auto rNode = configFile.Access("RecentFiles");
		int size = rNode.Length();

		for (int i = 0; i < size; ++i)
			recentFiles.PushBack(rNode.Read<String>(i));
	}
}

void EconomyScene::SaveRecentPath(StringView path)
{
	Debug::Assert(configFile.IsValid(), "INTERNAL ERROR: Config file does not exist!");

	bool change = false;
	auto recentFilesArray = configFile.Access("RecentFiles");

	// Si tenim 20 o més recentFiles, elimina el primer
	if (recentFiles.Size() >= 20)
	{
		recentFiles.PopFront();
		recentFilesArray.Remove(0);
		change = true;
	}

	bool emplace = true;
	// Si el recent files no està buit
	if (!recentFiles.IsEmpty())
	{
		// Mira si existeix un path igual en els recent files
		int index = recentFiles.Find(path);

		// Si existeix un path igual
		if (index != -1)
		{
			// Si l'índex trobat no és l'últim
			if (index != recentFiles.Size() - 1)
			{
				// Elimina'l de recent files i del config file
				recentFiles.Erase(index);
				recentFilesArray.Remove(index);
				change = true;
			}
			// Si l'índex trobat és l'últim
			else
				// No facis un emplace del path perquè ja és l'últim 
				emplace = false;
		}
	}
	
	if (emplace)
	{
		recentFiles.EmplaceBack(path);
		recentFilesArray.Push(path);
		change = true;
	}

	if (change) configFile.Save();
}

void EconomyScene::NewGestor()
{
	if (gestors.Size() >= 4)
		return;

	gestors.EmplaceBack(gestors.Size(), "New Gestor", 0, &file, &config, &errorMessage);
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
				if (recentFiles.IsEmpty())
				{
					ImGui::Text("There is no recent files...");
				}
				else
				{
					recentFiles.Iterate(
						[&](String& recent)
						{
							if (!ImGui::MenuItem(recent.Str()))
								return true;

							LoadInternal(recent);
							return false;
						}
					);
				}

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

				gestors.Iterate(
					[&](const GestorSystem& g) { g.DrawExport(); }
				);

				ImGui::EndMenu();
			}

			ImGui::Separator();

			ImGui::MenuItem("Preferences", "Ctrl + Shft + P", &preferencesWindow);

			ImGui::EndMenu();
		}
		//TODO: Enable the edit menu
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
				((FilterContainer*)gestors[focusedGestor].CreateContainer(ContainerType::FILTER))
				->NewLabel();

			if (ImGui::MenuItem("New Limit"))
				((LimitContainer*)gestors[focusedGestor].CreateContainer(ContainerType::LIMIT))
				->NewLabel();

			if (ImGui::MenuItem("New Future"))
				((FutureContainer*)gestors[focusedGestor].CreateContainer(ContainerType::FUTURE))
				->NewLabel();

			ImGui::AddSeparator();

			ImGui::BeginDisabled(gestors.Size() >= 4);
			if (ImGui::MenuItem("New Gestor"))
				NewGestor();
			ImGui::EndDisabled();

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("About"))
		{
			ImGui::Text("No CurrentName Gestor %s", VERSION); ImGui::SameLine();

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

		if (ImGui::BeginTabBar("##PreferencesTabBar"))
		{
			if (ImGui::BeginTabItem("General"))
			{
				ImGui::AddHelper("Shows, at the side of each container,\na text noting it's type.", "?"); ImGui::SameLine();
				ImGui::Checkbox("Show Container Typology CurrentName", &config.showContainerType);

				ImGui::AddHelper("Shows the unsigned money in terms\nof future income.", "?"); ImGui::SameLine();
				ImGui::Checkbox("Show Unasigned Future Money ", &config.showFutureUnasigned);

				ImGui::AddHelper("Enlarges the size of the text\nlabels of each container.", "?"); ImGui::SameLine();
				ImGui::PushItemWidth(config.textFieldSize);
				ImGui::DragFloat("Text Fiend Size", &config.textFieldSize, 0.1f, 1.0f, 1000.0f, "%f pts", ImGuiSliderFlags_AlwaysClamp);
				ImGui::PopItemWidth();

				ImGui::EndTabItem();
			}

			gestors.Iterate(
				[&](GestorSystem& g, int i)
				{
					ImGui::PushID(g.id.Data());
					if (ImGui::BeginTabItem(g.Name().Data()))
					{
						ImGui::Text("Currency:");
						if (ImGui::Combo("##Currency", &config.currency[i], config.comboCurrency, 5))
							g.SetFormat("%.2f", config.currency[i]);

						ImGui::EndTabItem();
					}
					ImGui::PopID();
				}
			);

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
		ImGui::Text(file.Name().Data());
		unsigned int size = gestors.Size();

		if (size > 0) ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 0.0f));
		if (size > 0 && ImGui::BeginTable("##systemtable", size, ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable))
		{
			// Captura la posició Y de l'inici de la taula
			float tableHeight = (ImGui::GetWindowPos().y + ImGui::GetWindowHeight()) - ImGui::GetCursorScreenPos().y;
			ImVec2 mousePos = ImGui::GetMousePos();

			for (unsigned int i = 0; i < size; ++i)
			{
				GestorSystem& gestor = gestors[i];
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

				gestor.Draw();
			}

			ImGui::EndTable();
			ImGui::PopStyleVar();
		}

		if (!errorMessage.IsNull())
			ImGui::TextColored(ImVec4(1, 0, 0, 1), errorMessage.Str());

		if (!warningMessage.IsNull())
			ImGui::TextColored(ImVec4(1, 1, 0, 1), warningMessage.Str());
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
			((FilterContainer*)gestors[focusedGestor].CreateContainer(ContainerType::FILTER))
				->NewLabel();
			action = true;
		}

		if (ImGui::Button("LIMIT "))
		{
			((LimitContainer*)gestors[focusedGestor].CreateContainer(ContainerType::LIMIT))
				->NewLabel();
			action = true;
		}

		if (ImGui::Button("FUTURE"))
		{
			((LimitContainer*)gestors[focusedGestor].CreateContainer(ContainerType::FUTURE))
				->NewLabel();
			action = true;
		}

		if (action) Container::UpdateOpenState = true;
	}
	ImGui::End();
}

void EconomyScene::UpdateShortcuts()
{
	ctrl = Input::GetKey(KeyCode::LCTRL);
	shft = Input::GetKey(KeyCode::LSHIFT);
	p = Input::GetKeyDown(KeyCode::P);
	s = Input::GetKeyDown(KeyCode::S);
	o = Input::GetKeyDown(KeyCode::O);
	n = Input::GetKeyDown(KeyCode::N);

	if (!ctrl)
		return;

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
