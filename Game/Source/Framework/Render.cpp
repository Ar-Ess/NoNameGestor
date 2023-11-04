#include "Render.h"
#include "../External/imgui/imgui_impl_sdl.h"
#include "../External/imgui/imgui_impl_sdlrenderer.h"

Render::Render(Window* win) : Module()
{
	this->win = win;
	SetBackgroundColor({0, 0, 0, 0});
	renderer = nullptr;
}

Render::~Render()
{
}

bool Render::Awake()
{
	uint32 flags = SDL_RENDERER_ACCELERATED;
	flags |= SDL_RENDERER_PRESENTVSYNC;

	renderer = SDL_CreateRenderer(win->window, -1, flags);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	if (!renderer)
		return false;

	return true;
}

bool Render::Start()
{
	ImGui_ImplSDL2_InitForSDLRenderer(win->window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);

	SDL_RenderGetViewport(renderer, &viewport);
	VSync(true);

	return true;
}

bool Render::PreUpdate(float dt)
{
	// Generate new frame
	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame(win->window);
	ImGui::NewFrame();

	SDL_RenderClear(renderer);

	return true;
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

void Render::VSync(bool vSync)
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

void Render::ResetViewPort()
{
	SDL_RenderSetViewport(renderer, &viewport);
}

SDL_Renderer* Render::Renderer()
{
	return renderer;
}
