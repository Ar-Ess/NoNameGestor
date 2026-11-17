#include "LimitContainer.h"

#include "NoNameGestor/Containers/Label.h"
#include "NoNameGestor/Gestor/Aggregate.h"
#include "NoNameGestor/Gestor/Configuration.h"
#include "NoNameGestor/Utils/ImGuiExtension.h"

LimitContainer::LimitLabel::LimitLabel(std::string&& name, float money, float limit) :
	Label(std::move(name), money), limit(limit)
{
}

Flag LimitContainer::LimitLabel::Draw(bool unified, bool first, bool single, int textFieldSize, String* format)
{
	Flag ret = Flag::AllFalse;
	ImGui::PushID(id.Data());
	{
		if (first) { if (ImGui::Button("+")) ret.Set(0, true); }
		else ImGui::Dummy({ 33, 0 });

		ImGui::SameLine();

		float width = 100.0f;
		if (!unified)
		{
			if (!single)
			{
				if (ImGui::Button("X"))
					ret.Set(1, true);
			}
			else ImGui::Dummy({ 15, 0 });

			ImGui::SameLine();

			ImGui::PushItemWidth(textFieldSize);
			ImGui::InputText("##LimitName", &name);
			ImGui::PopItemWidth(); ImGui::SameLine();
		}
		else width += 50;

		ImGui::BeginGroup();
		{
			ImVec2 itemSize;
			ImGui::PushItemWidth(width);
			{
				ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, limit, format->Str(), ImGuiSliderFlags_AlwaysClamp);
				money = Maths::Clamp(money, 0, limit);
				itemSize = ImGui::GetItemRectSize();
				itemSize.y -= 15;
			}
			ImGui::PopItemWidth();

			float ratio = Maths::Approximately(limit, 0) ? 1 : money / limit;
			ImGui::ProgressBar(ratio, itemSize, "");
		}
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::Text("/"); ImGui::SameLine();
		ImGui::Text(format->Str(), limit); ImGui::SameLine();
		if (ImGui::Button("Edit"))
		{
			ImGui::OpenPopup("Edit Limit Popup");
			ret.Set(2, true);
		}
	}
	ImGui::PopID();

	return ret;
}

LimitContainer::LimitContainer(std::string&& name, bool hidden, bool open, bool unified, String* format, Configuration* config) :
	Container(std::move(name), hidden, open, unified, format, config)
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

	int erase = -1;

	for (int i = 0; i < labels.size(); ++i)
	{
		LimitLabel* l = (LimitLabel*)labels[i];
		Flag f = l->Draw(unified, i == 0, labels.size() <= 1, config->textFieldSize, format);

		if (f.Get(0)) NewLabel("New Limit");
		else if (f.Get(1)) erase = i;
		else if (f.Get(2)) select = l;
	}

	if (erase != -1) labels.erase(labels.begin() + erase);

	if (hidden) ImGui::EndDisabled();

	if (select == nullptr)
		return;

	ImGui::RS::CenterNextWindow();
	ImGui::SetNextWindowSize(ImVec2(140, 100));

	ImGui::PushID(select->Id().Data());

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
	for (int i = 0; i < labels.size(); ++i)
		lnode.Access(i).Write("limit", ((LimitLabel*)labels[i])->limit);
}

void LimitContainer::NewLabel(std::string&& name, float money, float limit)
{
	Container::NewLabel(new LimitLabel(std::move(name), money, limit));
}