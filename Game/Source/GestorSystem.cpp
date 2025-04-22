#include "GestorSystem.h"
#include "ContainerHeader.h"
#include "FileManager.h"
#include <imgui/imgui_internal.h>

GestorSystem::GestorSystem(const char* name, bool* showFutureUnasigned, bool* showContainerType, std::string* openFileName, std::string* openFilePath, ImFont* bigFont)
{
	inputContainer = new InputContainer("MONEY ", &format);
	totalContainer = new TotalContainer("TOTAL ", &format, showFutureUnasigned);
	this->showContainerType = showContainerType;
	this->id = reinterpret_cast<int>(this);
	this->name = name;
	this->bigFont = bigFont;
	this->openFileName = openFileName;
	this->openFilePath = openFilePath;
}

GestorSystem::~GestorSystem()
{
	DeleteAllContainer();
	RELEASE(inputContainer);
	RELEASE(totalContainer);
}

bool GestorSystem::Start()
{
	return true;
}

bool GestorSystem::Update()
{
	inputContainer->Update();
	float totalMoney = inputContainer->GetMoney();
	float futureMoney = 0;

	for (Container* r : containers)
	{
		if (r->hidden) continue;

		r->Update();

		switch (r->GetType())
		{
		case ContainerType::FUTURE: futureMoney += r->GetMoney(); break;
		case ContainerType::FILTER:
		case ContainerType::LIMIT:
		default: totalMoney -= r->GetMoney(); break;
		}
	}

	totalContainer->SetMoney(totalMoney, futureMoney);
	totalContainer->Update();

	return true;
}

bool GestorSystem::Draw()
{
	AddSpacing(1);

	ImGui::PushID(id);
	AddClearInputText("##GestorName", &name);
	ImGui::PopID();

	AddSpacing(1);

	inputContainer->Draw();

	AddSpacing(2);

	size_t size = containers.size();
	for (suint i = 0; i < size; ++i)
	{
		Container* r = containers[i];
		ImGui::PushID(r->GetId() / ((i * size) + size * size));
		bool reordered = false;

		bool hidden = r->hidden;
		if (hidden) ImGui::BeginDisabled();

		ImGui::Dummy({ 20, 0 }); ImGui::SameLine();

		if (*showContainerType)
		{
			ImGui::Text(r->GetTypeString());
			ImGui::SameLine();
		}
		ImGui::PushItemWidth(textFieldSize);
		ImGui::InputText("##LabelName", r->GetString()); ImGui::PopItemWidth(); ImGui::SameLine();

		if (hidden) ImGui::EndDisabled();

		if (ImGui::Button(":"))
			ImGui::OpenPopup("Options Popup");
		if (ImGui::BeginPopup("Options Popup"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				DeleteContainer(i);
				ImGui::EndPopup();
				ImGui::PopID();
				break;
			}
			if (r->GetSize() <= 1 && ImGui::MenuItem("Unify", "", &r->unified)) r->SwapNames();
			ImGui::MenuItem("Hide", "", &r->hidden);
			ImGui::EndPopup();
		}
		ImGui::SameLine();

		if (r->loadOpen)
		{
			ImGui::SetNextItemOpen(r->open);
			r->loadOpen = false;
		}

		/*
		if (r->open = ImGui::TreeNodeEx("[]", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			if (ImGui::BeginDragDropSource())
			{
				intptr_t id = r->GetId();
				ImGui::SetDragDropPayload("Container", &id, sizeof(intptr_t));
				ImGui::Text(r->GetName());
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Container");
				if (payload)
				{
					MoveContainer(ReturnContainerIndex(*((intptr_t*)payload->Data)), i);
					reordered = true;
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::Dummy({ 10, 0 }); ImGui::SameLine();
			r->Draw();

			ImGui::TreePop();
		}
		*/

		{
			bool nodeOpen = r->open;

			// Dibuixa la fletxeta com a botó
			if (ImGui::ArrowButton("##arrow", nodeOpen ? ImGuiDir_Down : ImGuiDir_Right))
			{
				nodeOpen = !nodeOpen;
				r->open = nodeOpen;
			}

			ImGui::SameLine();
			
			ImVec2 textSize = ImGui::CalcTextSize("[]");
			ImVec2 textPos = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("##hoverZone", textSize);
			bool hoveringText = ImGui::IsItemHovered();

			ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), hoveringText ? "[ ]" : "[]");

			// Drag & Drop source
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				intptr_t id = r->GetId();
				ImGui::SetDragDropPayload("Container", &id, sizeof(intptr_t));
				ImGui::Text(r->GetName());
				ImGui::EndDragDropSource();
			}

			// Drag & Drop target
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Container");
				if (payload)
				{
					MoveContainer(ReturnContainerIndex(*((intptr_t*)payload->Data)), i);
					reordered = true;
				}
				ImGui::EndDragDropTarget();
			}

			if (nodeOpen)
			{
				ImGui::Indent();
				ImGui::Dummy({ 10, 0 }); ImGui::SameLine();
				r->Draw();
				ImGui::Unindent();
			}
		}

		ImGui::PopID();
		AddSpacing(0);
		if (reordered) break;
	}

	AddSpacing(2);
	AddSeparator();

	totalContainer->Draw();

	return true;
}

