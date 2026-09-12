#include "EconomyScene.h"

#include "Framework/Engine/App.h"
#include "Framework/Scenes/Startup.h"
#include "Framework/Utils/Maths.h"
#include "Framework/External/SDL/include/SDL_events.h"
#include "Framework/External/SDL/include/SDL_render.h"

#include "NoNameGestor/Gestor/GestorSystem.h"
#include "NoNameGestor/Containers/ContainerHeader.h"
#include "NoNameGestor/Containers/ContainerEnum.h"
#include "NoNameGestor/Utils/ImGuiExtension.h"

#include "NoNameGestor/External/ImGuiFileDialog/ImGuiFileDialog.h"
#include "NoNameGestor/External/imgui/imgui_impl_sdl2.h"
#include "NoNameGestor/External/imgui/imgui_impl_sdlrenderer2.h"

#define NOMINMAX
#include <windows.h>

#include "Framework/Time/TimeSpan.h"
#include "NoNameGestor/External/imgui/imgui_internal.h"

#define VERSION 1.4f
#define EXTENSION ".nng"
#define NEW_FILE "New_File.nng"
#define DEFAULT_BACKUP_CREATION_DIRECTORY App::DataDirectory() + "Backups\\"
#define DEFAULT_FILE_DIALOG_DIRECTORY "C:\\"

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
	if (!ImGui_ImplSDLRenderer2_Init(renderer))
	{
		Debug::Log("ImGui SDL Renderer backend initialization failed");
		return false;
	}

	return true;
}

bool EconomyScene::Start()
{
	ImGui::RS::InitializeExtension();

	LoadConfiguration();

	if (App::IsAppLaunchedWithFile())
		LoadInternal(App::OpenedFilePath());
	else
		NewFile();

	startDate = App::OpenAppTime;
	endDate = App::OpenAppTime + TimeSpan<>::From::Months(12);

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
		{
			ret = false;
			// If Auto Backup when App Closing enabled, back up file (not backup a backup, neither a new file if not enabled)
			if (config.autoCloseAppBackup && file.IsValid() && !file.IsBackup() && (config.backupUnsavedFiles || !file.IsNew()))
				Backup();
		}
	}

	// Generate ImGui Renderer New Frame 
	ImGui_ImplSDLRenderer2_NewFrame();
	// Generate SDL2 New Frame
	ImGui_ImplSDL2_NewFrame();
	// Generate ImGui New Frame
	ImGui::NewFrame();

	UpdateShortcuts();
	UpdateAutomaticBackup();

	gestors.Iterate([](GestorSystem* g) { g->Update(); });

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
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
	SDL_RenderPresent(renderer);

	return true;
}

