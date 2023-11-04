#include "Render.h"

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

	if (!renderer)
		return false;

	return true;
}

bool Render::Start()
{
	SDL_RenderGetViewport(renderer, &viewport);
	VSync(true);

	return true;
}

bool Render::PreUpdate(float dt)
{
	SDL_RenderClear(renderer);

	return true;
}

bool Render::Update(float dt)
{
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	SDL_RenderPresent(renderer);

	return true;
}

bool Render::CleanUp()
{
	LOG("Destroying SDL render");
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
