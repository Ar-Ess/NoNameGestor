#pragma once

#include "Framework/Utils/Maths.h"

#include "NoNameGestor/Containers/Container.h"

class LimitContainer : public Container
{
public: // Functions

	LimitContainer(const std::string& name, bool hidden, bool open, bool unified, String* format, Configuration* config) :
		Container(name, hidden, open, unified, format, config)
	{
	}

	LimitContainer(const FileManager::FileNode& node, String* format, Configuration* config) :
		Container(node, format, config)
	{
		FileManager::FileNode lNode = node.Access("labels");
		int size = lNode.Length();

		for (int j = 0; j < size; ++j)
		{
			FileManager::FileNode node = lNode.Access(j);

			String name = node.Read<String>("name");
			float money = node.Read<float>("money");
			float limit = node.Read<float>("limit");
			NewLabel(name.Str(), money, limit);
		}
	}

	~LimitContainer() override
	{
	}

	bool Update(Aggregate& agg) override 
	{
		if (!Container::Update(agg))
			return false;

		agg.assigned += money;
		return true;
	}

	void Draw() override
	{
		if (hidden) ImGui::BeginDisabled();

		labels.Iterate(
			[&](Label& l, int i, int size)
			{
				ImGui::PushID(id.Data());
				{
					if (i == 0) { if (ImGui::Button("+")) NewLabel("New Limit"); }
					else ImGui::Dummy({ 33, 0 });

					ImGui::SameLine();

					float width = 100.0f;
					if (!unified)
					{
						if (size > 1) { if (ImGui::Button("X")) { labels.Erase(i); return false; } }
						else ImGui::Dummy({ 15, 0 });

						ImGui::SameLine();

						ImGui::PushItemWidth(config->textFieldSize);
						ImGui::InputText("##LimitName", &l.name);
						ImGui::PopItemWidth(); ImGui::SameLine();
					}
					else width += 50;

					float limit = l.limit.Value();

					ImGui::BeginGroup();
					{
						ImVec2 itemSize;
						ImGui::PushItemWidth(width);
						{
							ImGui::DragFloat("##Drag", &l.money, 1.0f, 0.0f, limit, format->Str(), ImGuiSliderFlags_AlwaysClamp);
							l.money = Maths::Clamp(l.money, 0, limit);
							itemSize = ImGui::GetItemRectSize();
							itemSize.y -= 15;
						}
						ImGui::PopItemWidth();

						float ratio = Maths::Approximately(limit, 0) ? 1 : l.money / limit;
						ImGui::ProgressBar(ratio, itemSize, "");
					}
					ImGui::EndGroup();

					ImGui::SameLine();

					ImGui::Text("/"); ImGui::SameLine();
					ImGui::Text(format->Str(), limit); ImGui::SameLine();
					if (ImGui::Button("Edit"))
						editLimit = i;
				}
				ImGui::PopID();
			}
		);

		if (hidden) ImGui::EndDisabled();

		if (editLimit == -1)
			return;

		ImGui::PushID(id.Data());
		{
			ImGui::OpenPopup("Edit Limit");
			{
				ImVec2 size = ImGui::GetWindowSize();
				ImGui::SetNextWindowSize(ImVec2(140, 100));
				ImGui::SetNextWindowPos(ImVec2(size.x / 2, size.y / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

				if (ImGui::BeginPopupModal("Edit limit", nullptr, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
				{
					static float tempLimit = 0;
					ImGui::DragFloat("##Drag1", &tempLimit, 1.0f, 0.0f, MAX_MONEY, format->Str(), ImGuiSliderFlags_AlwaysClamp);

					if (ImGui::Button("Done"))
					{
						labels[editLimit].limit = tempLimit;
						editLimit = -1;
						tempLimit = 0;
					}
				}
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}

	void Save(FileManager::FileNode node) const override
	{
		Container::Save(node);

		auto lnode = node.Access("labels");
		labels.Iterate(
			[&](const Label& l, int i)
			{
				lnode.Access(i).Write("limit", l.limit.Value());
			}
		);
	}

	float GetLabelLimit(int i) const
	{
		return labels[i].limit.Value();
	}

	void NewLabel(const std::string& name = "New Limit", float money = 0.0f, float limit = 0.0f)
	{
		Container::NewLabel(Label(name, money, limit));
	}

	const char* TypeName() const override
	{
		return "LIMIT ";
	}

	ContainerType Type() const override
	{
		return ContainerType::LIMIT;
	}

private:

	int editLimit = -1;
};
