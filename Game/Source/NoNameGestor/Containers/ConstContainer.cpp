#include "ConstContainer.h"

#include "Framework/Engine/App.h"
#include "Framework/Time/TimeSpan.h"

#include "NoNameGestor/Containers/Label.h"
#include "NoNameGestor/Gestor/Aggregate.h"
#include "NoNameGestor/Gestor/Configuration.h"
#include "NoNameGestor/Utils/ImGuiExtension.h"

#include "NoNameGestor/External/imgui/imgui.h"
#include "NoNameGestor/External/imgui/imgui_stdlib.h"

#include <cmath>

ConstContainer::ConstLabel::ConstLabel(const std::string& name, float money, float perMonth, const DateTime& expectedRunoutDate, int offsetMonths) :
	Label(name, money), perMonth(perMonth), expectedRunoutDate(expectedRunoutDate), offsetMonths(offsetMonths)
{
	availableMoney = 0;
	finalPlannedMonths = 0;
	maxPlannedMonths = 0;
	//TODO: Framework: Make TimeSpan accept uint64_t as a value
	//monthCount = TimeSpan<>::From::Time(0, 0, 0, 0, 0, m, y);
}

bool ConstContainer::ConstLabel::CanSetExpectedRunout() const
{
	return !Maths::Approximately(perMonth, 0) && !Maths::Approximately(money, 0);
}

bool ConstContainer::ConstLabel::HasExpectedRunout() const
{
	return expectedRunoutDate != DateTime::BaseEpoch;
}

void ConstContainer::ConstLabel::CalculateBaseMoneyState()
{
	availableMoney = 0.0f;
	maxPlannedMonths = 0;

	if (Maths::Approximately(money, 0) || Maths::Approximately(perMonth, 0))
		return;

	if (money < perMonth)
	{
		availableMoney = money;
		return;
	}

	availableMoney = std::fmod(money, perMonth);
	maxPlannedMonths = Maths::Floor(money / perMonth);

	if (Maths::Approximately(availableMoney, 0))
	{
		availableMoney = perMonth;
		maxPlannedMonths--;
	}
}

ConstContainer::ConstContainer(const std::string& name, bool hidden, bool open, bool unified, String* format, Configuration* config) :
	Container(name, hidden, open, unified, format, config)
{
}

ConstContainer::ConstContainer(const FileManager::FileNode& node, String* format, Configuration* config) :
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

void ConstContainer::Awake()
{
	currentTime = App::OpenAppTime;
}

bool ConstContainer::Update(Aggregate& agg)
{
	if (!Container::Update(agg))
		return false;

	labels.Iterate(
		[&](Label* label)
		{
			ConstLabel* l = (ConstLabel*)label;

			l->CalculateBaseMoneyState();

			if (l->HasExpectedRunout())
			{
				// Calculate months until runout
				const int runoutMonths = currentTime.MonthsUntil(l->expectedRunoutDate);

				// If expected runout has been reached
				if (runoutMonths <= 0)
				{
					l->expectedRunoutDate = DateTime::BaseEpoch;
					l->expectedRunoutMoney = 0.0f;

					// Continue using normal mode.
				}
				// Money was increased after setting the expected runout.
				else if (l->money > l->expectedRunoutMoney)
				{
					SetWarningMessage(label, "Warning: Money increased after setting the Expected Runout. Expected Runout has been disabled.");

					l->expectedRunoutDate = DateTime::BaseEpoch;
					l->expectedRunoutMoney = 0.0f;

					// Continue using normal mode.
				}
				// In Runout Mode
				else
				{
					// Set max planned months
					l->maxPlannedMonths = runoutMonths;
					// Set final planned months
					l->finalPlannedMonths = runoutMonths;
					// Calculate available money
					l->availableMoney = l->money - l->perMonth * runoutMonths;
					return true;
				}
			}

			// Normal mode
			// Clamp offset between max planned months and 0
			l->offsetMonths = Maths::Clamp(l->offsetMonths, 0, l->maxPlannedMonths);

			// Add the offset extra money
			l->availableMoney += l->offsetMonths * l->perMonth;

			// Clamp final planned months
			l->finalPlannedMonths = Maths::Max(l->maxPlannedMonths - l->offsetMonths, 0);

			return true;
		}
	);

	agg.assigned += money;
	return true;
}

