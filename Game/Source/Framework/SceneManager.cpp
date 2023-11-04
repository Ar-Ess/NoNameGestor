#include "SceneManager.h"
#include "../EconomyScene.h"
#include <functional>
#include "ProgressManager.h"

SceneManager::SceneManager(Render* render, Input* input, Window* window)
{
	this->render = render;
	this->input = input;
	this->window = window;

	actSet += std::bind(&SceneManager::ChangeScene, this, std::placeholders::_1);
	actAdd += std::bind(&SceneManager::PushScene  , this, std::placeholders::_1);

}

SceneManager::~SceneManager()
{
}

bool SceneManager::Awake()
{
	return true;
}

bool SceneManager::Start()
{
	bool ret = true;

	PushScene(new EconomyScene());
	ret = ChangeScene(0);

	return ret;
}

bool SceneManager::Update(float dt)
{
	if (currScene < 0) return true;

	if (!scenes[currScene]->Update(dt)) !trgUpdate;

	if (!scenes[currScene]->Draw(dt)) !trgUpdate;

	return trgUpdate();
}

bool SceneManager::CleanUp()
{
	LOG("Freeing scene manager");

	scenes[currScene]->CleanUp();

	scenes.Clear();

	return true;
}

bool SceneManager::ChangeScene(int scene)
{
	bool ret = true;

	if (currScene > -1 && scene != currScene) ret = scenes[currScene]->CleanUp();

	prevScene = currScene;
	currScene = scene;

	ret = scenes[currScene]->Start();

	return ret;
}

void SceneManager::PushScene(Scene* scene)
{
	scenes.Add(scene);
	scene->actAdd = &actAdd;
	scene->actSet = &actSet;
	scene->trgSaveLoad = &trgSaveLoad;
	scene->render = render;
	scene->input = input;
	scene->window = window;
}

void SceneManager::SaveLoad(bool save)
{
	if (!ProgressManager::Begin(save))
	{
		LOG("PROGRESS MANAGER ERROR: Could not begin the Progress Manager");
		return;
	}

	scenes.Iterate<bool>
	(
		save,
		[](Scene* scene, bool save)
		{
			if (save) scene->SaveScene();
			else scene->LoadScene();
		}
	);

	ProgressManager::End(save);

	trgSaveLoad = -1;
}
