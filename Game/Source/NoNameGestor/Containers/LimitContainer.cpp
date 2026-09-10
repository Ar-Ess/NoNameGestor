#include "LimitContainer.h"

#include "NoNameGestor/Containers/Label.h"
#include "NoNameGestor/Gestor/Aggregate.h"
#include "NoNameGestor/Gestor/Configuration.h"
#include "NoNameGestor/Utils/ImGuiExtension.h"

#include "NoNameGestor/External/imgui/imgui.h"
#include "NoNameGestor/External/imgui/imgui_stdlib.h"

LimitContainer::LimitLabel::LimitLabel(const std::string& name, float money, float limit) :
	Label(name, money), limit(limit)
{
}

LimitContainer::LimitContainer(const std::string& name, bool hidden, bool open, bool unified, String* format, Configuration* config) :
	Container(name, hidden, open, unified, format, config)
{
}

LimitContainer::LimitContainer(const FileManager::FileNode& node, String* format, Configuration* config) :
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

bool LimitContainer::Update(Aggregate& agg)
{
	if (!Container::Update(agg))
		return false;

	agg.assigned += money;
	return true;
}

void LimitContainer::Draw()
{
	if (hidden) ImGui::BeginDisabled();

	labels.Iterate(
		[&](Label* label, int i, int size)
		{
			LimitLabel* l = (LimitLabel*)label;
			ImGui::PushID(l->id.Data());
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
					ImGui::InputText("##LimitName", &l->name);
					ImGui::PopItemWidth(); ImGui::SameLine();
				}
				else width += 50;

				ImGui::BeginGroup();
				{
					ImVec2 itemSize;
					ImGui::PushItemWidth(width);
					{
						ImGui::DragFloat("##Drag", &l->money, 1.0f, 0.0f, l->limit, format->Str(), ImGuiSliderFlags_AlwaysClamp);
						l->money = Maths::Clamp(l->money, 0, l->limit);
						itemSize = ImGui::GetItemRectSize();
						itemSize.y -= 15;
					}
					ImGui::PopItemWidth();

					float ratio = Maths::Approximately(l->limit, 0) ? 1 : l->money / l->limit;
					ImGui::ProgressBar(ratio, itemSize, "");
				}
				ImGui::EndGroup();

				ImGui::SameLine();

				ImGui::Text("/"); ImGui::SameLine();
				ImGui::Text(format->Str(), l->limit); ImGui::SameLine();
				if (ImGui::Button("Edit"))
				{
					ImGui::OpenPopup("Edit Limit Popup");
					select = l;
				}
			}
			ImGui::PopID();
		}
	);

	if (hidden) ImGui::EndDisabled();

	if (select == nullptr)
		return;

	ImGui::RS::CenterNextWindow();
	ImGui::SetNextWindowSize(ImVec2(140, 100));

	ImGui::PushID(select->id.Data());

	if (ImGui::BeginPopupModal("Edit Limit Popup", nullptr, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::PushID(id.Data());
		static float tempLimit = 0;
		ImGui::DragFloat("##Drag1", &tempLimit, 1.0f, 0.0f, MAX_MONEY, format->Str(), ImGuiSliderFlags_AlwaysClamp);

		if (ImGui::Button("Done"))
		{
			select->limit = tempLimit;
			select = nullptr;
			tempLimit = 0;
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopID();
		ImGui::EndPopup();
	}

	ImGui::PopID();

}

void LimitContainer::Save(FileManager::FileNode node) const
{
	Container::Save(node);

	auto lnode = node.Access("labels");
	labels.Iterate(
		[&](Label* l, int i)
		{
			lnode.Access(i).Write("limit", ((LimitLabel*)l)->limit);
		}
	);
}

void LimitContainer::NewLabel(const std::string& name, float money, float limit)
{
	Container::NewLabel(new LimitLabel(name, money, limit));
}