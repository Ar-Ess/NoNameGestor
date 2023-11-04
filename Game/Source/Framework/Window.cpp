#include "Window.h"

Window::Window() : Module()
{
}

Window::~Window()
{
}

bool Window::Awake()
{
	LOG("Init SDL window & surface");
	bool ret = true;

	size = { 1280.0f, 720.0f };

	if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;

	uint32 flags = SDL_WINDOW_SHOWN;

	window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, size.x, size.y, flags);

	if (!window)
	{
		LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
		screenSurface = SDL_GetWindowSurface(window);

	return ret;
}

bool Window::Start()
{
	WindowMode(mode);
	return true;
}

bool Window::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	// Destroy window
	if (window != nullptr) SDL_DestroyWindow(window);

	// Quit SDL subsystems
	SDL_Quit();

	return true;
}

void Window::SetTitle(const char* newTitle)
{
	SDL_SetWindowTitle(window, newTitle);
}

void Window::WindowMode(WinMode mode)
{
	if (this->mode == mode) return;

	SDL_SetWindowFullscreen(window, (SDL_WindowFlags)mode);
	this->mode = mode;
}

WinMode Window::WindowMode()
{
	return mode;
}
