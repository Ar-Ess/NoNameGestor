#pragma once

#include "Framework/Engine/Module.h"
#include "Framework/Data/Vector.h"
#include "Framework/Utils/Action.h"

#include <type_traits>
#include <concepts>

class Scene;

class SceneManager : public Module<SceneManager>
{

	friend class App;
	friend class ProgressManager;

public:

	// Changes the current scene to the one at the given type.
	// If the scene type is invalid, it will return false and do nothing. "PushScene" that scene first.
	// If "force" is true, it will allow to change to the inputted scene type even if it is the current one, reloading it.
	template<class T>
	static bool ChangeScene(bool force = false)
	{
		static_assert(std::derived_from<T, Scene>, "SceneManager::PushScene<Scene>() can't push T because it is not a class derived from Scene.");
		static_assert(std::is_default_constructible_v<T>, "SceneManager::PushScene<Scene>() can't push T because it is not default constructible.");
		static_assert(!std::same_as<T, Scene>, "SceneManager::PushScene<Scene>() can't push T because it is the Scene class. T must derive from it, not be it.");
		static_assert(std::same_as<T, std::remove_cvref_t<T>>, "SceneManager::PushScene<Scene>() can't push T because T must be an unqualified type (no const, volatile, &, or &&).");
		static_assert(!std::is_pointer_v<T>, "SceneManager::PushScene<Scene>() can't push T because it must be a scene type, not a pointer type. Use T, not T*.");

		auto& inst = Instance();
		Scene* scene = nullptr;
		if (!inst.scenes.Retrieve([](Scene* scene) { return dynamic_cast<T*>(scene) != nullptr; }, scene))
		{
			Debug::Log("SceneManager ERROR: CAN NOT CHANGE TO A SCENE THAT IS NOT IN THE STACK");
			return false;
		}

		return inst.ChangeSceneInternal(scene, force);
	}

	// Pushes a new scene to the stack of scenes.
	// It will be initialized and awake, but it will not be started until it is changed to.
	template<class T>
	static bool PushScene()
	{
		static_assert(std::derived_from<T, Scene>, "SceneManager::PushScene<Scene>() can't push T because it is not a class derived from Scene (or at least, not derived from 'public Scene').");
		static_assert(std::is_default_constructible_v<T>, "SceneManager::PushScene<Scene>() can't push T because it is not default constructible.");
		static_assert(!std::same_as<T, Scene>, "SceneManager::PushScene<Scene>() can't push T because it is the Scene class. T must derive from it, not be it.");
		static_assert(std::same_as<T, std::remove_cvref_t<T>>, "SceneManager::PushScene<Scene>() can't push T because T must be an unqualified type (no const, volatile, &, or &&).");
		static_assert(!std::is_pointer_v<T>, "SceneManager::PushScene<Scene>() can't push T because it must be a scene type, not a pointer type. Use T, not T*.");
		
		auto& inst = Instance();

		if (inst.scenes.Contains([](Scene* scene) { return dynamic_cast<T*>(scene) != nullptr; }))
		{
			Debug::Log("SceneManager ERROR: CAN NOT PUSH A SCENE THAT IS ALREADY IN THE STACK");
			return false;
		}

		return inst.PushSceneInternal(new T());
	}

private:

	SceneManager();

	bool Start() override;

	bool PreUpdate(float dt) override;

	bool Update(float dt) override;

	bool FixedUpdate(float dt) override;

	static void Save();

	static void Load();

	bool ChangeSceneInternal(Scene* scene, bool force);

	bool PushSceneInternal(Scene* scene);

private:

	Scene* currScene = nullptr;
	Scene* prevScene = nullptr;

	Vector<Scene*, true> scenes;

};
