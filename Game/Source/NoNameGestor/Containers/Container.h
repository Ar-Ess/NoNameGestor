#pragma once

#include "Framework/Data/Vector.h"

#include "NoNameGestor/Containers/Label.h"
#include "NoNameGestor/Containers/ContainerEnum.h"
#include "NoNameGestor/Gestor/Currency.h"
#include "NoNameGestor/Gestor/Configuration.h"
#include "NoNameGestor/Gestor/Aggregate.h"
#include "NoNameGestor/Utils/FileManager.h"
#include "NoNameGestor/Utils/ImGuiExtension.h"

#include "NoNameGestor/External/imgui/imgui.h"
#include "NoNameGestor/External/imgui/imgui_stdlib.h"
#include "NoNameGestor/External/imgui/imgui_internal.h"

constexpr auto MAX_MONEY = 340282000000000000000000000000000000000.0f;

class Container
{
public: // Functions

	virtual ~Container();

	virtual bool Update(Aggregate& agg);

	bool DrawBase(bool& erase, ID& move);

	virtual void Draw() {}

	virtual bool DrawExport() const
	{
		ImGui::Text(" - ");
		ImGui::SameLine();
		ImGui::PushID(id.Data());
		ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);
		ImGui::MenuItem(CurrentName()->c_str(), "", &exporting);
		ImGui::PopItemFlag();
		ImGui::PopID();
		return exporting;
	}

	virtual void Save(FileManager::FileNode node) const;

	std::string* CurrentName();

	const std::string* CurrentName() const;

	virtual const char* TypeName() const;

	virtual ContainerType Type() const;

	float Money() const;

	unsigned int Size() const;

	void Export(std::ofstream& exp) const;

protected: // Functions

	Container(const std::string& name, bool hidden, bool open, bool unified, String* format, Configuration* config);

	Container(const FileManager::FileNode& node, String* format, Configuration* config);

	void NewLabel(Label&& label);

public: // Variables

	static bool UpdateOpenState;
	
	bool hidden = false;
	bool unified = true;
	bool open = false;
	bool exporting = false;
	ID id;

protected: // Variables

	Vector<Label> labels;
	float money = 0.0f;
	std::string name;
	String* format = nullptr;
	Configuration* config = nullptr;

};