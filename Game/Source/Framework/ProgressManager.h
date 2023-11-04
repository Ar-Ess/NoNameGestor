#pragma once

#include "../External/JSON/json.hpp"
#include <string>

class ProgressManager
{
public:

	friend class SceneManager;

public:

	static void Archive(unsigned int archive);

	static bool SaveInt(const char* field, int value);
	
	static bool SaveFloat(const char* field, float value);
	
	static bool SaveBool(const char* field, bool value);
	
	static bool SaveString(const char* field, const char* value);

	static int LoadInt(const char* field);

	static float LoadFloat(const char* field);

	static bool LoadBool(const char* field);

	static std::string LoadString(const char* field);
		
private:

	static bool Begin(bool save);

	static bool End(bool save);

	ProgressManager() = default;
	~ProgressManager() = default;
	ProgressManager(const ProgressManager&) = delete;
	ProgressManager(ProgressManager&&) = delete;
	ProgressManager& operator=(const ProgressManager&) = delete;
	ProgressManager& operator=(ProgressManager&&) = delete;

	static ProgressManager& Instance();

private:

	unsigned int archive = 0;
	bool init = false;
	bool active = false;
	bool save = false;
	nlohmann::json* data = nullptr;

};