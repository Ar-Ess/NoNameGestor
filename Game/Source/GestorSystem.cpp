#include "GestorSystem.h"
#include "ContainerHeader.h"
#include "FileManager.h"

GestorSystem::GestorSystem(bool* showFutureUnasigned, bool* allowFutureCovering, bool* showArrearUnasigned, bool* allowArrearsFill, bool* showConstantTotal, bool* showContainerType)
{
	totalContainer = new TotalContainer("Total Money", &format);
	unasignedContainer = new UnasignedContainer("Unasigned Money", &format, showFutureUnasigned, allowFutureCovering, showArrearUnasigned, allowArrearsFill, showConstantTotal);
	this->showContainerType = showContainerType;
}

GestorSystem::~GestorSystem()
{
	DeleteAllContainer();
	RELEASE(totalContainer);
	RELEASE(unasignedContainer);
}

bool GestorSystem::Start()
{
	return true;
}

bool GestorSystem::Update()
{
	totalContainer->Update();
	float totalMoney = totalContainer->GetMoney();
	float futureMoney = 0;
	float arrearMoney = 0;
	float constMoney = 0;

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

	unasignedContainer->SetMoney(totalMoney + futureMoney + arrearMoney, totalMoney, futureMoney, arrearMoney, constMoney);
	unasignedContainer->Update();

	return true;
}

bool GestorSystem::Draw()
{
	AddSpacing(3);

	totalContainer->Draw();

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
		if (r->open = ImGui::TreeNodeEx("[]", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding))
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

			ImGui::Dummy({ 15, 0 }); ImGui::SameLine();
			r->Draw();

			ImGui::TreePop();
		}
		ImGui::PopID();

		AddSpacing(0);

		if (reordered) break;
	}

	AddSpacing(0);

	unasignedContainer->Draw();

	return true;
}

bool GestorSystem::Save(FileManager* file, const char* path)
{
	file->EditFile(path).
		Write("containers").Number(totalContainer->GetMoney()).
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
			break;
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

		// X aspects
		file->ViewFile(path, positionToRead).
			Read("name").AsString(name).
			Read("type").AsInt(type).
			Read("money").AsFloat(money).
			Read("hide").AsBool(hidden).
			Read("open").AsBool(open).
			Read("unfd").AsBool(unified);

		jumplinesCount += 6; // Adding X aspects to jumplines

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
		default: break;
		}

	}

	jumplines += jumplinesCount;

	totalContainer->SetMoney(total);

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
	float* moneyPtr = totalContainer->GetMoneyPtr();

	switch (container)
	{
	case ContainerType::FILTER: containers.emplace_back((Container*)(new FilterContainer(name, hidden, open, unified, moneyPtr, &format))); break;
	case ContainerType::LIMIT: containers.emplace_back((Container*)(new  LimitContainer(name, hidden, open, unified, moneyPtr, &format))); break;
	case ContainerType::FUTURE: containers.emplace_back((Container*)(new FutureContainer(name, hidden, open, unified, moneyPtr, &format))); break;
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

