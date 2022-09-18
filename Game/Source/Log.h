#pragma once

#include <string>


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

	float totalInstance = 0;

protected:

	Log(const char* name, float totalInstance, LogType type, bool* dateFormat);

	std::string name;
	LogType type = LogType::NO_LOG;
	intptr_t id = 0;
	bool* dateFormat = nullptr;
	int date[3] = {0, 0, 0};
};
