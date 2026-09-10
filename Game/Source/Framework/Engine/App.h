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

	struct ModuleConfig
	{
		void Enable(ModuleType module) const;

		void Disable(ModuleType module) const;

		bool IsEnabled(ModuleType module) const;

		int Count() const;

	private:

		mutable Flag moduleConfig = Flag::AllTrue;

	};

	using ModuleConfigurator = void(*)(ModuleConfig&);

public:

	// Main loop function of the app. Call this on your code main function.
	// configurator is a ptr to a function that configures the modules.
	// void ConfigModules(App::ModuleConfig& config) { config.Disable(App::ModuleType::AUDIO); }
	// Leave nullptr to initialize all modules.
	static int Run(int argc, char* args[], ModuleConfigurator configurator = nullptr);
	
	static void TargetFPS(unsigned int fps);
	static unsigned int TargetFPS();

	static uint64_t FrameCount();

	static const float DeltaTime();

	static const float FixedDeltaTime();

	static void Quit();

	static const String& AppName();
	static const String& ExecutablePath();
	static const String& ExecutableDirectory();
	static const String& ProjectDirectory();
	static const String& DataDirectory();
	static const String& AssetsPath();
	static const String& ConfigurationPath();
	static const String& ArchivesDirectory();
	static const String& UserDirectory();
	static const String& LogsDirectory();
	static const String& WorkingDirectory();
	static const String& OpenedFilePath();
	static const bool IsAppLaunchedWithFile();

private:

	App(int argc, char* args[], const ModuleConfig& config);

	bool Start();

	bool Update();

	bool CleanUp();

	void GenerateAppPaths(int argc, char* args[]);

	void ConfigureModules(const ModuleConfig& config);

public:

	static const bool DebugMode;

	static const DateTime OpenAppTime;

private:

	// Arguments
	const Array<char*> arguments;

	// Data
	String appName = nullptr;

	String executablePath = nullptr;
	String executableDirectory = nullptr;

	String projectDirectory = nullptr;
	String dataDirectory = nullptr;

	// Assets folder in debug, Assets.pak in release
	String assetsPath = nullptr;

	String configurationPath = nullptr;
	String archivesDirectory = nullptr;
	String userDirectory = nullptr;
	String logsDirectory = nullptr;

	String workingDirectory = nullptr;

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

	static App* instance;

};