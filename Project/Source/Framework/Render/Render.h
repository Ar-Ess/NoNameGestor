#pragma once

#include "Framework/Engine/Module.h"
#include "Framework/Shapes/Circle.h"
#include "Framework/Shapes/Line.h"
#include "Framework/Render/Camera.h"
#include "Framework/Render/Color.h"
#include "Framework/Render/BlendMode.h"
#include "Framework/Render/Flip.h"
#include "Framework/Data/String.h"

struct SDL_Surface;
struct SDL_Renderer;
struct SDL_Texture;
typedef struct SDL_FRect;
typedef struct SDL_FPoint;

class Render : public Module<Render>
{
	friend class App;
	friend class AssetsManager;

public:

	// Drawing
	static bool DrawRectangle(const Rect& rect, bool anchoredToCam = false, const Color& color = Color::White, bool fill = true, const Point& scale = Point::One);
	static bool DrawLine(const Line& line, bool anchoredToCam = false, const Color& color = Color::White);
	static bool DrawCircle(const Circle& circle, bool anchoredToCam = false, const Color& color = Color::White, bool filled = true, float scale = 1);
	static bool DrawGrid(const Rect& rect, const Point& divisions, bool anchoredToCam = false, const Color& color = Color::White, bool startLine = true, bool endLine = true);
	static bool DrawTexture(SDL_Texture* texture, const Rect& texRect, const Point& scale, const Color& color, BlendMode mode, bool anchoredToCam, Rect mask, float angle, const Point& rotationAnchor, Flip flip);

	static SDL_Texture* CreateTextureFromSurface(SDL_Surface* surface, Point& size);

	// Adapts from bottom-left (engine) to top-left (SDL) regarding the world screen
	static const SDL_FRect& AdaptToSDLAxisScreen(const Rect& rectangle);
	// Adapts from bottom-left (engine) to top-left (SDL) regarding the screen inputted
	static const SDL_FRect& AdaptToSDLAxisCustomScreen(const Rect& rectangle, const Point& newScreenSize);
	// Adapts from bottom-left (engine) to top-left (SDL) regarding the world screen
	static const SDL_FPoint& AdaptToSDLAxisScreen(const Point& point);

private:

	Render();

	virtual ~Render();

	bool Awake() override;

	bool Start() override;

	bool PreUpdate(float dt) override;

	bool Update(float dt) override;
	
	bool FixedUpdate(float dt) override;

	bool CleanUp() override;

	void UpdateViewport();


public:

	static Camera camera;
	static Rect viewport;
	static Color background;

private:

	static SDL_Renderer* renderer;
	bool vSync = true;
};