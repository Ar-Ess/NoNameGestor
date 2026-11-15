#pragma once

#include "Framework/Data/Vector.h"
#include "Framework/Engine/ID.h"

#include "NoNameGestor/Containers/ContainerEnum.h"
#include "NoNameGestor/Utils/FileManager.h"

#include <string>

class InputContainer;
class TotalContainer;
class Container;
struct Configuration;

class GestorSystem
{
public:

	GestorSystem(int index, const char* name, float totalMoney, FileManager::File* file, Configuration* config, String* errorMessage);

	GestorSystem(int index, const FileManager::FileNode& node, FileManager::File* file, Configuration* config, String* errorMessage);

	~GestorSystem();

public:

	bool Update();

	bool Draw(float width, Flag& enable);

	void DrawExport() const;

	void DrawCashFlow(float widthRatio, float initX) const;

	bool Save(int index, FileManager::FileNode node) const;

	void SetFormat(const char* format, int currency);

	StringView Name() const;

	Container* CreateContainer(ContainerType container, const std::string& name = "New Container", bool hidden = false, bool open = true, bool unified = true);

	Container* CreateContainer(ContainerType container, const FileManager::FileNode& containerNode);

private:

	void Export(const Array<Container*>& exporting) const;

	void MoveContainer(unsigned short index, unsigned short position);

public:

	ID id = ID::Empty;

private:

	// Containers
	InputContainer* inputContainer = nullptr;
	TotalContainer* totalContainer = nullptr;
	Vector<Container*> containers;

	// Naming
	std::string name;
	String format;

	// Dependencies
	FileManager::File* file = nullptr;
	Configuration* config = nullptr;
	String* errorMessage = nullptr;

};
