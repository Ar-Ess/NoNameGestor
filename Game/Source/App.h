#ifndef __APP_H__
#define __APP_H__

#include "Module.h"
#include "PerfTimer.h"
#include "Timer.h"

#include <vector>
#include "External/imgui/imgui.h"

// Modules
class Window;
class Input;
class Render;
class Scene;

class App
{
public:
	App(int argc, char* args[]);

	virtual ~App();

	bool Start();

	bool Update();

	bool CleanUp();

	void AddModule(Module* module);

	int GetArgc() const;

	const char* GetArgv(int index) const;

	inline uint64 GetFrameCount() const
	{
		return frameCount;
	}

	inline float GetDeltaTime() const
	{
		return dt;
	}

private:

	void PrepareUpdate();

	void FinishUpdate();

	bool InitImGui();

public:

	// Modules
	Window* win = nullptr;
	Input* input = nullptr;
	Render* render = nullptr;
	Scene* scene = nullptr;

private:

	int argc = 0;

	char** args = nullptr;

	std::vector<Module*> modules;

	PerfTimer ptimer;

	uint64 frameCount = 0;

	Timer startupTime;
	Timer frameTime;
	Timer lastSecFrameTime;

	uint32 lastSecFrameCount = 0;

	uint32 prevLastSecFrameCount = 0;

	float dt = 0.0f;

	const uint32 frameDelay = 1000 / 60;

	int	cappedMs = -1;

};

#endif	// __APP_H__