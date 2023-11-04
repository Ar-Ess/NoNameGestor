#include "Scene.h"
#include "Action.h"

void Scene::SetScene(Scenes index)
{
	actSet->Invoke((int)index);
}

void Scene::PushScene(Scene* push)
{
	actAdd->Invoke(push);
}

void Scene::Save()
{
	const int save = 1;
	*trgSaveLoad = save;
}

void Scene::Load()
{
	const int load = 0;
	*trgSaveLoad = load;
}
