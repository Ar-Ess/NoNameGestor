#include "FutureContainer.h"

#include "NoNameGestor/Gestor/Aggregate.h"
#include "NoNameGestor/Gestor/Configuration.h"

#include "NoNameGestor/External/imgui/imgui.h"
#include "NoNameGestor/External/imgui/imgui_stdlib.h"

FutureContainer::FutureContainer(std::string&& name, bool hidden, bool open, bool unified, String* format, Configuration* config) :
	Container(std::move(name), hidden, open, unified, format, config)
{
}

FutureContainer::FutureContainer(const FileManager::FileNode& node, String* format, Configuration* config) :
	Container(node, format, config)
{
	FileManager::FileNode lNode = node.Access("labels");
	int size = lNode.Length();

	for (int j = 0; j < size; ++j)
	{
		FileManager::FileNode node = lNode.Access(j);

		String name = node.Read<String>("name");
		float money = node.Read<float>("money");
		NewLabel(name.Str(), money);
	}
}

bool FutureContainer::Update(Aggregate& agg)
{
	if (!Container::Update(agg))
		return false;

	agg.future += money;
	return true;
}

void FutureContainer::Draw()
{
	if (hidden) ImGui::BeginDisabled();

	int erase = -1;

	for (int i = 0; i < labels.size(); ++i)
	{
		Label* l = labels[i];
		Flag f = l->Draw(unified, i == 0, labels.size() <= 1, config->textFieldSize, format);

		if (f.Get(0)) NewLabel("New Future");
		else if (f.Get(1)) erase = i;
	}

	if (erase != -1) labels.erase(labels.begin() + erase);

	if (hidden) ImGui::EndDisabled();
}

void FutureContainer::NewLabel(std::string&& name, float money)
{
	Container::NewLabel(new Label(std::move(name), money));
}