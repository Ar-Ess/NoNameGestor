#include "Log.h"

Log::Log(const char* name, float totalInstance, LogType type, bool* dateFormat)
{
	this->name = name;
	this->type = type;
	this->dateFormat = dateFormat;
	this->totalInstance = totalInstance;
	this->id = reinterpret_cast<int>(this);
}
