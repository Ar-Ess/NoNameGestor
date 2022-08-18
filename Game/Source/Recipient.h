#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"
#include <string>
static float textFieldSize = 150.0f;

enum class RecipientType
{
	NO_RECIPIENT,
	TOTAL_MONEY,
	UNASIGNED_MONEY,
	FILTER,
	LIMIT,
	FUTURE,
	ARREAR,
};

class Recipient
{
public: // Functions

	virtual ~Recipient() 
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
		case RecipientType::FILTER: return "FILTER";
		case RecipientType::LIMIT: return "LIMIT ";
		case RecipientType::FUTURE: return "FUTURE";
		case RecipientType::ARREAR: return "ARREAR";
		}
		return "NO RECIPIENT";
	}

	RecipientType GetType() const
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

	Recipient(const char* name, float money, bool hidden, RecipientType type) 
	{
		this->money = money;
		this->name = name;
		this->type = type;
		this->hidden = hidden;
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

protected: // Variables

	float money = 0.0f;
	std::string name;
	std::intptr_t id = 0;
	RecipientType type = RecipientType::NO_RECIPIENT;
	std::string format;
};