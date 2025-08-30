#ifndef __GESTOR_SYSTEM_H__
#define __GESTOR_SYSTEM_H__

#include <vector>
#include "ContainerEnum.h"

class Container;
class InputContainer;
class TotalContainer;
class FileManager;
class ImFont;

#include <string>

class GestorSystem
{
public:

	GestorSystem(const char* name, bool* showFutureUnasigned, bool* showContainerType, std::string* openFileName, std::string* openFilePath, ImFont* bigFont, float* textFieldSize, std::string* errorMessage);

	~GestorSystem();

	bool Start();

	bool Update();

	bool Draw();

	void DrawExport();

	void ExportGestor(std::vector<Container*>* exporting);

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

	void AddClearInputText(const char* name, std::string* buffer);

private:

	InputContainer* inputContainer = nullptr;
	TotalContainer* totalContainer = nullptr;
	std::vector<Container*> containers;

	std::string name;
	std::string format;

	std::string* openFileName = nullptr;
	std::string* openFilePath = nullptr;

	bool* showContainerType = nullptr;
	std::intptr_t id = 0;

	ImFont* bigFont = nullptr;

	float* textFieldSize = nullptr;
	std::string* errorMessage = nullptr;

};

#endif
