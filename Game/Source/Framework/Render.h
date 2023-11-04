#pragma once

#include "Module.h"

#include "SDL/include/SDL.h"
#include "Point.h"
#include "Window.h"

class Render : public Module
{

	friend class App;

public:

	SDL_Renderer* Renderer();

	// Set background color
	void SetBackgroundColor(SDL_Color color);

	// VSYNC Functions
	bool VSync() const { return vSync; }
	void VSync(bool vSync);

	// Size Functions
	unsigned int Scale() const { return scale; };
	void Scale(unsigned int scale) { this->scale = scale; }

private:

	Render(Window* win);

	virtual ~Render();

	bool Awake();

	bool Start();

	bool PreUpdate(float dt);

	bool Update(float dt);

	bool CleanUp();

	void SetViewPort(const SDL_Rect& rect);

	void ResetViewPort();

private:

	SDL_Renderer* renderer = nullptr;
	SDL_Rect viewport = {};
	SDL_Color background = {};
	Window* win = nullptr;
	bool vSync = true;
	unsigned int scale = 1;
};