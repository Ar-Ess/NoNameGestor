#include "Page.h"
#include "Container.h"
#include "ContainerHeader.h"

Page::Page()
{
	name = "New_File";
	name += EXTENSION;
	id = new ID();
}

Page::~Page()
{
	delete id;
	name.clear();
}

void Page::Start(const char* currency)
{
	SetFormat("%.2f ", currency);
}

void Page::Update()
{
	actualMoney = money;

	containers.Iterate<float*, float*, float*, float*>
		(
			&actualMoney,
			&futureMoney,
			&arrearMoney,
			&constMoney,
			[](Container* c, float* totalMoney, float* futureMoney, float* arrearMoney, float* constMoney)
			{
				if (c->hidden) return;

				c->Update();

				switch (c->GetType())
				{
				case ContainerType::FUTURE: *futureMoney += c->GetMoney(); break;
				case ContainerType::ARREAR: *arrearMoney -= c->GetMoney(); break;
				case ContainerType::FILTER:
				case ContainerType::LIMIT:
				case ContainerType::SAVING:
				default: *totalMoney -= c->GetMoney(); break;
				}
			}
	);

	money = actualMoney + futureMoney + arrearMoney;
}

void Page::Draw()
{
	ImGui::Spacing(3);

	ImGui::Text("Total Money"); ImGui::SameLine();
	ImGui::PushItemWidth(150.0f);
	ImGui::DragFloat("##Drag", &money, 1.0f, 0.0f, 340282000000000000000000000000000000000.0f, format.c_str(), ImGuiSliderFlags_AlwaysClamp);
	ImGui::PopItemWidth();

	ImGui::Spacing(2);

	for (unsigned int i = 0; i < containers.Size(); ++i)
	{
		Container* c = containers[i];
		ImGui::PushID(c->id->Value() / (i + 1));
		bool reordered = false;

		bool hidden = c->hidden;
		if (hidden) ImGui::BeginDisabled();

		ImGui::Dummy({ 20, 0 }); ImGui::SameLine();

		ImGui::PushItemWidth(textFieldSize);
		ImGui::InputText("##LabelName", c->GetString()); ImGui::PopItemWidth(); ImGui::SameLine();

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
			if (ImGui::MenuItem("Process"))
			{
				int dif = 1;
				c->GetType() == ContainerType::FUTURE ? dif = 1 : dif = -1;
				float totalResult = money + (c->GetMoney() * dif);
				if (totalResult >= 0)
				{
					//*totalContainer->GetMoneyPtr() = totalResult;
					DeleteContainer(i);
					ImGui::EndPopup();
					ImGui::PopID();
					break;
				}
			}
			if (c->GetSize() <= 1 && ImGui::MenuItem("Unify", "", &c->unified)) c->SwapNames();
			ImGui::MenuItem("Hide", "", &c->hidden);
			ImGui::EndPopup();
		}
		ImGui::SameLine();

		if (c->loadOpen)
		{
			ImGui::SetNextItemOpen(c->open);
			c->loadOpen = false;
		}
		if (c->open = ImGui::TreeNodeEx("[]", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding))
		{
			if (ImGui::BeginDragDropSource())
			{
				intptr_t id = c->id->Value();
				ImGui::SetDragDropPayload("Container", &id, sizeof(intptr_t));
				ImGui::Text(c->GetName());
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
			c->Draw();

			ImGui::TreePop();
		}
		ImGui::PopID();

		ImGui::Spacing(0);

		if (reordered) break;
	}

	ImGui::Spacing(3);

	ImGui::Text("Balance:"); ImGui::SameLine();
	ImGui::Text(format.c_str(), money);
}

void Page::CreateContainer(ContainerType type)
{
	Container* c = nullptr;

	switch (type)
	{
	case ContainerType::NO_CONTAINER:
		return;
	case ContainerType::FILTER:
		c = new FilterContainer();
		break;
	case ContainerType::LIMIT:
		c = new LimitContainer();
		break;
	case ContainerType::FUTURE:
		c = new FutureContainer();
		break;
	case ContainerType::ARREAR:
		c = new ArrearContainer();
		break;
	default:
		break;
	}

	containers.PushBack(c);
}

void Page::SetFormat(const char* form, const char* currency)
{
	format.clear();
	format += form;
	format += currency;
}

void Page::DeleteContainer(unsigned int index)
{
	containers.Erase(index);
	SwitchLoadOpen();
}

void Page::SwitchLoadOpen()
{
	containers.Iterate([](Container* c) {c->loadOpen = true; });
}

void Page::MoveContainer(unsigned int index, unsigned int position)
{
	if (index == position) return;
	assert(index >= 0 && index < containers.Size());
	assert(position >= 0 && position < containers.Size());

	Container* r = containers[index];
	containers.Erase(index);

	containers.Insert(r, position);
}

int Page::ReturnContainerIndex(intptr_t id)
{
	int i = -1;

	containers.Iterate<intptr_t, int*>
		(
			id,
			&i,
			[](Container* c, intptr_t id, int* i)
			{
				(*i)++;
				if (c->id->Value() == id) return false;
			}
	);

	assert(i != -1); // There is not a container like "r"

	return i;
}

