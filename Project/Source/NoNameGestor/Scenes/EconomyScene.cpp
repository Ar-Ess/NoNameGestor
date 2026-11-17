#include "EconomyScene.h"

#include "Framework/Scenes/Startup.h"
#include "Framework/Utils/Maths.h"
#include "Framework/External/SDL/include/SDL_events.h"
#include "Framework/External/SDL/include/SDL_render.h"

#include "NoNameGestor/External/imgui/imgui_impl_sdl2.h"
#include "NoNameGestor/External/imgui/imgui_impl_sdlrenderer2.h"

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

	gestor.Awake();

	return true;
}

bool EconomyScene::Start()
{
	bool ret = gestor.Start();
	return ret;
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
			gestor.QuitBackup();
		}
	}

	// Generate ImGui Renderer New Frame 
	ImGui_ImplSDLRenderer2_NewFrame();
	// Generate SDL2 New Frame
	ImGui_ImplSDL2_NewFrame();
	// Generate ImGui New Frame
	ImGui::NewFrame();

	ret &= gestor.Update();

	return ret;
}

bool EconomyScene::Draw(float dt)
{	
	// Clear Screen
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	gestor.Draw();

	// ImGui Render
	ImGui::Render();
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
	SDL_RenderPresent(renderer);

	return true;
}

bool EconomyScene::CleanUp()
{
	gestor.CleanUp();

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