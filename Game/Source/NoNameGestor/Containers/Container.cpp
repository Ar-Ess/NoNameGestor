#include "Container.h"

#include "Framework/Coroutine/CoroutineManager.h"
#include "Framework/Render/Color.h"

#include "NoNameGestor/Gestor/Configuration.h"
#include "NoNameGestor/Gestor/Aggregate.h"
#include "NoNameGestor/Utils/ImGuiExtension.h"

#include "NoNameGestor/External/imgui/imgui.h"
#include "NoNameGestor/External/imgui/imgui_stdlib.h"
#include "NoNameGestor/External/imgui/imgui_internal.h"

bool Container::UpdateOpenState = false;

Container::~Container()
{
}

bool Container::Update(Aggregate& agg)
{
	if (hidden)
		return false;

	money = 0;
	labels.Iterate([&](Label* l)
		{ money += l->money; });

	return true;
}

bool Container::DrawExport() const
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

bool Container::DrawBase(float maxWidth, bool& erase, ID& move)
{
	bool ret = true;
	bool reordered = false;
	this->maxWidth = maxWidth;

	ImGui::PushID(id.Data());
	{
		// If Hidden, begin disabled group
		if (hidden) ImGui::BeginDisabled();

		// Generate somw width space
		ImGui::Dummy({ 20, 0 }); ImGui::SameLine();

		// Add the container type if user defined
		if (config->showContainerType) ImGui::Text(TypeName()); ImGui::SameLine();

		// Push text user defined width and create the Input Texts for the container name
		ImGui::PushItemWidth(config->textFieldSize);
		{
			ImGui::InputText("##LabelName", CurrentName());
		}
		ImGui::PopItemWidth(); ImGui::SameLine();

		// End disabled group
		if (hidden) ImGui::EndDisabled();

		// Generate button to open options popup
		if (ImGui::Button(":")) ImGui::OpenPopup("Options Popup");

		// Options Popup window begin
		if (ImGui::BeginPopup("Options Popup"))
		{
			// Delete Container
			if (ImGui::MenuItem("Delete"))
			{
				erase = true;
				ret = false;
			}

			// Unify Container
			if (labels.Size() <= 1) ImGui::MenuItem("Unify", "", &unified);

			// Hide Container
			ImGui::MenuItem("Hide", "", &hidden);

			// End Popup
			ImGui::EndPopup();
		}

		ImGui::SameLine();

		if (UpdateOpenState) ImGui::SetNextItemOpen(open);

		//bool nodeOpen = c->open;

		// Dibuixa la fletxeta com a botó
		if (ImGui::ArrowButton("##arrow", open ? ImGuiDir_Down : ImGuiDir_Right))
			open = !open;

		// Get if Arrow Button is hovered
		bool showMoneySum = ImGui::IsItemHovered();

		ImGui::SameLine();

		// Calculate Invisible Reorder Button
		ImVec2 textSize = ImGui::CalcTextSize("[]");
		ImVec2 textPos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##hoverZone", textSize);
		bool hoveringText = ImGui::IsItemHovered();

		// Add the text on top of the invisible button
		ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), hoveringText ? "[ ]" : "[]");

		// Drag & Drop source
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::SetDragDropPayload("Container", &id, sizeof(ID));
			ImGui::Text(CurrentName()->c_str());
			ImGui::EndDragDropSource();
		}

		// Drag & Drop target
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Container");
			if (payload != nullptr)
			{
				move = *(ID*)payload->Data;
				ret = false;
			}
			ImGui::EndDragDropTarget();
		}

		if (showMoneySum)
		{
			ImGui::SameLine();
			ImGui::Text(format->Str(), money);
		}

		if (open)
		{
			ImGui::Indent();
			ImGui::Dummy({ 10, 0 }); ImGui::SameLine();
			Draw();
			ImGui::Unindent();
		}
	}
	ImGui::PopID();

	ImGui::RS::Spacing(0);

	return ret;
}

