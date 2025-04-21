#ifndef __GESTOR_SYSTEM_H__
#define __GESTOR_SYSTEM_H__

#include <vector>
#include "ContainerEnum.h"

class Container;
class TotalContainer;
class UnasignedContainer;
class FileManager;

#include <string>

class GestorSystem
{
public:

	GestorSystem(bool* showFutureUnasigned, bool* allowFutureCovering, bool* showArrearUnasigned, bool* allowArrearsFill, bool* showConstantTotal, bool* showContainerType);

	~GestorSystem();

	bool Start();

	bool Update();

	bool Draw();

	bool Save(FileManager* file, const char* path);

	bool Load(FileManager* file, const char* path, int& jumplines);

	void SetFormat(const char* format, const char* currency);

	Container* CreateContainer(ContainerType container, const char* name = "New Container", bool hidden = false, bool open = true, bool unified = true);

	void SwitchLoadOpen();

private:

	void DeleteAllContainer();

	// Input from 0 (smallest spacing) to whatever you need
	void AddSpacing(unsigned int spaces = 1);

	// Input from 1 to whatever you need
	void AddSeparator(unsigned int separator = 1);

	// Create helper pop up
	void AddHelper(const char* desc, const char* title = "(?)");

	void DeleteContainer(unsigned short index);

	void MoveContainer(unsigned short index, unsigned short position);

	int ReturnContainerIndex(intptr_t id);


private:

	TotalContainer* totalContainer = nullptr;
	UnasignedContainer* unasignedContainer = nullptr;
	std::vector<Container*> containers;

	std::string format;

	bool* showContainerType;

};

#endif
