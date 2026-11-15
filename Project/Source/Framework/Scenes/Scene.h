#pragma once

#include "Framework/Input/InputManager.h"
#include "Framework/Assets/AssetsManager.h"
#include "Framework/Window/Window.h"
#include "Framework/GUI/GuiManager.h"
#include "Framework/Audio/AudioSystem.h"
#include "Framework/Coroutine/CoroutineManager.h"
#include "Framework/Progress/ProgressManager.h"
#include "Framework/Render/Render.h"
#include "Framework/Scenes/SceneManager.h"

class Scene
{
public:

	friend class SceneManager;

public: // Methods

	virtual ~Scene() {}
	
	virtual bool Awake() { return true; }

	virtual bool Start() { return true; }

	virtual bool Update(float dt) { return true; }

	virtual bool FixedUpdate(float dt) { return true; }

	virtual bool Draw(float dt) { return true; }

	virtual bool End() { return true; }

	virtual bool CleanUp() { return true; }

	// This function must not be called from inside a scene
	// It must only be overriden with the parameters to save
	// To save all the scenes, call ProgressManager::RequestSave()
	virtual void SaveScene() {}

	// This function must not be called from inside a scene
	// It must only be overriden with the parameters to load
	// To load all the scenes, call ProgressManager::RequestLoad()
	virtual void LoadScene() {}

public: // Operators

	bool operator==(const Scene& scene) const;

protected: // Methods

	Scene();

private:

	const ID id;

};