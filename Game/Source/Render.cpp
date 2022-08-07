#include "App.h"
#include "Window.h"
#include "Render.h"
#include "Input.h"
#include "Defs.h"
#include "Log.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define VSYNC true

#pragma comment (lib, "Source/External/glew/glew32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */

#include "../Source/External/ImGui/imgui_impl_sdl.h"
#include "../Source/External/ImGui/imgui_impl_opengl3.h"


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
	LOG("render start");
	// back background
	//uint32 flags = SDL_RENDERER_ACCELERATED;
	//flags |= SDL_RENDERER_PRESENTVSYNC;

	//renderer = SDL_CreateRenderer(win->window, -1, flags);
	//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	LOG("Creating 3D Renderer context");
	bool ret = true;

	//Create context
	context = SDL_GL_CreateContext(win->window);
	if (context == NULL)
	{
		LOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	if (ret == true)
	{
		//Use Vsync
		if (VSYNC && SDL_GL_SetSwapInterval(1) < 0)
			//LOG("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		GLenum error = glewInit();
		if (error != GLEW_OK)
		{
			ret = false;
		}

		//Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			//LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			//LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);

		//Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.f);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			//LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		GLfloat LightModelAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);

		GLfloat MaterialAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);
	}

	// Projection matrix for
	OnResize(SCREEN_WIDTH, SCREEN_HEIGHT);

	//SetVSync(true);

	return true;
}

bool Render::PreUpdate(float dt)
{
	bool ret = true;
	// Generate new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(win->window);
	ImGui::NewFrame();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	SDL_RenderClear(renderer);

	return ret;
}

bool Render::Update(float dt)
{
	//SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	//SDL_RenderPresent(renderer);

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::Render();

	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	// Update and Render additional Platform Windows
	// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
	//  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
		SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
	}

	SDL_GL_SwapWindow(win->window);

	return true;
}

bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	SDL_GL_DeleteContext(context);
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

void Render::ResetViewPort()
{
	SDL_RenderSetViewport(renderer, &viewport);
}

void Render::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//ProjectionMatrix = perspective(60.0f, (float)width / (float)height, 0.125f, 512.0f);
	//glLoadMatrixf(&ProjectionMatrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