bool EconomyScene::CleanUp()
{
	gestors.Clear();

	ImGui_ImplSDLRenderer2_Shutdown();
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
		//TODO: Framework: App::Directory and path thingis should return a string view, not a string!

		//TODO: Framework: Add trim function (opposite of Substring, it returns what is not selected by index & count)
		//TODO: Framework: Solve the problem of similarity between Substring static and non-static
		IGFD::FileDialogConfig fConfig;
		fConfig.path = file.IsNew() ? (!App::DebugMode ? App::DataDirectory().Data() : config.defaultDialogDirectory.Str()) : file.Path().Substring(0u, file.Path().Length() - 4).Data();
		fConfig.flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_DisableCreateDirectoryButton;
		fConfig.fileName = file.Name().Data();
		ImGuiFileDialog::Instance()->OpenDialog("SaveAs", "Choose a path", ".nng", fConfig);
	}

	String path;
	//TODO: Framework: Window doesn't provide a method with the resized size of the window.
	int width, height;
	SDL_GetWindowSize(Window::window, &width, &height);
	ImGui::SetNextWindowSize(ImVec2((float)width, (float)height), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(width / 2, height / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

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
	InternalSave(path, file);
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

	InternalSave(nullptr, file);
}

void EconomyScene::Backup()
{
	if (!config.backupUnsavedFiles && (!file.IsValid() || file.IsNew()))
	{
		//TODO: Error messaging handling with time of error vanishing
		errorMessage = "Can not backup an unsaved file. Please first save the current file or enable the functionality on Preferences.";
		return;
	}

	if (!FileManager::DirectoryExists(config.backupDirectory) && !FileManager::DirectoryCreate(config.backupDirectory))
	{
		errorMessage = "Error: it was not possible to create Backups folder in: " + config.backupDirectory + strerror(errno);
	}

	String info = DateTime::From::Now().ToString("_Backup_yyyy-MM-dd-HH-mm-ss");
	String path = config.backupDirectory + file.Name();
	path.Insert(info, path.Length() - 4);

	FileManager::File backup(true);
	InternalSave(path, backup);

	if (!backup.IsValid())
		errorMessage = "Unable to backup properly. File: " + path;
}

void EconomyScene::UpdateAutomaticBackup()
{
	if (!config.autoIntervalAppBackup || file.IsBackup())
		return;

	if (file.IsNew() && !config.backupUnsavedFiles)
		return;

	if (backupChrono.ChronoSec(config.comboIntervalValue[config.autoIntervalAppValue]))
	{
		Backup();
	}
}

void EconomyScene::InternalSave(StringView path, FileManager::File& file)
{
	bool backup = file.IsBackup();
	bool saveAs = !path.IsNullOrEmpty() || backup;

	// If it is a save as
	if (saveAs)
	{
		Debug::Assert(!(path.IsNullOrEmpty() && backup), "Internal Save Error: If saving a backup, a valid path must be provided. Can't Save() a backup, only SaveAs(path).");

		// Check if it has the correct extension
		if (!FileManager::FileHasExtension(path, EXTENSION))
		{
			errorMessage = "Path to save has not a valid file type. It must be a .nng file: " + path;
			return;
		}

		FileManager::FileInfo f = FileManager::FindFile(path, true);

		// Check file validity
		if (!f.IsValid())
		{
			errorMessage = "Path to SaveAs could not be found: " + path;
			return;
		}

		file = FileManager::OpenFile(f);

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
	file.Write("backup", backup);
	file.Write("gestors", FileManager::File::Array);
	auto gnode = file.Access("gestors");

	gestors.Iterate([&](const GestorSystem* g, int i) 
		{ 
			// Push an object representing a new gestor
			int index = gnode.Push(FileManager::File::Object);
			// Assure it is correctly pushed
			Debug::Assert(index != -1, "Internal Save Error: Pushing New Gestor returned an error.");
			// Access to that gestor and send it to be written
			g->Save(i, gnode.Access(index));
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
		IGFD::FileDialogConfig fConfig;
		fConfig.path = config.defaultDialogDirectory.Str();
		fConfig.flags = ImGuiFileDialogFlags_DisableCreateDirectoryButton;
		ImGuiFileDialog::Instance()->OpenDialog("OpenFile", "Choose a file", ".nng", fConfig);
	}

	String path;
	//TODO: Framework: Window doesn't provide a method with the resized size of the window.
	int width, height;
	SDL_GetWindowSize(Window::window, &width, &height);
	ImGui::SetNextWindowSize(ImVec2((float)width, (float)height), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(width / 2, height / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

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

	// If Auto Backup when File Closing enabled, back up file (not backup a backup, neither a new file if not enabled)
	if (config.autoCloseFileBackup && file.IsValid() && !file.IsBackup() && (config.backupUnsavedFiles || !file.IsNew()))
		Backup();

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

	bool isBackup = file.Read<bool>("backup");
	if (isBackup) FileManager::ToBackup(file);

	// Load Gestors
	FileManager::FileNode gestorsNode = file.Access("gestors");
	int size = gestorsNode.Length();

	for (unsigned int i = 0; i < size; ++i)
	{
		const FileManager::FileNode node = gestorsNode.Access(i);
		gestors.PushBack(
			new GestorSystem(
				i,
				node,
				&file,
				&config,
				&errorMessage
			)
		);
	}

	// Save Path to recentFiles
	if (!isBackup)
	{
		SaveRecentPath(file.Path());
		if (config.autoOpenFileBackup)
			Backup();
	}

	backupChrono.ChronoStop();
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
			gestors.PushBack(new GestorSystem(i, name.c_str(), money, &file, &config, &errorMessage));
			GestorSystem* system = gestors.Back();

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
				Container* c = system->CreateContainer(type, name, hide, open, unified);

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
	String path = App::DataDirectory() + "config.nng";

	configFile = FileManager::OpenFile(path, true);

	if (!configFile.IsValid())
	{
		// Config file was present but incorrectly formatted
		// Reset the file completely from scratch
		std::ofstream config(path.Str(), std::ios::out | std::ios::trunc);
		config << "{}";
		config.close();
		configFile = FileManager::OpenFile(path, true);
		Debug::Assert(configFile.IsValid(), "NONAMEGESTOR ERROR: Can't open or create configFile!");
		warningMessage = "Internal Error With User Preferences: Restarting user values to default.";
	}

	config = Configuration();

	if (configFile.Length() == 0) // just created
	{
		// User Preferences
		configFile.Write("UP_SCT", config.showContainerType);
		configFile.Write("UP_SFU", config.showFutureUnassigned);
		configFile.Write("UP_TFS", config.textFieldSize);
		configFile.Write("UP_BUF", false); // Backup Unsaved Files?
		configFile.Write("UP_BCD", String::Empty); // Backup Creation Directory
		configFile.Write("UP_DDD", String::Empty); // Default Dialog Directory
		configFile.Write("UP_ABS", false); // Automatic Backups System
		configFile.Write("UP_AOFB", false); // Automatic Opening File Backup
		configFile.Write("UP_ACFB", false); // Automatic Closing File Backup
		configFile.Write("UP_ACAP", false); // Automatic Closing App Backup
		configFile.Write("UP_AIAB", false); // Automatic Interval App Backup
		configFile.Write("UP_AIAV", 3); // Automatic Interval App Value

		// Internal Data
		configFile.Write("ID_RFA", FileManager::File::Array); // Recent Files Array

		configFile.Save();
	}
	else
	{
		// User Preferences
		configFile.Read("UP_SCT", config.showContainerType);
		configFile.Read("UP_SFU", config.showFutureUnassigned);
		configFile.Read("UP_TFS", config.textFieldSize);
		configFile.Read("UP_BUF", config.backupUnsavedFiles);
		configFile.Read("UP_BCD", config.backupDirectory);
		if (config.backupDirectory.IsNullOrEmpty())
			config.backupDirectory = DEFAULT_BACKUP_CREATION_DIRECTORY;
		configFile.Read("UP_DDD", config.defaultDialogDirectory);
		if (config.defaultDialogDirectory.IsNullOrEmpty())
			config.defaultDialogDirectory = DEFAULT_FILE_DIALOG_DIRECTORY;
		configFile.Read("UP_AOFB", config.autoOpenFileBackup);
		configFile.Read("UP_ACFB", config.autoCloseFileBackup);
		configFile.Read("UP_ACAB", config.autoCloseAppBackup);
		configFile.Read("UP_AIAB", config.autoIntervalAppBackup);
		configFile.Read("UP_AIAV", config.autoIntervalAppValue);

		// Internal Data
		config.recentFiles = Vector<String>(20);
		auto rNode = configFile.Access("ID_RFA");
		int size = rNode.Length();
		for (int i = 0; i < size; ++i)
			config.recentFiles.PushBack(rNode.Read<String>(i));
	}
}

void EconomyScene::SaveRecentPath(StringView path)
{
	Debug::Assert(configFile.IsValid(), "INTERNAL ERROR: Config file does not exist!");

	bool change = false;
	auto recentFilesArray = configFile.Access("ID_RFA");

	// Si tenim 20 o més recentFiles, elimina el primer
	if (config.recentFiles.Size() >= 20)
	{
		config.recentFiles.PopFront();
		recentFilesArray.Remove(0);
		change = true;
	}

	bool emplace = true;
	// Si el recent files no està buit
	if (!config.recentFiles.IsEmpty())
	{
		// Mira si existeix un path igual en els recent files
		int index = config.recentFiles.Find(path);

		// Si existeix un path igual
		if (index != -1)
		{
			// Si l'índex trobat no és l'últim
			if (index != config.recentFiles.Size() - 1)
			{
				// Elimina'l de recent files i del config file
				recentFilesArray.Remove(index);
				config.recentFiles.Erase(index);
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
		recentFilesArray.Push(path);
		config.recentFiles.EmplaceBack(path);
		change = true;
	}

	if (change) configFile.Save();
}

void EconomyScene::NewGestor()
{
	if (gestors.Size() >= 4)
		return;

	gestors.PushBack(new GestorSystem(gestors.Size(), "New Gestor", 0, &file, &config, &errorMessage));
}

bool EconomyScene::RetrieveDeleteBackups(const DateTime& date, Array<FileManager::FileInfo>& ret)
{
	ret = Array<FileManager::FileInfo>();
	auto files = FileManager::FindFiles(config.backupDirectory);

	if (files.IsEmpty()) return false;

	//TODO: Framework: Array must have a "Filter" fucntion that returns a different array based on a condition
	auto indexes = files.FindAll(
		[&](const FileManager::FileInfo& info)
		{
			return info.CreationDate() < date;
		}
	);

	if (indexes.IsEmpty())
		return false;

	ret = Array<FileManager::FileInfo>(indexes.Size());
	for (int i = 0; i < indexes.Size(); ++i)
		ret[i] = std::move(files[indexes[i]]);

	return true;
}

bool EconomyScene::RetrieveDeleteBackups(int amount, Array<FileManager::FileInfo>& ret)
{
	ret = Array<FileManager::FileInfo>();
	auto files = FileManager::FindFiles(config.backupDirectory);

	if (files.Size() <= amount) return false;

	files.Sort(
		[](const FileManager::FileInfo& a, const FileManager::FileInfo& b)
		{
			return a.CreationDate() > b.CreationDate();
		}
	);

	ret = Array<FileManager::FileInfo>(files.Size() - amount);

	ret.Iterate(
		[&](FileManager::FileInfo& info, int i)
		{
			info = std::move(files[i]);
		}
	);

	return true;
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
				if (config.recentFiles.IsEmpty())
				{
					ImGui::Text("There is no recent files...");
				}
				else
				{
					config.recentFiles.Iterate(
						[&](String& recent)
						{
							if (!ImGui::MenuItem(recent.Str()))
								return true;

							LoadInternal(recent);
							return false;
						},
						true
					);
				}

				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Save", "Ctrl + S"))
				Save();

			if (ImGui::MenuItem("Save As", "Ctrl + Shft + S"))
				SaveAs();

			if (ImGui::MenuItem("Backup Now"))
				Backup();


			ImGui::Separator();

			if (ImGui::BeginMenu("Export"))
			{
				ImGui::Text("Select the gestor: ");
				ImGui::Separator();
				ImGui::Spacing();

				gestors.Iterate(
					[&](const GestorSystem* g) { g->DrawExport(); }
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
				((FilterContainer*)gestors[focusedGestor]->CreateContainer(ContainerType::FILTER))
				->NewLabel();

			if (ImGui::MenuItem("New Limit"))
				((LimitContainer*)gestors[focusedGestor]->CreateContainer(ContainerType::LIMIT))
				->NewLabel();

			if (ImGui::MenuItem("New Future"))
				((FutureContainer*)gestors[focusedGestor]->CreateContainer(ContainerType::FUTURE))
				->NewLabel();

			if (ImGui::MenuItem("New Constant"))
				((FutureContainer*)gestors[focusedGestor]->CreateContainer(ContainerType::CONSTANT))
				->NewLabel();

			ImGui::RS::Separator();

			ImGui::BeginDisabled(gestors.Size() >= 4);
			if (ImGui::MenuItem("New Gestor"))
				NewGestor();
			ImGui::EndDisabled();

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("About"))
		{
			ImGui::Text("No Name Gestor %.1f", VERSION); ImGui::SameLine();

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
				ImGui::RS::Spacing(1);

				ImGui::RS::Helper("Shows, at the side of each container,\na text noting it's type.", "?"); ImGui::SameLine();
				if (ImGui::Checkbox("Show Container Typology Name", &config.showContainerType))
				{
					configFile.Write("UP_SCT", config.showContainerType);
					configFile.Save();
				}

				ImGui::RS::Helper("Shows the unsigned money in terms\nof future income.", "?"); ImGui::SameLine();
				if (ImGui::Checkbox("Show Unassigned Future Money ", &config.showFutureUnassigned));
				{
					configFile.Write("UP_SFU", config.showFutureUnassigned);
					configFile.Save();
				}

				ImGui::RS::Helper("Enlarges the size of the text\nlabels of each container.", "?"); ImGui::SameLine();
				ImGui::PushItemWidth(config.textFieldSize);
				ImGui::DragFloat("Text Fiend Size", &config.textFieldSize, 0.1f, 1.0f, 1000.0f, "%f pts", ImGuiSliderFlags_AlwaysClamp);
				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					configFile.Write("UP_TFS", config.textFieldSize);
					configFile.Save();
				}
				ImGui::PopItemWidth();

				ImGui::RS::Helper("Default directory where the File Dialog will open\nwhen no existing file location can be determined.", "?"); ImGui::SameLine();
				int r = -1;
				if (ImGui::RS::DirectoryBrowserField("Default Dialog Directory", &config.defaultDialogDirectory, r, 0, config.defaultDialogDirectory.Str()))
				{
					if (r == 1) // Browse operation returns a path
						configFile.Write("UP_DDD", config.defaultDialogDirectory);
					else if (r == 3) // Reset button clicked
					{
						config.defaultDialogDirectory = DEFAULT_FILE_DIALOG_DIRECTORY;
						configFile.Write("UP_DDD", String::Empty);
					}

					configFile.Save();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Gestors"))
			{
				ImGui::RS::Spacing(1);

				if (ImGui::BeginTabBar("##GestorsTabBar"))
				{
					gestors.Iterate(
						[&](GestorSystem* g, int i)
						{
							ImGui::PushID(g->id.Data());
							if (ImGui::BeginTabItem(g->Name().Data()))
							{
								ImGui::RS::Spacing(1);

								ImGui::Text("Currency:");
								if (ImGui::Combo("##Currency", &config.currency[i], config.comboCurrency, 5))
									g->SetFormat("%.2f", config.currency[i]);

								ImGui::EndTabItem();
							}
							ImGui::PopID();
						}
					);

					ImGui::EndTabBar();
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Backups"))
			{
				ImGui::RS::Spacing(1);

				ImGui::RS::SectionText("Backup Directory");
				{
					ImGui::RS::Helper("Determines if it is allowed to backup a newly created and unsaved file.", "?"); ImGui::SameLine();
					if (ImGui::Checkbox("Backup unsaved files?", &config.backupUnsavedFiles))
					{
						configFile.Write("UP_BUF", config.backupUnsavedFiles);
						configFile.Save();
					}

					ImGui::RS::Helper("Define where the program stores the backups.", "?"); ImGui::SameLine();
					int r = -1;
					if (ImGui::RS::DirectoryBrowserField("Backup Directory", &config.backupDirectory, r, 0, config.defaultDialogDirectory.Str()))
					{
						if (r == 1) // Browse operation returns a path
							configFile.Write("UP_BCD", config.backupDirectory);
						else if (r == 3) // Reset button clicked
						{
							config.backupDirectory = DEFAULT_BACKUP_CREATION_DIRECTORY;
							configFile.Write("UP_BCD", String::Empty);
						}

						configFile.Save();
					}
				}
				ImGui::RS::Spacing(2);

				ImGui::RS::SectionText("Automatic Backups");
				{
					ImGui::RS::Helper("When a file is opened, automatically generates a backup of it.", "?"); ImGui::SameLine();
					if (ImGui::Checkbox("Backup on file open", &config.autoOpenFileBackup))
					{
						configFile.Write("UP_AOFB", config.autoOpenFileBackup);
						configFile.Save();
					}
					ImGui::RS::Helper("When a file is closed, automatically generates a backup of it.", "?"); ImGui::SameLine();
					if (ImGui::Checkbox("Backup on file close", &config.autoCloseFileBackup))
					{
						configFile.Write("UP_ACFB", config.autoCloseFileBackup);
						configFile.Save();
					}
					ImGui::RS::Helper("When the app is closed, automatically generates a backup of the current file.", "?"); ImGui::SameLine();
					if (ImGui::Checkbox("Backup on app close", &config.autoCloseAppBackup))
					{
						configFile.Write("UP_ACAB", config.autoCloseAppBackup);
						configFile.Save();
					}
					ImGui::RS::Helper("Periodically generates backups depending on the specified timing.", "?"); ImGui::SameLine();
					if (ImGui::Checkbox("Backup periodically |", &config.autoIntervalAppBackup))
					{
						configFile.Write("UP_AIAB", config.autoIntervalAppBackup);
						configFile.Save();
						backupChrono.ChronoStop();
					}
					ImGui::SameLine(); ImGui::RS::TimeDisplay(backupChrono.ReadSec());
					ImGui::BeginDisabled(!config.autoIntervalAppBackup);
					ImGui::Dummy(ImVec2(7, 2)); ImGui::SameLine();
					if (ImGui::RS::SliderCombo("##IntervalCombo", &config.autoIntervalAppValue, config.comboIntervalText, 7, 150))
					{
						configFile.Write("UP_AIAV", config.autoIntervalAppValue);
						configFile.Save();
					}
					ImGui::EndDisabled();
				}
				ImGui::RS::Spacing(2);

				ImGui::RS::SectionText("Backup Clean Up");
				{
					static const char* comboCleanBackups[] = { "All Backups", "Older than", "Keep newest X" };
					static int a = 0;
					ImGui::RS::Helper("Defines how the search algorithm will proceed.\n - Delete All Backups: Clear all backups.\n - Delete Backups older than: Select a date and delete the older backups.\n - Keep newest X Backups: Select an amount and keep the newest amount of backups, deleting the oldest.", "?");
					ImGui::SameLine(); ImGui::Text("Delete Metrics: ");
					ImGui::PushItemWidth(126);
					ImGui::SameLine(0, 1); ImGui::Combo("##DeleteMetric", &a, comboCleanBackups, 3);
					ImGui::PopItemWidth();
					ImGui::SameLine(0, 6);

					static Array<FileManager::FileInfo> deleteBackups;
					static int index = -1;
					if (a != 0)
					{
						static DateTime date = DateTime::From::Now();
						static int amount = 1;

						if (a == 1)
						{
							if (ImGui::Button("     Scan     "))
								RetrieveDeleteBackups(date, deleteBackups);

							ImGui::RS::Helper("Choose the date to filter the backups.\nIt will filter out any newer created file since this date.", "?");
							ImGui::SameLine(); ImGui::RS::DateField("##DateBackupsSelect", &date);
						}
						else
						{
							if (ImGui::Button("     Scan     "))
								RetrieveDeleteBackups(amount, deleteBackups);

							ImGui::RS::Helper("Defines the amount of newest files to keep.", "?");
							ImGui::SameLine();
							ImGui::PushItemWidth(106);
							ImGui::DragInt("##AmountBackupsSelect", &amount, 0.15, 1, INT_MAX, "%d", ImGuiSliderFlags_ClampOnInput);
							ImGui::PopItemWidth();
						}

						if (!deleteBackups.IsEmpty())
						{
							ImGui::RS::Spacing();
							if (ImGui::Button("Delete"))
							{
								ImGui::OpenPopup("Confirm");
								index = 1;
							}
							ImGui::BeginChild("##BackupsDeleteDisplay", ImVec2(400, 160), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);

							ImGui::Spacing();
							deleteBackups.Iterate(
								[](const FileManager::FileInfo& info)
								{ ImGui::Text(info.Path().Data()); }
							);
							ImGui::Spacing();

							ImGui::EndChild();
						}
					}
					else if (ImGui::Button("Clean All"))
					{
						ImGui::OpenPopup("Confirm");
						index = 0;
					}

					ImGui::RS::CenterNextWindow();
					if (ImGui::BeginPopupModal("Confirm", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
					{
						if (index == 0)
							ImGui::TextAligned(0.45, ImGui::GetWindowWidth(), "Are you sure you want to delete all backups?");
						if (index == 1)
							ImGui::TextAligned(0.45, ImGui::GetWindowWidth(), "Are you sure you want to delete this backups?");
						ImGui::TextAligned(0.45, ImGui::GetWindowWidth(), "This process is irreversible.");

						ImGui::RS::Spacing(2);

						float w = ImGui::GetWindowWidth() / 2;
						ImGui::SetCursorPosX(w - 130);
						bool ret = false;
						if (ImGui::Button("Yes", ImVec2(120, 0)))
						{
							ret = true;
							if (index == 0) FileManager::RemoveFolder(config.backupDirectory.Str());
							else if (index == 1)
							{
								FileManager::RemoveFiles(deleteBackups);
								deleteBackups.Clear();
							}
						}
						ImGui::SameLine();

						ret |= ImGui::Button("No", ImVec2(120, 0));

						if (ret)
							ImGui::CloseCurrentPopup();

						ImGui::EndPopup();
					}
				}

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

	if (ImGui::Begin("##MainWindow", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		ImGui::RS::Spacing(0);
		ImGui::PushFont(ImGui::RS::InputTextFont);
		ImGui::Text(file.Name().Data());
		if (file.IsBackup())
		{
			ImGui::SameLine();
			ImGui::Text("- Restored");
		}
		ImGui::PopFont();

		ImGui::RS::Spacing();

		if (ImGui::BeginTabBar("##NNGFileMainTabBar"))
		{
			if (ImGui::BeginTabItem("Gestors"))
			{
				focussedTab = 0;
				enableTabs = Flag::AllFalse;
				DrawMainWindowGestors(enableTabs);
				ImGui::EndTabItem();
			}

			if (enableTabs[0] && ImGui::BeginTabItem("Cash Flow"))
			{
				focussedTab = 1;
				DrawMainWindowCashFlow();
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();

		if (!errorMessage.IsNull())
			ImGui::TextColored(ImVec4(1, 0, 0, 1), errorMessage.Str());

		if (!warningMessage.IsNull())
			ImGui::TextColored(ImVec4(1, 1, 0, 1), warningMessage.Str());
	}
	ImGui::End();
}

void EconomyScene::DrawMainWindowGestors(Flag& enable)
{
	//TODO: Build a way to eliminate a gestor
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 0.0f));
	if (ImGui::BeginTable("##systemtable", gestors.Size(), ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable))
	{
		// Captura la posició Y de l'inici de la taula
		float tableHeight = (ImGui::GetWindowPos().y + ImGui::GetWindowHeight()) - ImGui::GetCursorScreenPos().y;
		ImVec2 mousePos = ImGui::GetMousePos();

		gestors.Iterate(
			[&](GestorSystem* g, int i)
			{
				ImGui::TableNextColumn();

				float columnWidth = ImGui::GetColumnWidth();
				ImVec2 cellMin = ImGui::GetCursorScreenPos();
				ImVec2 cellMax = ImVec2(cellMin.x + columnWidth, cellMin.y + tableHeight);

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

				g->Draw(columnWidth, enable);
			}
		);

		ImGui::EndTable();
	}
	ImGui::PopStyleVar();
}

void EconomyScene::DrawMainWindowCashFlow()
{
	ImGui::Spacing();
	ImGui::Text("Start Date:"); ImGui::SameLine();
	ImGui::RS::MonthSelector("##MonthSelectorStart", &startMonthSelector, App::OpenAppTime, &startDate, 0, endMonthSelector - 3);
	ImGui::SameLine(270);
	ImGui::Text("End Date:"); ImGui::SameLine();
	ImGui::RS::MonthSelector("##MonthSelectorEnd", &endMonthSelector, App::OpenAppTime, &endDate, startMonthSelector + 3, 18);

	static float sds = 0;
	
	int total = startDate.MonthsUntil(endDate);
	float w = (ImGui::GetWindowWidth() - 500) * Maths::Lerp(sds, 1, 0.9);

	float x = 140 + (136 * (1 - sds));
	ImVec2 pos = ImGui::GetCursorPos() + ImVec2(x, 52);
	float wR = w / (float)total;

	ImGui::RS::Spacing(2);
	float sliderWidth = 138 * (1 - sds);
	ImGui::PushItemWidth(60 + sliderWidth);
	ImGui::SliderFloat("##AAA", &sds, 1, 0, "");
	ImGui::PopItemWidth();

	ImGui::GetWindowDrawList()->AddLine(ImVec2(pos.x - 0.5f, pos.y + 22), ImVec2(pos.x + w + 0.5f, pos.y + 22), IM_COL32(255, 255, 255, 255), 2.0f);
	for (int i = 0; i < total + 1; ++i)
	{
		DateTime monthDate = startDate + TimeSpan<>::From::Months(i);
		float xPos = pos.x + (i * wR);
		bool plotYear = i == 0 || i == total || monthDate.Month() == 1;
		ImGui::GetWindowDrawList()->AddLine(ImVec2(xPos, pos.y + (plotYear ? -16 : 6)), ImVec2(xPos, pos.y + 22), IM_COL32(255, 255, 255, 255), 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(xPos, pos.y + 22), ImVec2(xPos, pos.y + 300), IM_COL32(255, 255, 255, 100), 2.0f);
		if (i == total)
			continue;

		ImVec2 retPos = ImVec2(xPos - 62, pos.y - 42);
		if (plotYear)
		{
			ImGui::SetCursorPos(retPos);
			ImGui::Text("\n%d\n%s", monthDate.Year(), monthDate.MonthName());
		}
		else
		{
			ImGui::SetCursorPos(retPos);
			ImGui::Text("\n\n%s", monthDate.MonthName());
		}
	}

	ImGui::SetCursorPos(ImVec2(8, pos.y - 16));
	
	static int comboi = 0;
	auto lambda = [](void* gestors, int index) -> const char* { return index == 0 ? "All" : (*(Vector<GestorSystem*, true>*)gestors)[index - 1]->Name().Data(); };
	ImGui::PushItemWidth(60 + sliderWidth);
	ImGui::Combo("##CashFlowTabBar", &comboi, lambda, &gestors, gestors.Size() + 1);
	ImGui::PopItemWidth();

	ImGui::RS::Spacing(1);

	if (comboi == 0)
		gestors.Iterate(
			[&](GestorSystem* g) {
				g->DrawCashFlow(wR, pos.x);
			}
		);
	else
		gestors[comboi - 1]->DrawCashFlow(wR, pos.x);
}

void EconomyScene::DrawToolbarWindow(bool& ret)
{
	if (!ret) return;
	ret = true;

	if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		bool action = false;

		ImGui::BeginDisabled(focussedTab != 0);

		if (ImGui::Button("FILTER"))
		{
			((FilterContainer*)gestors[focusedGestor]->CreateContainer(ContainerType::FILTER))
				->NewLabel();
			action = true;
		}

		if (ImGui::Button("LIMIT "))
		{
			((LimitContainer*)gestors[focusedGestor]->CreateContainer(ContainerType::LIMIT))
				->NewLabel();
			action = true;
		}

		if (ImGui::Button("FUTURE"))
		{
			((LimitContainer*)gestors[focusedGestor]->CreateContainer(ContainerType::FUTURE))
				->NewLabel();
			action = true;
		}

		if (ImGui::Button("CONST "))
		{
			((ConstContainer*)gestors[focusedGestor]->CreateContainer(ContainerType::CONSTANT))
				->NewLabel();
			action = true;
		}

		ImGui::EndDisabled();

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
