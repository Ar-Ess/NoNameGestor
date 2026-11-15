#pragma once

#include "Framework/Engine/Module.h"
#include "Framework/Time/DateTime.h"
#include "Framework/Time/PerfTimer.h"
#include "Framework/Data/Array.h"
#include "Framework/Data/String.h"
#include "Framework/Data/Flag.h"

class App
{
public:

	enum class ModuleType
	{
		WINDOW,
		INPUT,
		ASSETS,
		AUDIO,
		COROUTINE,
		GUI,
		SCENES,
		RENDER,
		PROGRESS
	};

	enum class ConfigType
	{
		// It will execute the release mode based on the solution directory rather than the executable directory.
		// Release mode, by default, executes based on where the .exe is located. If you want to debug it from the
		// solution project, you should enable this configuration.
		// Disabled by default
		RELEASE_LOCAL_MODE = 9
	};

	struct EngineConfig
	{
		void EnableModule(ModuleType module) const;

		void DisableModule(ModuleType module) const;

		bool IsModuleEnabled(ModuleType module) const;

		void EnableConfig(ConfigType conf) const;

		void DisableConfig(ConfigType conf) const;

		bool IsConfigEnabled(ConfigType conf) const;

		int CountEnabledModules() const;

	public:
		
		String gameName = nullptr;

	private:

		// Module Configuration
		// Bit 0 - true  | WINDOW
		// Bit 1 - true  | INPUT
		// Bit 2 - true  | ASSETS
		// Bit 3 - true  | AUDIO
		// Bit 4 - true  | COROUTINE
		// Bit 5 - true  | GUI
		// Bit 6 - true  | SCENES
		// Bit 7 - true  | RENDER
		// Bit 8 - true  | PROGRESS
		// Engine Configuration
		// Bit 9 - false | RELEASE_LOCAL_MODE
		mutable Flag config = 0b0111111111;
	};

	using EngineConfigurator = void(*)(EngineConfig&);

public:

	// Main loop function of the app. Call this on your code main function.
	// configurator is a ptr to a function that configures the modules.
	// void ConfigModules(App::ModuleConfig& config) { config.Disable(App::ModuleType::AUDIO); }
	// Leave nullptr to initialize all modules.
	static int Run(int argc, char* args[], EngineConfigurator configurator = nullptr);
	
	static void TargetFPS(unsigned int fps);
	static unsigned int TargetFPS();

	static uint64_t FrameCount();

	static const float DeltaTime();

	static const float FixedDeltaTime();

	static void Quit();

	static StringView GameName();
	static StringView ExecutableName();
	static StringView ExecutablePath();
	static StringView ExecutableDirectory();
	static StringView ProjectDirectory();
	static StringView DataDirectory();
	static StringView AssetsDirectory();
	static StringView ConfigurationPath();
	static StringView ArchivesDirectory();
	static StringView UserDirectory();
	static StringView LogsDirectory();
	static StringView OpenedFilePath();
	static bool IsAppLaunchedWithFile();
	// Returns if the app should internally behave as debug, even though it may be in Release mode.
	// You may vary this behaviour on the initial EngineConfig "RELEASE_DEBUGGER_MODE" set to true.
	static bool IsInternallyDebug();

private:

	App(int argc, char* args[], int moduleCount);

	bool Awake(const EngineConfig& config);

	bool Start();

	bool Update();

	bool CleanUp();

	void ConfigureEngine(const EngineConfig& config);

	void GenerateAppPaths();
	
	void ConfigureModules(const EngineConfig& config);

public:

	static const bool DebugMode;

	static const DateTime OpenAppTime;

private:

	// Arguments
	const Array<char*> arguments;

	// Dataç
	String gameName = nullptr;
	String executableName = nullptr;
	String executablePath = nullptr;
	String executableDirectory = nullptr;

	String projectDirectory = nullptr;
	String dataDirectory = nullptr;

	// Assets folder in debug, Assets.pak in release
	String assetsDirectory = nullptr;

	String configurationPath = nullptr;
	String archivesDirectory = nullptr;
	String userDirectory = nullptr;
	String logsDirectory = nullptr;

	bool launchedWithFile = false;
	String openedFilePath = nullptr;

	// Modules
	Array<AppModule*, true> modules;

	// Timing
	uint64_t frameCount = 0;
	float dt = 0.0f;
	float accumulator = 0.0f;
	unsigned int targetFPS = 60;

	PerfTimer startupTime;
	PerfTimer frameTime;

	// Quit
	bool quit = false;
	bool releaseDebuggerMode = false;

	static App* instance;

};