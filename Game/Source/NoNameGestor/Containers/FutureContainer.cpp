#include "FutureContainer.h"

#include "NoNameGestor/Gestor/Aggregate.h"
#include "NoNameGestor/Gestor/Configuration.h"

#include "NoNameGestor/External/imgui/imgui.h"
#include "NoNameGestor/External/imgui/imgui_stdlib.h"

FutureContainer::FutureContainer(const std::string& name, bool hidden, bool open, bool unified, String* format, Configuration* config) :
	Container(name, hidden, open, unified, format, config)
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

	labels.Iterate(
		[&](Label* l, int i, int size)
		{
			ImGui::PushID(l->id.Data());
			{
				if (i == 0) { if (ImGui::Button("+")) NewLabel("New Future"); }
				else ImGui::Dummy({ 33, 0 });

				ImGui::SameLine();

				float width = 100.0f;
				if (!unified)
				{
					if (size > 1) { if (ImGui::Button("X")) { labels.Erase(i); return false; } }
					else ImGui::Dummy({ 15, 0 });

					ImGui::SameLine();

					ImGui::PushItemWidth(config->textFieldSize);
					{
						ImGui::InputText("##FutureName", &l->name);
					}
					ImGui::PopItemWidth(); ImGui::SameLine();
				}
				else width += 50;

				ImGui::PushItemWidth(width);
				{
					ImGui::DragFloat("##Drag", &l->money, 1.0f, 0.0f, MAX_MONEY, format->Str(), ImGuiSliderFlags_AlwaysClamp);
				}
				ImGui::PopItemWidth();
			}
			ImGui::PopID();
		}
	);

	if (hidden) ImGui::EndDisabled();
}

void FutureContainer::NewLabel(const std::string& name, float money)
{
	Container::NewLabel(new Label(name, money));
}