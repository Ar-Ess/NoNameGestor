#ifndef __SCENE_H__
#define __SCENE_H__

#include "Module.h"
#include "Input.h"
#include "Window.h"
#include "Audio.h"
#include "Textures.h"
#include "EconomyScene.h"

class GuiControl;
class GuiManager;

enum class Scenes
{
	NO_SCENE = -1,
	LOGO_SCENE,
	ECONOMY_SCENE,
};

class Scene : public Module
{
public:

	Scene(Render* render, Input* input, Textures* texture, Window* window, Audio* audio);

	virtual ~Scene();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	bool OnGuiMouseClickEvent(GuiControl* control, float value, bool check);

// SCENE MANAGER

	Scenes GetCurrScene() const
	{
		return currScene;
	}

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

	GuiManager* gui = nullptr;
	Render* render = nullptr;
	Input* input = nullptr;
	Textures* texture = nullptr;
	Window* window = nullptr;
	Audio* audio = nullptr;

	EconomyScene* economyScene = nullptr;

	bool exit = false;
	bool activeContinue = false;

	suint lvl = 0;
};

#endif // __SCENE_H__