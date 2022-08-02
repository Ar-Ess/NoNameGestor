#include "Scene.h"
#include "GuiManager.h"

#include "Defs.h"
#include "Log.h"
#include "imgui/imgui.h"

Scene::Scene(Render* render, Input* input, Textures* texture, Window* window, Audio* audio)
{
	this->gui = new GuiManager(input, render, audio, texture);
	this->render = render;
	this->input = input;
	this->texture = texture;
	this->window = window;
	this->audio = audio;
	this->economyScene = new EconomyScene(input);
}

Scene::~Scene()
{}

bool Scene::Start()
{
	//DEBUG BOOLS
	gui->Start(this);

	//FIRST SCENE
	if (!SetScene(Scenes::ECONOMY_SCENE)) return false;

	//CONTINUE ACTIVITY
	/*activeContinue = false;
	if (FILE* file = fopen("save_game.xml", "r"))
	{
		fclose(file);
		activeContinue = true;
	}*/

	render->SetScale(1); //Qui toqui aquesta linia de codi, i m'entero, no viu un dia més :) <3

	return true;
}

bool Scene::Update(float dt)
{
	bool ret = true;

	switch (currScene)
	{
	case Scenes::LOGO_SCENE:
		ret = UpdateLogoScene(dt);
		break;

	case Scenes::ECONOMY_SCENE:
		ret = UpdateEconomyScene(dt);
		break;
	}

	DebugCommands();

	return (!exit && ret);
}

bool Scene::CleanUp()
{
	LOG("Freeing scene");

	switch (currScene)
	{
	case Scenes::LOGO_SCENE:
		break;

	case Scenes::ECONOMY_SCENE:
		economyScene->CleanUp();
		break;
	}

	return true;
}

// SCENE MANAGER

bool Scene::SetScene(Scenes scene)
{
	bool ret = true;
	CleanUp();

	prevScene = currScene;
	currScene = scene;

	switch (currScene)
	{
	case Scenes::LOGO_SCENE:
		ret = SetLogoScene();
		break;

	case Scenes::ECONOMY_SCENE:
		ret = SetEconomyScene();
		break;
	}

	//easing.ResetIterations();

	return ret;
}

bool Scene::SetLogoScene()
{
	return true;
}

bool Scene::SetEconomyScene()
{
	bool ret = true;

	ret = economyScene->Start();

	return ret;
}

bool Scene::UpdateLogoScene(float dt)
{
	bool ret = true;

	if (input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_DOWN) ret = SetScene(Scenes::ECONOMY_SCENE);

	return ret;
}

bool Scene::UpdateEconomyScene(float dt)
{
	bool ret = true;

	ret = economyScene->Update();

	ret = economyScene->Draw();

	return ret;
}

// GUI CONTROLS

bool Scene::OnGuiMouseClickEvent(GuiControl* control, float value, bool check)
{
	switch (currScene)
	{
	case Scenes::ECONOMY_SCENE:
		switch (control->id)
		{
			break;
		}
		break;
	}

	return true;
}

void Scene::DebugCommands()
{
	if (input->GetKey(SDL_SCANCODE_F1) == KeyState::KEY_DOWN) gui->debug = !gui->debug;

	if (input->GetKey(SDL_SCANCODE_F11) == KeyState::KEY_DOWN) window->SetWinFullScreen(!window->fullScreen);

	switch (currScene)
	{
	case Scenes::LOGO_SCENE:
		break;
	}
}
