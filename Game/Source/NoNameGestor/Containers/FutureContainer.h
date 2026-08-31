#pragma once

#include "Container.h"

class FutureContainer : public Container
{
public: // Functions

	FutureContainer(const std::string& name, bool hidden, bool open, bool unified, String* format, Configuration* config) :
		Container(name, hidden, open, unified, format, config)
	{ }

	FutureContainer(const FileManager::FileNode& node, String* format, Configuration* config) :
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

	~FutureContainer() override
	{
	}

	bool Update(Aggregate& agg) override
	{
		if (!Container::Update(agg))
			return false;

		agg.future += money;
		return true;
	}

	void Draw() override
	{
		if (hidden) ImGui::BeginDisabled();
		
		labels.Iterate(
			[&](Label& l, int i, int size)
			{
				ImGui::PushID(l.id.Data());
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
							ImGui::InputText("##FutureName", &l.name);
						}
						ImGui::PopItemWidth(); ImGui::SameLine();
					}
					else width += 50;

					ImGui::PushItemWidth(width);
					{
						ImGui::DragFloat("##Drag", &l.money, 1.0f, 0.0f, MAX_MONEY, format->Str(), ImGuiSliderFlags_AlwaysClamp);
					}
					ImGui::PopItemWidth();
				}
				ImGui::PopID();
			}
		);

		if (hidden) ImGui::EndDisabled();
	}

	void NewLabel(const std::string& name = "New Future", float money = 0.0f)
	{
		Container::NewLabel(Label(name, money));
	}

	const char* TypeName() const override
	{
		return "FUTURE";
	}

	ContainerType Type() const override
	{
		return ContainerType::FUTURE;
	}

private:

};
