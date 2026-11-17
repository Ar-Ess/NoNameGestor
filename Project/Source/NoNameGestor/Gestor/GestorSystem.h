#pragma once

#include "NoNameGestor/Audio/VoiceNode.h"
#include "NoNameGestor/Containers/ContainerEnum.h"
#include "NoNameGestor/Utils/FileManager.h"

#include <string>
#include <vector>

class InputContainer;
class TotalContainer;
class Container;
struct Configuration;

class GestorSystem : public VoiceNode
{
public:

	GestorSystem(int index, std::string&& name, float totalMoney, FileManager::File* file, Configuration* config, String* errorMessage);

	GestorSystem(int index, const FileManager::FileNode& node, FileManager::File* file, Configuration* config, String* errorMessage);

	~GestorSystem() override;

	std::vector<VoiceNode*> Children() const override;
	std::string Describe(DetailLevel level) const override;

public:

	bool Update();

	bool Draw(float width, Flag& enable);

	void DrawExport() const;

	void DrawCashFlow(float widthRatio, float initX) const;

	bool Save(int index, FileManager::FileNode node) const;

	void SetFormat(const char* format, int currency);

	Container* CreateContainer(ContainerType container, std::string&& name = "New Container", bool hidden = false, bool open = true, bool unified = true);

	Container* CreateContainer(ContainerType container, const FileManager::FileNode& containerNode);

private:

	void Export(const std::vector<Container*>& exporting) const;

	void MoveContainer(unsigned short index, unsigned short position);

private:

	// Containers
	InputContainer* inputContainer = nullptr;
	TotalContainer* totalContainer = nullptr;
	std::vector<Container*> containers;

	// Naming
	String format;

	// Dependencies
	FileManager::File* file = nullptr;
	Configuration* config = nullptr;
	String* errorMessage = nullptr;

};
