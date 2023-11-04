#pragma once

#include "Defs.h"

class Module
{

	friend class App;

public:

	void Active(bool active)
	{
		this->active = active;
	}

	bool Active() const
	{
		return active;
	}

	virtual ~Module()
	{

	}

protected:

	Module() : active(true) {}

	virtual bool Awake()
	{
		return true;
	}

	virtual bool Start()
	{
		return true;
	}

	virtual bool PreUpdate(float dt)
	{
		return true;
	}

	virtual bool Update(float dt)
	{
		return true;
	}

	virtual bool End()
	{
		return true;
	}

	virtual bool CleanUp()
	{
		return true;
	}


private:

	bool active = true;

};