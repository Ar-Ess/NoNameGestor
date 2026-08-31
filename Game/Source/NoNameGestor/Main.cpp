#include "Framework/Engine/App.h"

static void ModuleConfig(App::ModuleConfig& config)
{
	config.Disable(App::ModuleType::RENDER);
}

int main(int argc, char* args[])
{
	return App::Run(argc, args, ModuleConfig);
}