#include "Scene.h"
#include "Defs.h"
#include "LogFunction.h"
#include "imgui/imgui.h"

Scene::Scene(Render* render, Input* input, Window* window, const char* directoryPath, const char* openedFile)
{
	this->render = render;
	this->input = input;
	this->window = window;
	this->economyScene = new EconomyScene(input, directoryPath, openedFile);
}

Scene::~Scene()
{}

bool Scene::Start()
{
	//FIRST SCENE
	if (!SetScene(Scenes::ECONOMY_SCENE)) return false;

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

void Scene::DebugCommands()
{
	if (input->GetKey(SDL_SCANCODE_F11) == KeyState::KEY_DOWN) window->SetWinFullScreen(!window->fullScreen);

	switch (currScene)
	{
	case Scenes::LOGO_SCENE:
		break;
	}
}
