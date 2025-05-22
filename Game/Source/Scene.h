#ifndef __SCENE_H__
#define __SCENE_H__

#include "Module.h"
#include "Input.h"
#include "Window.h"
#include "Render.h"
#include "EconomyScene.h"

enum class Scenes
{
	NO_SCENE = -1,
	LOGO_SCENE,
	ECONOMY_SCENE,
};

class Scene : public Module
{
public:

	Scene(Render* render, Input* input, Window* window, const char* directoryPath = nullptr, const char* openedFile = nullptr);

	virtual ~Scene();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

// SCENE MANAGER

	inline Scenes GetCurrScene() const { return currScene; }

	bool SetScene(Scenes scene);

private:

	Scenes currScene = Scenes::NO_SCENE;
	Scenes prevScene = Scenes::NO_SCENE;

	//Setters
	bool SetLogoScene();
	bool SetEconomyScene();

	//Updaters
	bool UpdateLogoScene(float dt);
	bool UpdateEconomyScene(float dt);

private: // Methods

	void DebugCommands();

private: // Variables

	Input* input = nullptr;
	Window* window = nullptr;
	Render* render = nullptr;

	EconomyScene* economyScene = nullptr;

	bool exit = false;
	bool activeContinue = false;
};

#endif // __SCENE_H__