#pragma once

#include "Module.h"
#include "SDL/include/SDL.h"
#include "Point.h"

enum class WinMode
{
	WINDOW_MODE = 0,
	FULLSCREEN_MODE = 1,
	DESKTOP_FULLSCREEN_MODE = 4097
};

class Window : public Module
{

	friend class App;

public:

	void SetTitle(const char* title);

	unsigned int Scale() const { return scale; }

	void WindowMode(WinMode mode);

	WinMode WindowMode();

	Point Size() const { return size; }

private:

	Window();

	virtual ~Window();

	bool Awake() override;

	bool Start() override;

	bool CleanUp() override;

private:

	friend class Render;

	SDL_Window* window = nullptr;
	SDL_Surface* screenSurface = nullptr;
	WinMode mode = WinMode::WINDOW_MODE;
	Point size;
	unsigned int scale = 1;

};