void ConstContainer::Draw()
{
	if (App::DebugMode)
	{
		int m = App::OpenAppTime.MonthsUntil(currentTime);
		if (ImGui::RS::PlusMinusButtonsH("DebugModePlusMinusButtons", &m, 0, 12, "") > 0)
			currentTime = App::OpenAppTime + TimeSpan<>::From::Months(m).AsDate();
		ImGui::SameLine();
		ImGui::Text("%s %d", currentTime.MonthName(), currentTime.Year());
	}

	if (hidden) ImGui::BeginDisabled();

	labels.Iterate(
		[&](Label* label, int i, int size)
		{
			ConstLabel* l = (ConstLabel*)label;

			ImGui::PushID(l->id.Data());
			{
				if (i == 0) { if (ImGui::Button("+")) NewLabel("New Const"); }
				else ImGui::Dummy({ 33, 0 });

				ImGui::SameLine();

				float width = 100.0f;
				if (!unified)
				{
					if (size > 1) { if (ImGui::Button("X")) { labels.Erase(i); return false; } }
					else ImGui::Dummy({ 15, 0 });

					ImGui::SameLine();

					ImGui::PushItemWidth(config->textFieldSize);
					ImGui::InputText("##ConstName", &l->name);
					ImGui::PopItemWidth(); ImGui::SameLine();
				}
				else width += 50;

				// Progress Bar
				ImGui::BeginGroup();
				{
					ImGui::PushItemWidth(width);
					ImGui::DragFloat("##Drag", &l->money, 1.0f, 0.0f, MAX_MONEY, format->Str(), ImGuiSliderFlags_AlwaysClamp);
					ImVec2 itemSize = ImGui::GetItemRectSize();
					itemSize.y -= 15;
					ImGui::PopItemWidth();

					bool negAvailableMoney = l->availableMoney < 0;
					if (negAvailableMoney) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1, 0.1, 0.1, 1));
					float ratio = Maths::Approximately(l->perMonth, 0) ? 1 : l->availableMoney / l->perMonth;
					ImGui::ProgressBar(ratio, itemSize, "");
					if (negAvailableMoney) ImGui::PopStyleColor();
				}
				ImGui::EndGroup();

				ImGui::SameLine();

				ImGui::Text("| %.2f/", l->availableMoney); ImGui::SameLine(0, 0);
				ImGui::Text(format->Str(), l->perMonth); ImGui::SameLine(0, 1);
				ImGui::Text("x%d", l->finalPlannedMonths); ImGui::SameLine(0, 5);
				if (!l->HasExpectedRunout())
				{
					int remainingMonths = l->maxPlannedMonths - l->offsetMonths;
					if (ImGui::RS::PlusMinusButtonsV("##PlusMinusConst", &remainingMonths, 0, l->maxPlannedMonths, "x%d") > 0)
						l->offsetMonths = l->maxPlannedMonths - remainingMonths;
					ImGui::SameLine();
				}

				if (ImGui::Button("Edit"))
				{
					ImGui::OpenPopup("Edit Const Popup");
					select = l;
				}
			}
			ImGui::PopID();

			DrawUserMessage(label);
		}
	);

	if (hidden) ImGui::EndDisabled();

	if (select == nullptr)
		return;

	ImGui::RS::CenterNextWindow();
	ImGui::SetNextWindowSize(ImVec2(260, 170));

	ImGui::PushID(select->id.Data());
	{
		if (ImGui::BeginPopupModal("Edit Const Popup", nullptr, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::PushID(id.Data());
			{
				ImGui::RS::Helper("Define the cost per month of the expense.", "?");
				ImGui::SameLine(); ImGui::Text("Money per month:");

				ImGui::DragFloat("##Drag1", &select->perMonth, 1.0f, 0.0f, MAX_MONEY, format->Str(), ImGuiSliderFlags_AlwaysClamp);

				ImGui::RS::Spacing();

				ImGui::RS::Helper("Set the month when you expect this budget to run out.\nThe expected runout moves automatically when the available money changes.\nA negative available amount means you are spending faster than planned.\nYou must have 'Money Per Month' set and at least 1 extra month in advance to lock up.", "?"); ImGui::SameLine();

				select->CalculateBaseMoneyState();

				const int maxPlannedMonths = select->maxPlannedMonths;
				bool canSetExpectedRunout = select->CanSetExpectedRunout();

				ImGui::BeginDisabled(!canSetExpectedRunout);
				{
					if (!canSetExpectedRunout) select->expectedRunoutDate = DateTime::BaseEpoch;

					ImGui::Text("Set Expected Runout Month:");
					ImGui::SameLine();
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);

					bool enabled = select->HasExpectedRunout();
					bool changed = ImGui::Checkbox("##ExpectedRunout", &enabled);

					int runoutMonths;

					if (changed && enabled)
					{
						// Enabling Runout: always start from the maximum.
						runoutMonths = maxPlannedMonths;
						select->offsetMonths = 0;

						select->expectedRunoutMoney = select->money;
					}
					else if (enabled)
					{
						runoutMonths = currentTime.MonthsUntil(select->expectedRunoutDate);
						runoutMonths = Maths::Clamp(runoutMonths, 1, maxPlannedMonths);
					}
					else
					{
						select->offsetMonths = Maths::Clamp(select->offsetMonths, 0, maxPlannedMonths);
						runoutMonths = maxPlannedMonths - select->offsetMonths;
					}

					if (ImGui::RS::PlusMinusButtonsH("##PlusMinusExpectedRunout", &runoutMonths, 1, maxPlannedMonths, "") > 0)
						select->offsetMonths = maxPlannedMonths - runoutMonths;

					ImGui::SameLine();

					DateTime expected = currentTime + TimeSpan<>::From::Months(runoutMonths).AsDate();
					ImGui::Text("%s %d", expected.MonthName(), expected.Year());

					if (enabled) select->expectedRunoutDate = expected;
					else
					{
						select->expectedRunoutDate = DateTime::BaseEpoch;
						select->expectedRunoutMoney = 0.0f;
					}
				}
				ImGui::EndDisabled();

				// Spacing
				ImGui::RS::Spacing();

				//TODO: fer més bonic i posar un offset també
				if (ImGui::Button("Done"))
				{
					select = nullptr;
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::PopID();
			ImGui::EndPopup();
		}
	}
	ImGui::PopID();

}

void ConstContainer::DrawCashFlow(float widthRatio, float initX) const
{
	labels.Iterate(
		[&](Label* label)
		{
			ConstLabel* l = (ConstLabel*)label;

			ImGui::Text(label->name.c_str());
			ImVec2 pos = ImGui::GetCursorPos();
			ImGui::RS::Separator();
			pos.x += initX - 8;
			pos.y += 16;
			float width = widthRatio * l->finalPlannedMonths;
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + width, pos.y - 15), IM_COL32(80, 140, 255, 200));
		}
	);
}

void ConstContainer::NewLabel(const std::string& name, float money, float perMonth, const DateTime& expectedRunoutDate, int offsetMonths)
{
	Container::NewLabel(new ConstLabel(name, money, perMonth, expectedRunoutDate, offsetMonths));
}

const char* ConstContainer::TypeName() const
{
	return "CONST ";
}

ContainerType ConstContainer::Type() const
{
	return ContainerType::CONSTANT;
}