#pragma once

#include "Framework/Engine/Module.h"
#include "Framework/Window/WindowEvent.h"
#include "Framework/Window/WindowConfig.h"
#include "Framework/Shapes/Point.h"
#include "Framework/Data/Flag.h"

typedef struct SDL_Window SDL_Window;

class Window : public Module<Window>
{

	friend class App;
	friend class EconomyScene;

public:

	static void SetTitle(const char* title, ...);

	static void Fullscreen(bool fullscreen);
	static bool Fullscreen();
	static void VSync(bool vsync);
	static bool VSync();

	static Point WindowSize();
	static Point RenderSize();
	static float Scale();

	static bool IsWindow(WinEvent event);

private:

	Window();

	virtual ~Window();

	bool Awake() override;

	bool Start() override;

	bool CleanUp() override;

public:

	static Point WorldSize;

private:

	friend class Render;
	friend class Input;

	static SDL_Window* window;
	Flag events;
	Flag config;

	// Real window size
	Point windowSize = Point::Zero;
	float scale = -1;
	Flag updateWindowInformation = Flag::AllTrue;
};