#include "GestorSystem.h"

#include "Framework/Engine/Debug.h"

#include "NoNameGestor/Containers/ContainerHeader.h"
#include "NoNameGestor/Utils/ImGuiExtension.h"
#include "NoNameGestor/Gestor/Configuration.h"
#include "NoNameGestor/Gestor/Aggregate.h"

#include "NoNameGestor/External/imgui/imgui.h"
#include "NoNameGestor/External/imgui/imgui_internal.h"
#include <filesystem>
#include "Framework/Engine/App.h"

GestorSystem::GestorSystem(int index, std::string&& name, float totalMoney, FileManager::File* file, Configuration* config, String* errorMessage) :
	VoiceNode(std::move(name), "Gestor"),
	file(file),
	config(config),
	errorMessage(errorMessage),
	inputContainer(new InputContainer("", totalMoney, &format)),
	totalContainer(new TotalContainer("", &format, config))
{
	SetFormat("%.2f", config->currency[index]);
}

GestorSystem::GestorSystem(int index, const FileManager::FileNode& gestorNode, FileManager::File* file, Configuration* config, String* errorMessage) :
	VoiceNode("", "Gestor"),
	file(file),
	config(config),
	errorMessage(errorMessage),
	inputContainer(new InputContainer("MONEY ", gestorNode.Read<float>("money"), &format)),
	totalContainer(new TotalContainer("TOTAL ", &format, config))
{
	name = gestorNode.Read<std::string>("name");
	int currency = config->currency[index] = gestorNode.Read<int>("currency");
	SetFormat("%.2f", currency);

	FileManager::FileNode containersNode = gestorNode.Access("containers");
	int size = containersNode.Length();

	for (int i = 0; i < size; ++i)
	{
		FileManager::FileNode cnode = containersNode.Access(i);
		ContainerType type = (ContainerType)cnode.Read<int>("type");
		CreateContainer(type, cnode);
	}
}

GestorSystem::~GestorSystem()
{
	delete inputContainer;
	inputContainer = nullptr;
	delete totalContainer;
	totalContainer = nullptr;
}

std::vector<VoiceNode*> GestorSystem::Children() const
{
	return std::vector<VoiceNode*>(containers.begin(), containers.end());
}

std::string GestorSystem::Describe(DetailLevel level) const
{
	std::string ret;
	switch (level)
	{
	case DetailLevel::BRIEF:
		ret = "Gestor name " + name + ". It has " + std::to_string(containers.size()) + " containers. "
			+ "Total Money Amount is " + std::to_string(inputContainer->Money()) + ". Total left money is " + std::to_string(totalContainer->Money());
		break;
	case DetailLevel::NORMAL:
		//ret = std::string("We are currently in a NNG file called ") + file.Name().Data()
		//	+ ", which has " + std::to_string(gestors.size()) + " gestors in it.";
		break;
	case DetailLevel::FULL:
	{
		//ret = std::string("We are currently in NoNameGestor app, with an open NNG file called ") + file.Name().Data()
		//	+ ". It has a total amount of " + std::to_string(gestors.size()) + " gestors in it. ";
		//static std::string numerals[4] = { "The first one", "The second one", "The third one", "The forth one" };
		//int i = 0;
		//for (GestorSystem* g : gestors)
		//	ret += numerals[i++] + " is called " + g->Name().data() + ". ";

		break;
	}
	default:
		ret = "";
		break;
	}
	return ret;
}

bool GestorSystem::Update()
{
	Aggregate set;

	inputContainer->Update(set);

	for (Container* c : containers)
		c->Update(set);

	totalContainer->Update(set);

	return true;
}

bool GestorSystem::Draw(float maxWidth, Flag& enable)
{
	ImGui::RS::Spacing();

	ImGui::PushID(id.Data());
	ImGui::RS::ClearInputText("##GestorName", &name);
	ImGui::PopID();

	ImGui::RS::Spacing();

	inputContainer->Draw();

	ImGui::RS::Spacing(2);

	int index = -1;
	ID move = ID::Empty;
	bool erase = false;

	for (int i = 0; i < containers.size(); ++i)
	{
		Container* c = containers[i];

		if (c->TypeEnum() == ContainerType::CONSTANT) enable(0, true);

		c->DrawBase(maxWidth, erase, move);

		if (index == -1 && (erase || !move.IsEmpty()))
			index = i;
	}

	if (!move.IsEmpty())
	{
		int i = 0;
		for (; i < containers.size(); ++i)
		{
			if (containers[i]->Id() == move)
				break;
		}
		//int i = containers.Find([&](const Container* c) { return c->Id() == move; });
		Debug::Assert(i != -1, "Container Drag&Drop Error: Couldn't find the container by ID.");
		MoveContainer(i, index);
	}
	else if (erase && index != -1)
	{
		containers.erase(containers.begin() + index);
	}

	Container::UpdateOpenState = index != -1;

	ImGui::RS::Spacing(2);
	ImGui::RS::Separator();

	totalContainer->Draw();

	return true;
}

