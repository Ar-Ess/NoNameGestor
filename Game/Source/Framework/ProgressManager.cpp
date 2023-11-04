#include "ProgressManager.h"
#include <assert.h>
#include "Defs.h"
#include <fstream>
#include <string>

#define INIT_ASSERT(x) assert(x == true, "PROGRESSION MANAGER ERROR: CAN NOT LOAD AN NON EXISTENT FILE, SAVE INFORMATION FIRST")
#define BEGIN_ASSERT(x) assert(x == false, "PROGRESSION MANAGER ERROR: MANAGER BEGINS TWICE")
#define END_ASSERT(x) assert(x == true, "PROGRESSION MANAGER ERROR: MANAGER ENDS TWICE")
#define SAVE_ASSERT(x) assert(x == true, "PROGRESSION MANAGER ERROR: CAN NOT USE A SAVE FUNCTION WHILE LOADING INFORMATION")
#define LOAD_ASSERT(x) assert(x == false, "PROGRESSION MANAGER ERROR: CAN NOT USE A LOAD FUNCTION WHILE SAVING INFORMATION")
#define FILE_NAME(archive) std::string(std::to_string(archive) + std::string("_save_game.json")).c_str()

ProgressManager& ProgressManager::Instance()
{
	static ProgressManager instance;
	return instance;
}

void ProgressManager::Archive(unsigned int archive)
{
	Instance().archive = archive;
}

bool ProgressManager::SaveInt(const char* field, int value)
{
	ProgressManager& p = Instance();
	SAVE_ASSERT(p.save);

	nlohmann::json* in = p.data;

	if (!in) return false;

	(*in)[field] = value;

	return true;
}

bool ProgressManager::SaveFloat(const char* field, float value)
{
	ProgressManager& p = Instance();
	SAVE_ASSERT(p.save);

	nlohmann::json* in = p.data;

	if (!in) return false;

	(*in)[field] = value;

	return true;
}

bool ProgressManager::SaveBool(const char* field, bool value)
{
	ProgressManager& p = Instance();
	SAVE_ASSERT(p.save);

	nlohmann::json* in = p.data;

	if (!in) return false;

	(*in)[field] = value;

	return true;
}

bool ProgressManager::SaveString(const char* field, const char* value)
{
	ProgressManager& p = Instance();
	SAVE_ASSERT(p.save);

	nlohmann::json* in = p.data;

	if (!in) return false;

	(*in)[field] = value;

	return true;
}

int ProgressManager::LoadInt(const char* field)
{
	ProgressManager& p = Instance();
	LOAD_ASSERT(p.save);

	nlohmann::json* in = p.data;

	if (!in) return false;

	return (*in)[field].get<int>();
}

float ProgressManager::LoadFloat(const char* field)
{
	ProgressManager& p = Instance();
	LOAD_ASSERT(p.save);

	nlohmann::json* in = p.data;

	return (*in)[field].get<float>();
}

bool ProgressManager::LoadBool(const char* field)
{
	ProgressManager& p = Instance();
	LOAD_ASSERT(p.save);

	nlohmann::json* in = p.data;

	return (*in)[field].get<bool>();
}

std::string ProgressManager::LoadString(const char* field)
{
	ProgressManager& p = Instance();
	LOAD_ASSERT(p.save);

	nlohmann::json* in = p.data;

	return (*in)[field].get<std::string>();
}

bool ProgressManager::Begin(bool save)
{
	ProgressManager& p = Instance();
	bool ret = true;
	BEGIN_ASSERT(p.active);
	if (!save) INIT_ASSERT(p.init);
	p.active = true;
	p.save = save;

	if (save)
	{
		p.data = new nlohmann::json();
		p.init = true;
	}
	else
	{
		std::ifstream saveFile(FILE_NAME(p.archive));
		if (saveFile.good()) 
			p.data = new nlohmann::json(nlohmann::json::parse(saveFile));
		else 
			ret = false;
		saveFile.close();
	}

	return ret;
}

bool ProgressManager::End(bool save)
{
	ProgressManager& p = Instance();
	END_ASSERT(p.active);
	p.active = false;

	if (save)
	{
		std::ofstream file(FILE_NAME(p.archive));
		file << (*p.data);
		file.close();
	}

	delete p.data;
	p.data = nullptr;

	return true;
}
