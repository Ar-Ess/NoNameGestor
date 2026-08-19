#pragma once

#include <concepts>

class AppModule
{
	friend class App;

protected:

	AppModule() : active(true) {}

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

	virtual bool FixedUpdate(float dt)
	{
		return true;
	}

	virtual bool CleanUp()
	{
		return true;
	}

protected:

	bool active = true;

};

template <typename T>
class Module : public AppModule
{
protected:

	Module() : AppModule()
	{
		static_assert(std::is_base_of_v<Module<T>, T>,
			"T must derive from Module<T> (CRTP requirement)");

		instance = static_cast<T*>(this);
	}

	static T& Instance()
	{
		return *instance;
	}

private:

	static T* instance;
};

template<typename T>
T* Module<T>::instance = nullptr;