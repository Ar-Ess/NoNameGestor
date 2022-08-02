#include "EconomyScene.h"

#define EXTENSION ".nng"
#include "External/ImGuiFileDialog/ImGuiFileDialog.h"

EconomyScene::EconomyScene(Input* input)
{
	this->input = input;
	this->file = new FileManager(EXTENSION);

	totalRecipient = new TotalMoneyRecipient("Total Money", 0.0f);
	unasignedRecipient = new UnasignedMoneyRecipient("Unasigned Money", 0.0f);

	openFileName = "New_File";
	openFilePath.clear();
}

EconomyScene::~EconomyScene()
{
}

bool EconomyScene::Start()
{
	totalRecipient->Start();

	for (Recipient* r : recipients)
		r->Start();

	unasignedRecipient->Start();

	return true;
}

bool EconomyScene::Update()
{
	UpdateShortcuts();

	if (ctrl && shft && d) demoWindow = !demoWindow;
	if (ctrl && shft && p) preferencesWindow = !preferencesWindow;
	if (ctrl && !shft && s) Save();
	if (ctrl && shft && s) SaveAs();
	if (ctrl && !shft && l) Load();

	totalRecipient->Update();
	float totalMoney = totalRecipient->GetMoney();

	for (Recipient* r : recipients)
	{
		r->Update();
		totalMoney -= r->GetMoney();
	}

	unasignedRecipient->Update();
	unasignedRecipient->SetMoney(totalMoney);

	return true;
}

bool EconomyScene::Draw()
{
	bool ret = true;

	ret = DrawDocking();
	ret = DrawMenuBar();

	ret = DrawPreferencesWindow(&preferencesWindow);
	ret = DrawMainWindow(&ret);
	ret = DrawToolbarWindow(&ret);

	if (demoWindow) ImGui::ShowDemoWindow();

	if (saving) Save();
	if (loading) Load();
	if (savingAs) SaveAs();

	return ret;
}

bool EconomyScene::CleanUp()
{
	return true;
}

void EconomyScene::SaveAs()
{
	if (!savingAs)
	{
		savingAs = true;
		return;
	}

	// open Dialog Simple
	ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose a path", ".nng", ".");
	std::string path;
	std::string name;
	size_t format = 0;
	//display
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			path = ImGuiFileDialog::Instance()->GetCurrentPath() + "\\";
			name = ImGuiFileDialog::Instance()->GetCurrentFileName();
			format = ImGuiFileDialog::Instance()->GetCurrentFilter().size();
			ImGuiFileDialog::Instance()->Close();
		}
		else
		{
			ImGuiFileDialog::Instance()->Close();
			savingAs = false;
			return;
		}
	}
	else
	{
		return;
	}

	savingAs = false;

	if (name.empty())
		path += openFileName;
	else
	{
		name.erase(name.end() - format, name.end());
		path += name;
		openFileName.clear();
		openFileName.shrink_to_fit();
		openFileName = name.c_str();
	}

	float total = totalRecipient->GetMoney();
	float unasigned = unasignedRecipient->GetMoney();

	file->OpenFile(path.c_str()).
		Write("total").Number(total).
		Write("size").Number((int)recipients.size());

	for (Recipient* r : recipients)
	{
		file->EditFile(path.c_str()).
			Write("name").String(r->GetName()).
			Write("type").Number((int)r->GetType()).
			Write("money").Number(r->GetMoney());
	}
}

void EconomyScene::Save()
{
	if (!saving)
	{
		if (openFilePath.empty()) savingAs = true;
		else
			saving = true;

		return;
	}

	saving = false;

	std::string savePath = openFilePath + openFileName;

	float total = totalRecipient->GetMoney();
	float unasigned = unasignedRecipient->GetMoney();

	file->OpenFile(savePath.c_str()).
		Write("total").Number(total).
		Write("size").Number((int)recipients.size());

	for (Recipient* r : recipients)
	{
		file->EditFile(savePath.c_str()).
			Write("name").String(r->GetName()).
			Write("type").Number((int)r->GetType()).
			Write("money").Number(r->GetMoney());
	}
}

