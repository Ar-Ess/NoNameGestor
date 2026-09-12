#include "Framework/Engine/App.h"

static void EngineConfig(App::EngineConfig& config)
{
	config.DisableModule(App::ModuleType::RENDER);
	config.DisableModule(App::ModuleType::GUI);
	config.DisableModule(App::ModuleType::ASSETS);
	config.DisableConfig(App::ConfigType::RELEASE_DEBUGGER_MODE);
	config.gameName = "NoNameGestor v1.4";
}

int main(int argc, char* args[])
{
	return App::Run(argc, args, EngineConfig);
}