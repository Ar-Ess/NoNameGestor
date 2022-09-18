#include "Input.h"
#include "Window.h"
#include "App.h"

#include "Defs.h"
#include "LogFunction.h"

#include "SDL/include/SDL.h"
#include "External/imgui/imgui_impl_sdl.h"
#include "External/imgui/imgui_impl_sdlrenderer.h"


Window::Window() : Module()
{
	window = NULL;
	screenSurface = NULL;
}

Window::~Window()
{
}

bool Window::Start()
{
	LOG("Init SDL window & surface");
	bool ret = true;

	size = { 1280, 720 };

	if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;

	SDL_WindowFlags wFlags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN);
	window = SDL_CreateWindow("No Name Gestor - NoNameSL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.x, size.y, wFlags);


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	io.WantCaptureMouse = true;
	io.WantCaptureKeyboard = true;

	if (!window)
	{
		LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
		screenSurface = SDL_GetWindowSurface(window);

	return ret;
}

bool Window::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	// Destroy window
	if (window != NULL)
	{
		SDL_DestroyWindow(window);
	}

	// Quit SDL subsystems
	SDL_Quit();
	return true;
}

void Window::SetTitle(const char* newTitle)
{
	SDL_SetWindowTitle(window, newTitle);
}

void Window::SetWinFullScreen(bool full)
{
	if (full) SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
	else if (!full) SDL_SetWindowFullscreen(window, 0);

	fullScreen = full;
}

void Window::SetWinDFullScreen(bool full)
{
	if (full) SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	else if (!full) SDL_SetWindowFullscreen(window, 0);

	dFullScreen = full;
}
