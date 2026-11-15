#pragma once

#include "Framework/Engine/Module.h"
#include "Framework/Data/String.h"
#include "Framework/Data/Flag.h"
#include "Framework/Utils/Trigger.h"

#include <concepts>

template<typename T>
concept AllowedType =
std::integral<std::remove_cvref_t<T>> ||
std::floating_point<std::remove_cvref_t<T>> ||
std::same_as<std::remove_cvref_t<T>, String>;

class ProgressManager : public Module<ProgressManager>
{
public:

	friend class App;
	friend class SceneManager;

public:

	// Call this function to initialize a Save request for all scenes.
	static void RequestSave();

	// Call this function to initialize a Load request for all scenes.
	static void RequestLoad();

	// Call this function to save a specific value of the game.
	// Can only be used inside a Scene::SaveScene() function
	template<AllowedType T>
	static bool SaveValue(const String& field, T value);

	// Call this function to load a specific value of the game.
	// Can only be used inside a Scene::LoadScene() function
	template<AllowedType T>
	static bool LoadValue(const String& field, T& value);

	static bool ArchiveExists(unsigned int archive);
		
private:

	bool Awake() override;

	bool PreUpdate(float dt) override;

	bool Update(float dt) override;

	ProgressManager();
	~ProgressManager();
	ProgressManager(const ProgressManager&) = delete;
	ProgressManager(ProgressManager&&) = delete;
	ProgressManager& operator=(const ProgressManager&) = delete;
	ProgressManager& operator=(ProgressManager&&) = delete;

	static const char* GetArchiveName(unsigned int archive);

public:

	// The current archive to save to
	static unsigned int archive;

private:

	Trigger requestSave;
	Trigger requestLoad;

	class JsonData;
	JsonData* data = nullptr;

	//static nlohmann::json* data;
	Flag state;
	enum class ProgressState
	{
		ACTIVE = 0,
		SAVE = 1,
	};

};
