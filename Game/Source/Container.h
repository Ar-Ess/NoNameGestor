#pragma once

#include "Defs.h"
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"
#include <string>
static float textFieldSize = 150.0f;

enum class ContainerType
{
	NO_CONTAINER,
	TOTAL_MONEY,
	UNASIGNED_MONEY,
	FILTER,
	LIMIT,
	FUTURE,
	ARREAR,
};

class Container
{
public: // Functions

	virtual ~Container() 
	{
		name.clear();
		name.shrink_to_fit();
	}

	virtual void Start(const char* currency) {}

	virtual void Update() {}

	virtual void Draw() {}

	const char* GetName() const
	{
		return name.c_str();
	}

	std::string* GetString()
	{
		return &name;
	}

	const char* GetTypeString() const
	{
		switch (type)
		{
		case ContainerType::FILTER: return "FILTER";
		case ContainerType::LIMIT : return "LIMIT ";
		case ContainerType::FUTURE: return "FUTURE";
		case ContainerType::ARREAR: return "ARREAR";
		}
		return "NO CONTAINER";
	}

	ContainerType GetType() const
	{
		return type;
	}

	intptr_t GetId() const
	{
		return id;
	}

	float GetMoney() const
	{
		return money;
	}

	float* GetMoneyPtr()
	{
		return &money;
	}

	void SetMoney(float money)
	{
		this->money = money;
	}

	virtual void SetCurrency(const char* currency)
	{
		SetFormat("%.2f ", currency);
	}

protected: // Functions

	Container(const char* name, float money, bool hidden, bool open, ContainerType type) 
	{
		this->money = money;
		this->name = name;
		this->type = type;
		this->hidden = hidden;
		this->open = open;
		id = reinterpret_cast<int>(this);
	}

	void SetFormat(const char* form, const char* currency)
	{
		format.clear();
		format.shrink_to_fit();
		format += form;
		format += currency;
	}

public: // Variables

	bool hidden = false;
	bool open = false;
	bool loadOpen = false;

protected: // Variables

	float money = 0.0f;
	std::string name;
	std::intptr_t id = 0;
	ContainerType type = ContainerType::NO_CONTAINER;
	std::string format;
};