void GestorSystem::DrawExport()
{
	ImGui::PushID(id);
	if (ImGui::BeginMenu(name.c_str()))
	{
		bool empty = containers.empty();
		bool selected = false;

		if (!empty)
		{
			ImGui::Text("Select the containers:");
			AddSpacing(1);
			for (Container* c : containers)
			{
				ImGui::Text(" - ");
				ImGui::SameLine();
				ImGui::PushID(c->GetId());
				ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
				ImGui::MenuItem(c->GetName(), "", &c->exporting);
				ImGui::PopItemFlag();
				ImGui::PopID();
				if (!selected && c->exporting) selected = true;
			}
		}
		else
		{
			ImGui::TextDisabled("No containers yet:");
			AddSpacing(3);
		}
		AddSpacing(1);
		AddSeparator();
		if (!selected || empty) ImGui::BeginDisabled();

		if (ImGui::Selectable("  Export", false, ImGuiSelectableFlags_None, { 70, 14 }))
		{
			std::vector<Container*> toExport;
			for (Container* c : containers)
			{
				if (!c->exporting) continue;
				toExport.emplace_back(c);
			}
			ExportGestor(&toExport);
		}
		if (!selected && !empty) ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::Text("|");
		ImGui::SameLine();
		if (ImGui::Selectable("Export All", false, ImGuiSelectableFlags_None, { 70, 14 }))
			ExportGestor(&containers);
		if (empty) ImGui::EndDisabled();
		ImGui::EndMenu();
	}
	ImGui::PopID();
}

void GestorSystem::ExportGestor(std::vector<Container*>* exporting)
{
	std::fstream file;
	std::string filePath = *openFilePath;
	if (filePath.empty()) filePath = "Exports\\";
	filePath += *openFileName;
	filePath.erase(filePath.end() - 4, filePath.end());
	filePath += "_Gestor.txt";

	file.open(filePath, std::ios::out);

	assert(file.is_open()); // File is not open

	std::vector<Container*>::const_iterator it = exporting->begin();
	for (it; it != exporting->end(); ++it)
	{
		file << (*it)->GetName() << ":";
		const char* currency = format.substr(5, format.size() - 5).c_str();

		if ((*it)->unified)
		{
			file << " " << (*it)->GetMoney() << " " << currency << std::endl << std::endl;
		}
		else
		{
			file << std::endl;
			unsigned int size = (*it)->GetSize();
			for (unsigned int i = 0; i < size; ++i)
			{
				file << " - " << (*it)->GetLabelName(i) << ": " << (*it)->GetLabelMoney(i) << " " << currency << std::endl << std::endl;
			}
		}
	}

	file.close();
}