void Container::Save(FileManager::FileNode node) const
{
	node.Write("name", name.c_str());
	node.Write("type", (int)Type());
	node.Write("money", money);
	node.Write("hide", hidden);
	node.Write("open", open);
	node.Write("unfd", unified);
	node.Write("labels", FileManager::File::Array);
	auto lnode = node.Access("labels");

	labels.Iterate(
		[&](Label* l)
		{
			// Push a new object
			int index = lnode.Push(FileManager::File::Object);

			// Assure it is correctly pushed
			Debug::Assert(index != -1, "Internal Save Error: Pushing New Container returned an error.");

			// Find the specific object
			auto newNode = lnode.Access(index);

			// Add the values
			newNode.Write("name", l->name);
			newNode.Write("money", l->money);
		}
	);
}

std::string* Container::CurrentName()
{
	return !unified || labels.IsEmpty() ? &name : &labels[0]->name;
}

const std::string* Container::CurrentName() const
{
	return !unified || labels.IsEmpty() ? &name : &labels[0]->name;
}

const char* Container::TypeName() const
{
	return "NO CONTAINER";
}

ContainerType Container::Type() const
{
	return ContainerType::NO_CONTAINER;
}

float Container::Money() const
{
	return money;
}

unsigned int Container::Size() const
{
	return labels.Size();
}

void Container::Export(std::ofstream& exp) const
{
	exp << CurrentName() << ":";
	auto currency = format->Substring(5, format->Length() - 5).Str();

	if (unified)
		exp << " " << money << " " << currency << std::endl << std::endl;
	else
	{
		exp << std::endl;
		labels.Iterate(
			[&](Label* l)
			{
				exp << " - " << l->name << ": " << l->money << " " << currency << std::endl << std::endl;
			}
		);
	}
}

Container::Container(const std::string& name, bool hidden, bool open, bool unified, String* format, Configuration* config) :
	id(ID::New()),
	labelDrawID(ID::Empty),
	money(0),
	name(name),
	hidden(hidden),
	open(open),
	unified(unified),
	format(format),
	maxWidth(maxWidth),
	config(config)
{
}

Container::Container(const FileManager::FileNode& node, String* format, Configuration* config) :
	id(ID::New()),
	labelDrawID(ID::Empty),
	format(format),
	maxWidth(maxWidth),
	config(config)
{
	name = node.Read <std::string>("name");
	money = node.Read<float>("money");
	hidden = node.Read<bool>("hide");
	open = node.Read<bool>("open");
	unified = node.Read<bool>("unfd");

	UpdateOpenState = true;
}

void Container::NewLabel(Label* label)
{
	if (!labels.IsEmpty() && !UpdateOpenState)
		unified = false;

	labels.PushBack(label);
}

void Container::DrawUserMessage(Label* label)
{
	if (message.IsNull() || !labelDrawID.HasValue() || labelDrawID.Value() != label->id)
		return;

	ImVec4 col = ImVec4(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
	ImGui::PushStyleColor(ImGuiCol_Text, col);
	ImGui::PushTextWrapPos(maxWidth);
	ImGui::TextWrapped(message.Str());
	ImGui::PopTextWrapPos();
	ImGui::PopStyleColor();
}

void Container::SetWarningMessage(Label* label, const char* message, float time, float fade)
{
	SetMessage(label, message, time, fade, Color(235, 235, 0, 255));
}

void Container::SetErrorMessage(Label* label, const char* message, float time, float fade)
{
	SetMessage(label, message, time, fade, Color::Red);
}

void Container::SetMessage(Label* label, const char* message, float time, float fade, const Color& c)
{
	if (!labelDrawID.HasValue())
		return;
	
	color = c;
	this->message = message;
	labelDrawID = label->id;
	CoroutineManager::Begin(RunMessage(time, fade));
}

Coroutine Container::RunMessage(float time, float fade)
{
	co_yield Yield::WaitForSeconds(time);

	for (float i = 0; i < fade; i += App::DeltaTime())
	{
		color.a = (1 - (i / fade)) * 255;
		co_yield Yield::WaitForNextFrame();
	}

	message = nullptr;
	labelDrawID.Clear();
}
