#ifndef __RENDER_H__
#define __RENDER_H__

#include "Module.h"

#include "Point.h"
#include "SDL/include/SDL.h"
#include "Window.h"

class Render : public Module
{
public:

	Render(Window* win);

	virtual ~Render();

	bool Start();

	bool PreUpdate(float dt);

	bool Update(float dt);

	bool CleanUp();

public:

	// Utils
	void SetViewPort(const SDL_Rect& rect);
	void ResetViewPort();

	// Set background color
	void SetBackgroundColor(SDL_Color color);

	// VSYNC Functions
	bool GetVSync() const { return vSync; }
	void SetVSync(bool vSync);

	// Size Functions
	suint GetScale() const { return scale; };
	void SetScale(suint scale) { this->scale = scale; }

public:

	SDL_Renderer* renderer = nullptr;
	SDL_Rect camera = {};
	SDL_Rect viewport = {};
	SDL_Color background = {};
	SDL_GLContext context = {};

private:

	Window* win = nullptr;

	bool vSync = true;
	suint scale = 1;

};

#endif // __RENDER_H__