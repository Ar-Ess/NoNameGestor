#include "Framework/Engine/App.h"

static void ModuleConfig(App::ModuleConfig& config)
{
	config.Disable(App::ModuleType::RENDER);
	config.Disable(App::ModuleType::GUI);
}

int main(int argc, char* args[])
{
	return App::Run(argc, args, ModuleConfig);
}