#pragma once

#include "Module.h"
#include "LightTimer.h"
#include "Action.h"
#include "Random.h"
#include "SDL/include/SDL_main.h"

class App
{
private:

	float AverageFPS() const
	{
		return averageFps;
	}

	uint64 FrameCount() const
	{
		return frameCount;
	}

	void SetTargetFPS(unsigned int fps)
	{
		targetFPS = fps;
	}

	int TargetFPS() const
	{
		return targetFPS;
	}

	App(int argc, char* args[]);

	virtual ~App();

	bool Start();

	bool Update();

	bool CleanUp();

	float FrameDelay() const
	{
		return 1000.0f / targetFPS;
	}

private:

	friend int main(int argc, char* args[]);

	int argc = 0;
	char** args = nullptr;

	// Modules
	Array<Module*> modules;

	// Save & Load
	int* trgSaveLoad = nullptr;
	Action<bool> actSaveLoad;

	// Timing
	uint64 frameCount = 0;
	float averageFps = 0.0f;
	float dt = 0.0f;
	int targetFPS = 60;

	LightTimer startupTime;
	LightTimer frameTime;

};