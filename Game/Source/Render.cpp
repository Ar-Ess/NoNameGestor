#include "App.h"
#include "Window.h"
#include "Render.h"
#include "Input.h"
#include "Defs.h"
#include "LogFunction.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define VSYNC true

#include "External/imgui/imgui_impl_sdl.h"
#include "External/imgui/imgui_impl_sdlrenderer.h"


Render::Render(Window* win) : Module()
{
	this->win = win;
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
	camera = {};
	renderer = nullptr;
}

Render::~Render()
{
}

bool Render::Start()
{
	LOG("Render start");

	renderer = SDL_CreateRenderer(win->window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	if (renderer == nullptr)
	{
		SDL_Log("Error creating SDL_Renderer!");
		return false;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(win->window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);

	SetVSync(true);

	return true;
}

bool Render::PreUpdate(float dt)
{
	bool ret = true;
	// Generate new frame
	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame(win->window);
	ImGui::NewFrame();

	SDL_RenderClear(renderer);

	return ret;
}

bool Render::Update(float dt)
{
	ImGui::Render();
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	SDL_RenderClear(renderer);
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderPresent(renderer);

	return true;
}

bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void Render::SetVSync(bool vSync)
{
	if (vSync)
	{
		SDL_GL_SetSwapInterval(-1);
	}
	else if (!vSync)
	{
		SDL_GL_SetSwapInterval(0);
	}
}

void Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_RenderSetViewport(renderer, &rect);
}