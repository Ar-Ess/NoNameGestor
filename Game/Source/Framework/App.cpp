#include "App.h"
#include "Defs.h"

#include "Window.h"
#include "Input.h"
#include "Render.h"
#include "SceneManager.h"

#define RELEASE_MODE 0

App::App(int argc, char* args[]) : argc(argc), args(args)
{
	Window* win = new Window();
	Input* input = new Input(win);
	Render* render = new Render(win);
	SceneManager* scene = new SceneManager(render, input, win);
	trgSaveLoad = &scene->trgSaveLoad;
	actSaveLoad += std::bind(&SceneManager::SaveLoad, scene, std::placeholders::_1);

	modules.Add(win);
	modules.Add(input);
	modules.Add(scene);
	modules.Add(render);
}

App::~App()
{
	modules.Clear();
}

bool App::Start()
{
	bool ret = true;
	
	SDL_Init(0);

	modules.Iterate<bool&>
	(
		ret,
		[](Module* m, bool& ret)
		{
			if (!m->Awake()) ret = false;
		}
	);

	modules.Iterate<bool&>
	(
		ret,
		[](Module* m, bool& ret)
		{
			if (!m->Start()) ret = false;
		}
	);

	return ret;
}

bool App::Update()
{
	bool ret = true;

	// Preparing Update
	frameCount++;
	dt = (float)frameTime.ReadSec();
	frameTime.Start();

	modules.Iterate<bool&, float>
	(
		ret,
		dt,
		[](Module* m, bool& ret, float dt)
		{
			if (!m->Active()) return;
			if (!m->PreUpdate(dt)) ret = false;
		}
	);

	modules.Iterate<bool&, float>
	(
		ret,
		dt,
		[](Module* m, bool& ret, float dt)
		{
			if (!m->Active()) return;
			if (!m->Update(dt)) ret = false;
		}
	);

	// Finishing Update
	averageFps = float(frameCount) / (float)startupTime.ReadSec();
	uint32 lastFrameMs = frameTime.Read();
	if (FrameDelay() > (float)lastFrameMs) SDL_Delay((uint32)FrameDelay() - lastFrameMs);

	// Save & Load Logic
	if (*trgSaveLoad != -1) actSaveLoad.Invoke((bool)*trgSaveLoad);

	return ret;
}

bool App::CleanUp()
{
	bool ret = true;

	modules.Iterate<bool&>
	(
		ret,
		[](Module* m, bool& ret)
		{
			if (!m->End()) ret = false;
		}
	);

	modules.Iterate<bool&>
	(
		ret,
		[](Module* m, bool& ret)
		{
			if (!m->CleanUp()) ret = false;
		}
	);

	return ret;
}