void EconomyScene::Load()
{
	if (!loading)
	{
		loading = true;
		return;
	}

	// open Dialog Simple
	ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose a path", ".nng", ".");
	std::string path;
	std::string name;
	size_t format = 0;
	//display
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			path = ImGuiFileDialog::Instance()->GetCurrentPath() + "\\";
			name = ImGuiFileDialog::Instance()->GetCurrentFileName();
			format = ImGuiFileDialog::Instance()->GetCurrentFilter().size();
			ImGuiFileDialog::Instance()->Close();
		}
		else
		{
			ImGuiFileDialog::Instance()->Close();
			savingAs = false;
			return;
		}
	}
	else
	{
		return;
	}

	loading = false;

	openFileName = name;
	openFilePath = path;

	path += name;
	path.erase(path.end() - format, path.end());

	float total = 0;
	int size = 0;

	for (Recipient* r : recipients) DeleteAllRecipients();

	file->ViewFile(path.c_str()).
		Read("total").AsFloat(total).
		Read("size").AsInt(size);

	for (unsigned int i = 0; i < size; ++i)
	{
		int type = -1;
		float money = 0;
		std::string name;

		file->ViewFile(path.c_str(), (i * 3) + 2).
			Read("name").AsString(name).
			Read("type").AsInt(type).
			Read("money").AsFloat(money);

		CreateRecipient((RecipientType)type, name.c_str(), money);
	}

	totalRecipient->SetMoney(total);

}

bool EconomyScene::DrawMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save", "Ctrl + S"))
				Save();

			if (ImGui::MenuItem("Save As", "Ctrl + Shft + S"))
				SaveAs();

			if (ImGui::MenuItem("Load", "Ctrl + L"))
				Load();

			ImGui::Separator();

			ImGui::MenuItem("Preferences", "Ctrl + Shft + P", &preferencesWindow);

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::MenuItem("Undo", "Ctrl + Z");
			ImGui::MenuItem("Redo", "Ctrl + Shft + Z");
			ImGui::Separator();
			ImGui::MenuItem("Copy", "Ctrl + C");
			ImGui::MenuItem("Paste", "Ctrl + V");
			ImGui::MenuItem("Cut", "Ctrl + X");
			ImGui::MenuItem("Duplicate", "Ctrl + D");

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Filter"))
				CreateRecipient(RecipientType::FILTER);

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			ImGui::MenuItem("Demo Window", "Ctrl + Shft + D", &demoWindow);

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	return true;
}

bool EconomyScene::DrawDocking()
{
	bool ret = true;

	ImGuiDockNodeFlags dockspace_flags = (ImGuiDockNodeFlags_PassthruCentralNode);
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("Docking", (bool*)0, (ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus));

	ImGui::PopStyleVar();

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & (ImGuiConfigFlags_DockingEnable))
	{
		ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	ImGui::End();

	return ret;
}

bool EconomyScene::DrawPreferencesWindow(bool* open)
{
	bool ret = true;
	if (!(*open)) return ret;

	if (ImGui::Begin("Preferences", open))
	{
		ImGui::End();
	}

	return ret;
}

bool EconomyScene::DrawMainWindow(bool* open)
{
	bool ret = true;
	if (!(*open)) return ret;

	if (ImGui::Begin("##Main", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::Text(openFileName.c_str());
		AddSeparator(1);
		AddSpacing(3);
		totalRecipient->Draw();

		size_t size = recipients.size();
		int i = 0;
		for (Recipient* r : recipients)
		{
			bool reordered = false;
			ImGui::PushID(i);
			ImGui::PushItemWidth(200.0f);

			ImGui::Text(r->GetTypeString()); ImGui::SameLine(); ImGui::InputText("##LabelName", r->GetString()); ImGui::PopItemWidth(); ImGui::SameLine();
			if (ImGui::TreeNodeEx("[]", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding))
			{
				if (ImGui::BeginDragDropSource())
				{
					intptr_t id = r->GetId();
					ImGui::SetDragDropPayload("Recipient", &id, sizeof(intptr_t));
					ImGui::Text(r->GetName());
					ImGui::EndDragDropSource();
				}
				if (ImGui::BeginDragDropTarget())
				{
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Recipient");
					if (payload)
					{
						MoveRecipient(ReturnRecipientIndex(*((intptr_t*)payload->Data)), i);
						reordered = true;
					}
					ImGui::EndDragDropTarget();
				}

				r->Draw();

				ImGui::TreePop();
			}
			ImGui::PopID();

			++i;
			if (reordered) break;
		}

		unasignedRecipient->Draw();

		ImGui::End();
	}

	return ret;
}

bool EconomyScene::DrawToolbarWindow(bool* open)
{
	bool ret = true;
	if (!(*open)) return ret;

	if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::End();
	}

	return ret;
}

void EconomyScene::UpdateShortcuts()
{
	ctrl = input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_REPEAT;
	shft = input->GetKey(SDL_SCANCODE_LSHIFT) == KeyState::KEY_REPEAT;

	d    = input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_DOWN;
	p    = input->GetKey(SDL_SCANCODE_P) == KeyState::KEY_DOWN;
	s    = input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_DOWN;
	l    = input->GetKey(SDL_SCANCODE_L) == KeyState::KEY_DOWN;
}

void EconomyScene::CreateRecipient(RecipientType recipient, const char* name, float money)
{
	switch (recipient)
	{
	case RecipientType::FILTER: recipients.push_back((Recipient*)(new FilterRecipient(name, money)));

	default:
		return;
	}
}