bool GestorSystem::Save(FileManager* file, const char* path)
{
	file->EditFile(path).
		Write("name").String(name.c_str()).
		Write("containers").Number(inputContainer->GetMoney()).
		Write("size").Number((int)containers.size());

	for (Container* r : containers)
	{
		file->EditFile(path).
			Write("name").String(r->GetName()).
			Write("type").Number((int)r->GetType()).
			Write("money").Number(r->GetMoney()).
			Write("hide").Bool(r->hidden).
			Write("open").Bool(r->open).
			Write("unfd").Bool(r->unified);

		switch (r->GetType())
		{
		case ContainerType::LIMIT:
		{
			LimitContainer* lPR = (LimitContainer*)r;
			int size = lPR->GetSize();
			file->EditFile(path).
				Write("size").Number(size);

			for (int i = 0; i < size; ++i)
			{
				file->EditFile(path)
					.Write("name").String(lPR->GetLabelName(i))
					.Write("limit").Number(lPR->GetLabelLimit(i))
					.Write("money").Number(lPR->GetLabelMoney(i));
			}
			break;
		}
		case ContainerType::FILTER:
		case ContainerType::FUTURE:
		default:
		{
			Container* c = r;
			int size = c->GetSize();
			file->EditFile(path).
				Write("size").Number(size);

			for (unsigned int i = 0; i < size; ++i)
			{
				file->EditFile(path)
					.Write("name").String(c->GetLabelName(i))
					.Write("money").Number(c->GetLabelMoney(i));
			}
			break;
		}
		}
	}

	return true;
}

bool GestorSystem::Load(FileManager* file, const char* path, int& jumplines)
{
	DeleteAllContainer();

	float total = 0;
	int size = 0;

	file->ViewFile(path, jumplines).
		Read("containers").AsFloat(total).
		Read("size").AsInt(size);

	jumplines += 2; // Because of the two new aspects on top

	int jumplinesCount = 0;
	const int containerPropertiesAmount = 6; // Update if more properties added to a container \/
	int added = 0;

	for (unsigned int i = 0; i < size; ++i)
	{
		int positionToRead = (i * containerPropertiesAmount) + jumplines + added;
		int type = -1;
		float money = 0;
		bool hidden = false, open = false, unified = true;
		std::string name;

		// Container Properties
		file->ViewFile(path, positionToRead).
			Read("name").AsString(name).
			Read("type").AsInt(type).
			Read("money").AsFloat(money).
			Read("hide").AsBool(hidden).
			Read("open").AsBool(open).
			Read("unfd").AsBool(unified);

		jumplinesCount += containerPropertiesAmount; // Adding Container Properties to jumplines

		switch ((ContainerType)type)
		{
		case ContainerType::LIMIT:
		{
			LimitContainer* lC = (LimitContainer*)CreateContainer((ContainerType)type, name.c_str(), hidden, open, unified);
			lC->ClearLabels();
			lC->loadOpen = true;

			int lSize = 0;
			int futurePositionToRead = positionToRead + containerPropertiesAmount;

			file->ViewFile(path, futurePositionToRead).
				Read("size").AsInt(lSize);

			jumplinesCount++; // Adding for this top properti "size", update if there is more
			added++;

			for (suint i = 0; i < lSize; ++i)
			{
				std::string lName;
				float lMoney, lLimit; //     Change depending on "this properties" \/
				file->ViewFile(path, (futurePositionToRead + 1) + (i * 3)).
					Read("name").AsString(lName). // this 3 are the "this properties"
					Read("limit").AsFloat(lLimit). // Also this one
					Read("money").AsFloat(lMoney); // Also this one

				lC->NewLabel(lName.c_str(), lMoney, lLimit);

				added += 3; // Change depending on how many "this properties"
				jumplinesCount += 3;

			}
			break;
		}
		case ContainerType::FILTER:
		case ContainerType::FUTURE:
		default:
		{
			Container* c = CreateContainer((ContainerType)type, name.c_str(), hidden, open, unified);
			c->ClearLabels();
			c->loadOpen = true;

			int fSize = 0;
			int futurePositionToRead = positionToRead + containerPropertiesAmount;

			file->ViewFile(path, futurePositionToRead).
				Read("size").AsInt(fSize);

			jumplinesCount++; // Adding for this top properti "size", update if there is more
			added++;

			for (suint i = 0; i < fSize; ++i)
			{
				std::string fName;
				float fMoney;
				file->ViewFile(path, (futurePositionToRead + 1) + (i * 2)).
					Read("name").AsString(fName). // Variables on top
					Read("money").AsFloat(fMoney);

				c->NewLabel(fName.c_str(), fMoney);

				added += 2; // Change depending on how many variables on top
				jumplinesCount += 2; // Adding for name & money property
			}

			break;
		}
		}

	}

	jumplines += jumplinesCount;

	inputContainer->SetMoney(total);

	return true;
}

