#include "App.h"

#include "Window.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"

#include <time.h>

#include "Defs.h"
#include "LogFunction.h"

App::App(int argc, char* args[]) : argc(argc), args(args)
{
	PERF_START(ptimer);

	win = new Window();
	input = new Input(win);
	render = new Render(win);
	scene = new Scene(render, input, win);

	AddModule(win);
	AddModule(input);
	AddModule(scene);
	AddModule(render);
	
	PERF_PEEK(ptimer);
}

App::~App()
{
	size_t size = modules.size();
	for (size_t i = 0; i < size; ++i)
	{
		Module* m = modules[i];
		RELEASE(m);
	}

	modules.shrink_to_fit();
	modules.clear();
}

void App::AddModule(Module* module)
{
	modules.push_back(module);
}

bool App::Start()
{
	bool ret = true;
	PERF_START(ptimer);

	InitImGui();

	size_t size = modules.size();
	for (size_t i = 0; i < size; ++i) ret = modules[i]->Start();
	
	PERF_PEEK(ptimer);

	srand((uint)time(NULL));

	return ret;
}

bool App::InitImGui()
{
	bool ret = true;

	PERF_START(ptimer);

	// Setup SDL
	// (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
	// depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	PERF_PEEK(ptimer);

	return ret;
}

void App::PrepareUpdate()
{
	frameCount++;
	lastSecFrameCount++;

	dt = frameTime.ReadSec();
	frameTime.Start();
}

bool App::Update()
{
	bool ret = true;

	PrepareUpdate();

	if (input->GetWindowEvent(EventWindow::WE_QUIT)) ret = false;

	
	size_t size = modules.size();
	for (size_t i = 0; i < size; ++i)
	{
		Module* m = modules[i];
		if (!m->active) continue;

		if (!m->PreUpdate(dt)) ret = false;
	}

	for (size_t i = 0; i < size; ++i)
	{
		Module* m = modules[i];
		if (!m->active) continue;

		if (!m->Update(dt)) ret = false;
	}

	FinishUpdate();
	

	return ret;
}

void App::FinishUpdate()
{

	if (lastSecFrameTime.Read() > 1000)
	{
		lastSecFrameTime.Start();
		prevLastSecFrameCount = lastSecFrameCount;
		lastSecFrameCount = 0;
	}

	float averageFps = float(frameCount) / startupTime.ReadSec();
	float secondsSinceStartup = startupTime.ReadSec();
	uint32 lastFrameMs = frameTime.Read();
	uint32 framesOnLastUpdate = prevLastSecFrameCount;

	/*static char title[256];
	sprintf_s(title, 256, "Av.FPS: %.2f Last Frame Ms: %02u Last sec frames: %i Last dt: %.3f Time since startup: %.3f Frame Count: %I64u ",
			  averageFps, lastFrameMs, framesOnLastUpdate, dt, secondsSinceStartup, frameCount);
	*/
	win->SetTitle("No Name Gestor");

	if (frameDelay > lastFrameMs)
	{
		SDL_Delay(frameDelay - lastFrameMs);
	}
}

bool App::CleanUp()
{
	bool ret = true;

	size_t size = modules.size();
	for (size_t i = 0; i < size; ++i)
	{
		Module* m = modules[i];

		modules[i]->CleanUp();
	}

	return ret;
}

int App::GetArgc() const
{
	return argc;
}

const char* App::GetArgv(int index) const
{
	if(index < argc)
		return args[index];
	else
		return NULL;
}