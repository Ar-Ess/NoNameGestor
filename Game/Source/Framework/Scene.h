#pragma once

class Render;
class Input;
class AssetsManager;
class Window;
class GuiManager;
class AudioSystem;
class Trigger;
template<class ...T> class Action;

enum class Scenes
{
	MAIN_SCENE,
};

class Scene
{
public:

	friend class SceneManager;

public: // Methods

	virtual ~Scene() {}

	virtual bool Start() { return true; }

	virtual bool Update(float dt) { return true; }

	virtual bool Draw(float dt) { return true; }

	virtual bool CleanUp() { return true; }

	// This function must not be called from inside a scene
	// It must only be overriden with the parameters to save
	// To save the scenes, call Save()
	virtual void SaveScene() {}

	// This function must not be called from inside a scene
	// It must only be overriden with the parameters to load
	// To load the scenes, call Load()
	virtual void LoadScene() {}

protected: // Methods

	Scene() {}

	void SetScene(Scenes index);

	void PushScene(Scene* push);

	void Save();
	
	void Load();

protected: // Variables

	Render* render = nullptr;
	Input* input = nullptr;
	AssetsManager* assets = nullptr;
	Window* window = nullptr;
	GuiManager* gui = nullptr;
	AudioSystem* audio = nullptr;

private: // Variables

	Action<int>*    actSet = nullptr;
	Action<Scene*>* actAdd = nullptr;
	int* trgSaveLoad = nullptr;

};