bool Render::DrawTexture(SDL_Texture* texture, Point position, Point size, bool anchored, Rect* section, double angle, SDL_RendererFlip flip) const
{
	if (!texture) return false;

	bool ret = true;
	SDL_Rect rect = { int(position.x), int(position.y), 0, 0 };
	SDL_Rect* sectPtr = nullptr;
	SDL_Rect sect = {};

	if (anchored)
	{
		rect.x += (int)camera.x;
		rect.y += (int)camera.y;
	}

	if (section != nullptr)
	{
		rect.w = int(section->w);
		rect.h = int(section->h);

		sect = { (int)section->x, (int)section->y, (int)section->w, (int)section->h };
		sectPtr = &sect;
	}
	else SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);

	rect.w *= int(size.x);
	rect.h *= int(size.y);

	if (SDL_RenderCopyEx(renderer, texture, sectPtr, &rect, angle, NULL, flip) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawRectangle(Rect rect, SDL_Color color, Point size, bool anchored, bool fill) const
{
	bool ret = true;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

	Rect rec(rect);

	if (anchored)
	{
		rect.x += (int)camera.x;
		rect.y += (int)camera.y;
	}

	rec.w *= size.x;
	rec.h *= size.y;

	SDL_Rect rectangle = {(int)rec.x, (int)rec.y, (int)rec.w, (int)rec.h};

	int result = (fill) ? SDL_RenderFillRect(renderer, &rectangle) : SDL_RenderDrawRect(renderer, &rectangle);

	if (result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawLine(Point start, Point end, SDL_Color color, bool anchored) const
{
	bool ret = true;
	suint scale = win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

	int result = -1;

	if (anchored)
		result = SDL_RenderDrawLine(renderer, camera.x + (int)start.x * (int)scale, camera.y + (int)start.y * (int)scale, camera.x + (int)end.x * (int)scale, camera.y + (int)end.y * (int)scale);
	else
		result = SDL_RenderDrawLine(renderer, (int)start.x * (int)scale, (int)start.y * (int)scale, (int)end.x * (int)scale, (int)end.y * (int)scale);

	if (result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawCircle(CircleCollider circle, SDL_Color color, bool fill, bool anchored) const
{
	bool ret = true;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

	int result = -1;
	SDL_Point points[360];

	float factor = (float)M_PI / 180.0f;

	for (uint i = 0; i < 360; ++i)
	{
		points[i].x = (int)(circle.x + circle.radius * cos(double(i) * double(factor)));
		points[i].y = (int)(circle.y + circle.radius * sin(double(i) * double(factor)));

		if (anchored)
		{
			points[i].x += camera.x;
			points[i].y += camera.y;
		}
	}

	result = SDL_RenderDrawPoints(renderer, points, 360);

	if (fill)
	{
		for (double dy = 1; dy <= circle.radius; dy += 1)
		{
			double dx = floor(sqrt((2.0 * circle.radius * dy) - (dy * dy)));
			if (anchored)
			{
				SDL_RenderDrawLine(renderer, camera.x + (int)circle.x - (int)dx, camera.y + (int)circle.y + (int)dy - (int)circle.radius, camera.x + (int)circle.x + (int)dx, camera.y + (int)circle.y + (int)dy - (int)circle.radius);
				SDL_RenderDrawLine(renderer, camera.x + (int)circle.x - (int)dx, camera.y + (int)circle.y - (int)dy + (int)circle.radius, camera.x + (int)circle.x + (int)dx, camera.y + (int)circle.y - (int)dy + (int)circle.radius);
			}
			else
			{
				SDL_RenderDrawLine(renderer, (int)circle.x - (int)dx, (int)circle.y + (int)dy - (int)circle.radius, (int)circle.x + (int)dx, (int)circle.y + (int)dy - (int)circle.radius);
				SDL_RenderDrawLine(renderer, (int)circle.x - (int)dx, (int)circle.y - (int)dy + (int)circle.radius, (int)circle.x + (int)dx, (int)circle.y - (int)dy + (int)circle.radius);
			}
		}
	}

	if (result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawParticle(SDL_Texture* texture, Point position, Rect* section, Rect* rectSize, SDL_Color color, SDL_BlendMode blendMode, double angle) const
{
	bool ret = true;
	uint scale = win->GetScale();

	SDL_Rect rect;
	SDL_Rect* sectPtr = nullptr;
	SDL_Rect sect = {};

	rect.x = camera.x + (int)position.x * (int)scale;
	rect.y = camera.y + (int)position.y * (int)scale;

	if (rectSize != NULL)
	{
		rect.w = (int)rectSize->w;
		rect.h = (int)rectSize->h;
	}
	else if (section != NULL)
	{
		rect.w = (int)section->w;
		rect.h = (int)section->h;

		sect = { (int)section->x, (int)section->y, (int)section->w, (int)section->h };
		sectPtr = &sect;
	}
	else
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);

	int px = rect.w / 2;
	int py = rect.h / 2;

	rect.w *= scale;
	rect.h *= scale;

	SDL_Point* p = NULL;
	SDL_Point pivot;
	pivot.x = px;
	pivot.y = py;
	p = &pivot;

	if (SDL_SetTextureColorMod(texture, color.r, color.g, color.b) != 0)
		LOG("Cannot set texture color mode. SDL_SetTextureColorMod error: %s", SDL_GetError());

	if (SDL_SetTextureAlphaMod(texture, color.a) != 0)
		LOG("Cannot set texture alpha mode. SDL_SetTextureAlphaMod error: %s", SDL_GetError());

	if (SDL_SetTextureBlendMode(texture, blendMode) != 0)
		LOG("Cannot set texture blend mode. SDL_SetTextureBlendMode error: %s", SDL_GetError());


	if (SDL_RenderCopyEx(renderer, texture, sectPtr, &rect, angle, NULL, SDL_FLIP_NONE) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}