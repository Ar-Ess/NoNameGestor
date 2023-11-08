#pragma once
#include <string>
#include "Framework/Vector.h"
#include <assert.h>
#include "ContainerEnum.h"

#define EXTENSION ".nng"

class ID;
class Container;

class Page
{
public:

	Page();
	~Page();

	void Start(const char* currency);
	void Update();
	void Draw();

	void CreateContainer(ContainerType type);

private:

	void SetFormat(const char* form, const char* currency);

	void DeleteContainer(unsigned int index);

	void SwitchLoadOpen();

	void MoveContainer(unsigned int index, unsigned int position);

	int ReturnContainerIndex(intptr_t id);

public:

	std::string name;
	ID* id = nullptr; // Do not add the header, when having cpp, forward declare and header on cpp

private:

	float money = 0;
	float futureMoney = 0.0f;
	float actualMoney = 0.0f;
	float arrearMoney = 0.0f;
	float constMoney = 0.0f;
	Vector<Container*> containers;
	std::string format;
};