void GestorSystem::SetFormat(const char* format, const char* currency)
{
	this->format.clear();
	this->format.shrink_to_fit();
	this->format += format;
	this->format += currency;
}

Container* GestorSystem::CreateContainer(ContainerType container, const char* name, bool hidden, bool open, bool unified)
{
	switch (container)
	{
	case ContainerType::FILTER: containers.emplace_back((Container*)(new FilterContainer(name, hidden, open, unified, &format))); break;
	case ContainerType::LIMIT:  containers.emplace_back((Container*)(new  LimitContainer(name, hidden, open, unified, &format))); break;
	case ContainerType::FUTURE: containers.emplace_back((Container*)(new FutureContainer(name, hidden, open, unified, &format))); break;
	default: break;
	}

	return containers.back();
}

void GestorSystem::SwitchLoadOpen()
{
	for (Container* c : containers) c->loadOpen = true;
}

void GestorSystem::DeleteAllContainer()
{
	for (Container* r : containers) 
		RELEASE(r);
	containers.clear();
}

void GestorSystem::AddSpacing(unsigned int spaces)
{
	short int plus = 0;
	if (spaces == 0) plus = 1;
	for (unsigned int i = 0; i < spaces * 2 + plus; i++) ImGui::Spacing();
}

void GestorSystem::AddSeparator(unsigned int separator)
{
	if (separator == 0) return;
	for (unsigned int i = 0; i < separator; i++) ImGui::Separator();
}

void GestorSystem::AddHelper(const char* desc, const char* title)
{
	ImGui::TextDisabled(title);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void GestorSystem::DeleteContainer(unsigned short index)
{
	assert(index >= 0 && index < containers.size());

	Container* r = containers[index];
	containers.erase(containers.begin() + index);
	RELEASE(r);
	SwitchLoadOpen();
}

void GestorSystem::MoveContainer(unsigned short index, unsigned short position)
{
	if (index == position) return;
	suint size = containers.size();
	assert(index >= 0 && index < size);
	assert(position >= 0 && position < size);

	Container* r = containers[index];
	containers.erase(containers.begin() + index);

	containers.insert(containers.begin() + position, r);
}

int GestorSystem::ReturnContainerIndex(intptr_t id)
{
	size_t size = containers.size();
	int i = 0;
	for (std::vector<Container*>::const_iterator it = containers.begin(); it != containers.end(); ++it)
	{
		Container* rTarget = (*it);
		if (id == rTarget->GetId()) return i;
		++i;
	}

	i = -1;
	assert(i != -1); // There is not a container like "r"

	return i;
}

void GestorSystem::AddClearInputText(const char* name, std::string* buffer)
{
	// Guarda l'estat de l'estil actual
	ImGui::PushFont(bigFont);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);  // Sense vora
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));  // Opcional: ajusta padding

	// Colors transparents
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));          // Fons transparent
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));   // Quan el cursor està a sobre
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));    // Quan està actiu (editant)

	ImGui::InputText(name, buffer);

	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	float maxTextX = ImGui::CalcTextSize((*buffer).c_str()).x + min.x + 10;
	float y = max.y + 1;
	ImU32 color = ImGui::IsItemActive() ? IM_COL32(80, 140, 255, 255) : IM_COL32(160, 160, 160, 100);
	ImGui::GetWindowDrawList()->AddLine(ImVec2(min.x, y), ImVec2(maxTextX, y), color, 1.5f);

	// Torna a l'estil normal
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);
	ImGui::PopFont();
}