void GestorSystem::DrawExport() const
{
	ImGui::PushID(id.Data());
	if (ImGui::BeginMenu(("> " + name).c_str()))
	{
		bool selected = false;
		bool empty = containers.empty();

		if (!empty)
		{
			ImGui::Text("Select the containers:");
			ImGui::RS::Spacing();
			for (const Container* c : containers)
			{
				bool s = c->DrawExport();
				if (!selected) selected = s;
			}
		}
		else
		{
			ImGui::TextDisabled("No containers yet:");
			ImGui::RS::Spacing(3);
		}

		ImGui::RS::Spacing();
		ImGui::RS::Separator();

		if (!selected || empty) ImGui::BeginDisabled();

		if (ImGui::Selectable("  Export", false, ImGuiSelectableFlags_None, { 70, 14 }))
		{
			std::vector<Container*> result;
			std::copy_if(containers.begin(), containers.end(), std::back_inserter(result), [](const Container* c) { return c->exporting; });
			Export(result);
		}
		if (!selected && !empty) ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::Text("|");
		ImGui::SameLine();
		if (ImGui::Selectable("Export All", false, ImGuiSelectableFlags_None, { 70, 14 }))
			Export(containers);
		if (empty) ImGui::EndDisabled();
		ImGui::EndMenu();
	}
	ImGui::PopID();
}

void GestorSystem::DrawCashFlow(float widthRatio, float initX) const
{
	for (Container* c : containers)
	{
		if (c->TypeEnum() != ContainerType::CONSTANT)
			continue;

		c->DrawCashFlow(widthRatio, initX);
	}
}

void GestorSystem::Export(const std::vector<Container*>& exporting) const
{
	String path = App::DataDirectory() + "Export\\";

	if (!FileManager::DirectoryExists(path) && !FileManager::DirectoryCreate(path))
	{
		*errorMessage = "Error: it was not possible to create Exports folder in: " + path;
		return;
	}

	String name = file->Name();
	name.Replace(".nng", "_Gestor_Export.txt");

	std::ofstream exp((path + name).Str(), std::ios::out | std::ios::trunc);

	if (!exp.is_open())
	{
		*errorMessage = "Error: File in " + path + name + " can't be opened. " + strerror(errno);
		return;
	}

	for (const Container* c : exporting)
		c->Export(exp);

	exp.close();
}

bool GestorSystem::Save(int index, FileManager::FileNode node) const
{
	node.Write("name", name);
	node.Write("currency", config->currency[index]);
	node.Write("money", inputContainer->Money());
	node.Write("containers", FileManager::File::Array);
	auto cnode = node.Access("containers");

	for (const Container* c : containers)
	{
		// Push an object representing a new gestor
		int index = cnode.Push(FileManager::File::Object);
		// Assure it is correctly pushed
		Debug::Assert(index != -1, "Internal Save Error: Pushing New Container returned an error.");
		// Access to that gestor and send it to be written
		c->Save(cnode.Access(index));
	}

	return true;
}

void GestorSystem::SetFormat(const char* format, int currency)
{
	this->format = format;
	this->format += " ";
	this->format += config->comboCurrency[currency];
}

Container* GestorSystem::CreateContainer(ContainerType container, std::string&& name, bool hidden, bool open, bool unified)
{
	switch (container)
	{
	case ContainerType::FILTER:   containers.push_back(new FilterContainer(std::move(name), hidden, open, unified, &format, config)); break;
	case ContainerType::LIMIT:    containers.push_back(new  LimitContainer(std::move(name), hidden, open, unified, &format, config)); break;
	case ContainerType::FUTURE:   containers.push_back(new FutureContainer(std::move(name), hidden, open, unified, &format, config)); break;
	case ContainerType::CONSTANT: containers.push_back(new  ConstContainer(std::move(name), hidden, open, unified, &format, config)); break;
	default: break;
	}

	Container* c = containers.back();
	c->Awake();

	return c;
}

Container* GestorSystem::CreateContainer(ContainerType container, const FileManager::FileNode& containerNode)
{
	switch (container)
	{
	case ContainerType::FILTER:   containers.push_back(new FilterContainer(containerNode, &format, config)); break;
	case ContainerType::LIMIT:    containers.push_back(new  LimitContainer(containerNode, &format, config)); break;
	case ContainerType::FUTURE:   containers.push_back(new FutureContainer(containerNode, &format, config)); break;
	case ContainerType::CONSTANT: containers.push_back(new  ConstContainer(containerNode, &format, config)); break;
	default: break;
	}

	Container* c = containers.back();
	c->Awake();

	return c;
}

void GestorSystem::MoveContainer(unsigned short index, unsigned short position)
{
	if (index == position) return;

	Debug::Assert(index >= 0 && index < containers.size());
	Debug::Assert(position >= 0 && position < containers.size());

	if (position > index)
		std::rotate(containers.begin() + index,
			containers.begin() + index + 1,
			containers.begin() + position + 1);
	else if (position < index)
		std::rotate(containers.begin() + position,
			containers.begin() + index,
			containers.begin() + index + 1);

	//containers.Move(index, position);
}