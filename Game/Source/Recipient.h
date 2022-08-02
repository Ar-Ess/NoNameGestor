#ifndef __RECIPIENT_H__
#define __RECIPIENT_H__

#include "imgui/imgui.h"
#include <string>

enum class RecipientType
{
	NO_RECIPIENT,
	TOTAL_MONEY,
	UNASIGNED_MONEY,
	FILTER
};

class Recipient
{
public: // Functions

	~Recipient() {}

	virtual void Start() {}
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

	void SetMoney(float money)
	{
		this->money = money;
	}

protected: // Functions

	Recipient(const char* name, float money, RecipientType type) 
	{
		this->money = money;
		this->name = name;
		this->type = type;
		id = reinterpret_cast<int>(this);
	}

protected: // Variables

	float money = 0.0f;
	std::string name;
	std::intptr_t id = 0;
	RecipientType type = RecipientType::NO_RECIPIENT;
};

#endif // !__RECIPIENT_H__