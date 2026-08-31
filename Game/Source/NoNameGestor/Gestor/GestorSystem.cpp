#include "GestorSystem.h"

#include "Framework/Engine/Debug.h"

#include "NoNameGestor/Containers/ContainerHeader.h"
#include "NoNameGestor/Utils/ImGuiExtension.h"
#include "NoNameGestor/Gestor/Configuration.h"
#include "NoNameGestor/Gestor/Aggregate.h"

#include "NoNameGestor/External/imgui/imgui_internal.h"
#include <filesystem>

GestorSystem::GestorSystem(int index, const char* name, float totalMoney, FileManager::File* file, Configuration* config, String* errorMessage) :
	id(ID::New()),
	name(name),
	file(file),
	config(config),
	errorMessage(errorMessage),
	inputContainer(new InputContainer("", totalMoney, &format)),
	totalContainer(new TotalContainer("", &format, config))
{
	SetFormat("%.2f", config->currency[index]);
}

GestorSystem::GestorSystem(int index, const FileManager::FileNode& gestorNode, FileManager::File* file, Configuration* config, String* errorMessage) :
	id(ID::New()),
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

bool GestorSystem::Update()
{
	Aggregate set;

	inputContainer->Update(set);

	containers.Iterate(
		[&](Container* c) { c->Update(set); }
	);

	totalContainer->Update(set);

	return true;
}

bool GestorSystem::Draw()
{
	ImGui::AddSpacing();

	ImGui::PushID(id.Data());
	ImGui::AddClearInputText("##GestorName", &name);
	ImGui::PopID();

	ImGui::AddSpacing();

	inputContainer->Draw();

	ImGui::AddSpacing(2);

	int index = -1;
	ID move = ID::Empty;
	bool erase = false;

	containers.Iterate(
		[&](Container* c, int i, int size)
		{
			bool ret = c->DrawBase(erase, move);

			if (erase || !move.IsEmpty())
				index = i;

			return ret;
		}
	);

	if (!move.IsEmpty())
	{
		int i = containers.Find([&](const Container* c) { return c->id == id; });
		Debug::Assert(i != -1, "Container Drag&Drop Error: Couldn't find the container by ID.");
		MoveContainer(i, index);
	}

	if (index != -1)
	{
		containers.Erase(index);
		Container::UpdateOpenState = true;
	}
	else Container::UpdateOpenState = false;

	ImGui::AddSpacing(2);
	ImGui::AddSeparator();

	totalContainer->Draw();

	return true;
}

void GestorSystem::DrawExport() const
{
	ImGui::PushID(id.Data());
	if (ImGui::BeginMenu(("> " + name).c_str()))
	{
		bool selected = false;
		bool empty = containers.IsEmpty();

		if (!empty)
		{
			ImGui::Text("Select the containers:");
			ImGui::AddSpacing();
			containers.Iterate(
				[&](const Container* c)
				{
					bool s = c->DrawExport();
					if (!selected) selected = s;
				}
			);
		}
		else
		{
			ImGui::TextDisabled("No containers yet:");
			ImGui::AddSpacing(3);
		}

		ImGui::AddSpacing();
		ImGui::AddSeparator();

		if (!selected || empty) ImGui::BeginDisabled();

		if (ImGui::Selectable("  Export", false, ImGuiSelectableFlags_None, { 70, 14 }))
		{
			Array<Container*> exp;
			if (containers.RetrieveAll([](const Container* c) { return c->exporting; }, exp))
				Export(exp);
		}
		if (!selected && !empty) ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::Text("|");
		ImGui::SameLine();
		if (ImGui::Selectable("Export All", false, ImGuiSelectableFlags_None, { 70, 14 }))
			Export(containers.ToArray());
		if (empty) ImGui::EndDisabled();
		ImGui::EndMenu();
	}
	ImGui::PopID();
}

void GestorSystem::Export(const Array<Container*>& exporting) const
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

	exporting.Iterate(
		[&](const Container* c) { c->Export(exp); }
	);

	exp.close();
}

bool GestorSystem::Save(int index, FileManager::FileNode node) const
{
	node.Write("name", name);
	node.Write("currency", config->currency[index]);
	node.Write("money", inputContainer->Money());
	node.Write("containers", FileManager::File::Array);
	auto cnode = node.Access("containers");

	containers.Iterate(
		[&](const Container* c)
		{
			// Push an object representing a new gestor
			int index = cnode.Push(FileManager::File::Object);
			// Assure it is correctly pushed
			Debug::Assert(index != -1, "Internal Save Error: Pushing New Container returned an error.");
			// Access to that gestor and send it to be written
			c->Save(cnode.Access(index));
		}
	);

	return true;
}

void GestorSystem::SetFormat(const char* format, int currency)
{
	this->format = format;
	this->format += " ";
	this->format += config->comboCurrency[currency];
}

Container* GestorSystem::CreateContainer(ContainerType container, const std::string& name, bool hidden, bool open, bool unified)
{
	switch (container)
	{
	case ContainerType::FILTER: containers.PushBack(new FilterContainer(name, hidden, open, unified, &format, config)); break;
	case ContainerType::LIMIT:  containers.PushBack(new  LimitContainer(name, hidden, open, unified, &format, config)); break;
	case ContainerType::FUTURE: containers.PushBack(new FutureContainer(name, hidden, open, unified, &format, config)); break;
	default: break;
	}

	return containers.Back();
}

Container* GestorSystem::CreateContainer(ContainerType container, const FileManager::FileNode& containerNode)
{
	switch (container)
	{
	case ContainerType::FILTER: containers.PushBack(new FilterContainer(containerNode, &format, config)); break;
	case ContainerType::LIMIT:  containers.PushBack(new  LimitContainer(containerNode, &format, config)); break;
	case ContainerType::FUTURE: containers.PushBack(new FutureContainer(containerNode, &format, config)); break;
	default: break;
	}

	return containers.Back();
}

StringView GestorSystem::Name() const
{
	return name.c_str();
}

void GestorSystem::MoveContainer(unsigned short index, unsigned short position)
{
	if (index == position) return;

	Debug::Assert(index >= 0 && index < containers.Size());
	Debug::Assert(position >= 0 && position < containers.Size());

	Container* c = containers[index];
	containers.Insert(c, position);
	containers.Erase(index > position ? index + 1 : index);
}