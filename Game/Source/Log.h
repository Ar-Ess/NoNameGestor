#pragma once

#include <string>
#include <assert.h>


enum class LogType
{
	NO_LOG = -1,
	MOVEMENT_LOG,
	INFORMATIVE_LOG
};

class Log
{
public:

	virtual ~Log() {}

	virtual void Draw(const char* currency)
	{

	}

	virtual float GetOldInstance() const
	{
		return 0.0f;
	}

	virtual float GetNewInstance() const
	{
		return 0.0f;
	}

	LogType GetType() const
	{
		return type;
	}

	const char* GetName() const
	{
		return name.c_str();
	}

	intptr_t GetId() const
	{
		return id;
	}

	void SetDate(int day, int month, int year)
	{
		date[0] = day;
		date[1] = month;
		date[2] = year;
	}

	int GetDate(int index) const
	{
		// Index must be between 0 && 2
		assert(index >= 0 && index < 3);
		return date[index];
	}

	float totalInstance = 0;

protected:

	Log(const char* name, float totalInstance, LogType type, bool* dateFormat);

	std::string name;
	LogType type = LogType::NO_LOG;
	intptr_t id = 0;
	bool* dateFormat = nullptr;
	int date[3] = {0, 0, 0};
};
