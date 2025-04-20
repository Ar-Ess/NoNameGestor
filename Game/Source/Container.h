#pragma once

#include "Defs.h"
#include "Label.h"
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"
#include <string>
#include <vector>
#include "ContainerEnum.h"

#define MAX_MONEY 340282000000000000000000000000000000000.0f

static float textFieldSize = 150.0f;

class Container
{
public: // Functions

	virtual ~Container() 
	{
		name.clear();
		name.shrink_to_fit();
		namePtr = nullptr;
	}

	virtual void Start(const char* currency) {}

	virtual void Update() {}

	virtual void Draw() {}

	const char* GetName() const
	{
		return namePtr->c_str();
	}

	std::string* GetString()
	{
		return namePtr;
	}

	const char* GetTypeString() const
	{
		switch (type)
		{
		case ContainerType::FILTER:   return "FILTER";
		case ContainerType::LIMIT :   return "LIMIT ";
		case ContainerType::FUTURE:   return "FUTURE";
		case ContainerType::ARREAR:   return "ARREAR";
		case ContainerType::CONSTANT: return "CONST ";
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

	unsigned int GetSize() const
	{
		return size;
	}

	void NewLabel(const char* name = "New Container", float money = 0.0f, float limit = 1.0f)
	{
		if (!labels.empty() && !loadOpen)
		{
			unified = false;
			SwapNames();
		}
		labels.push_back(new Label(name, money, limit));
		size = labels.size();
	}

	void ClearLabels()
	{
		for (Label* l : labels) RELEASE(l);
		labels.clear();
		labels.shrink_to_fit();
		size = 0;
	}

	float GetLabelMoney(int i) const
	{
		return labels[i]->money;
	}

	const char* GetLabelName(int i) const
	{
		return labels[i]->name.c_str();
	}

	void SetMoney(float money)
	{
		this->money = money;
	}

	virtual void SetCurrency(const char* currency)
	{
		SetFormat("%.2f ", currency);
	}

	void SwapNames()
	{
		if (unified)
			namePtr = &labels[0]->name;
		else
			namePtr = &name;
	}

protected: // Functions

	Container(const char* name, bool hidden, bool open, bool unified, float* totalMoneyPtr, ContainerType type) 
	{
		this->money = 0;
		this->name = name;
		this->namePtr = &this->name;
		this->type = type;
		this->hidden = hidden;
		this->open = open;
		this->unified = unified;
		this->totalMoneyPtr = totalMoneyPtr;
		id = reinterpret_cast<int>(this);

	}

	void SetFormat(const char* form, const char* currency)
	{
		format.clear();
		format.shrink_to_fit();
		format += form;
		format += currency;
	}

	void DeleteLabel(int index)
	{
		labels[index]->name.clear();
		labels[index]->name.shrink_to_fit();
		labels.erase(labels.begin() + index);
		size = labels.size();
	}

public: // Variables

	bool hidden = false;
	bool unified = true;
	bool open = false;
	bool loadOpen = false;
	bool exporting = false;

protected: // Variables

	float* totalMoneyPtr = nullptr;
	std::vector<Label*> labels;
	float money = 0.0f;
	std::string* namePtr = nullptr; //-TODO: quan carregem fitxer, mirar si esta unified o no i asignar nom corresponent
	std::string name;
	std::intptr_t id = 0;
	ContainerType type = ContainerType::NO_CONTAINER;
	std::string format;

	unsigned int size = 0; //-TODO: Mirar si puc fer-ho amb size++/size-- en comptes d'igualar ->size()
};