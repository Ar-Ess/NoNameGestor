#include "EconomyScene.h"

#define EXTENSION ".nng"
#include "External/ImGuiFileDialog/ImGuiFileDialog.h"

EconomyScene::EconomyScene(Input* input)
{
	this->input = input;
	this->file = new FileManager(EXTENSION);

	totalRecipient = new TotalMoneyRecipient("Total Money", 0.0f);
	unasignedRecipient = new UnasignedMoneyRecipient("Unasigned Money", 0.0f, &showFutureUnasigned, &allowFutureCovering);

	openFileName = "New_File";
	openFilePath.clear();
}

EconomyScene::~EconomyScene()
{
}

bool EconomyScene::Start()
{
	return true;
}

bool EconomyScene::Update()
{
	UpdateShortcuts();

	if (ctrl &&  shft && d) demoWindow = !demoWindow;
	if (ctrl &&  shft && p) preferencesWindow = !preferencesWindow;
	if (ctrl && !shft && s) Save();
	if (ctrl &&  shft && s) SaveAs();
	if (ctrl && !shft && l) Load();
	if (ctrl && !shft && n) NewFile();

	totalRecipient->Update();
	float totalMoney = totalRecipient->GetMoney();
	float futureMoney = 0;

	for (Recipient* r : recipients)
	{
		r->Update();
		if (r->GetType() == RecipientType::FUTURE)
			futureMoney += r->GetMoney();
		else
			totalMoney -= r->GetMoney();
	}

	unasignedRecipient->SetMoney(totalMoney + futureMoney, totalMoney, futureMoney);
	unasignedRecipient->Update();


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

void EconomyScene::NewFile()
{
	openFileName = "New_File";
	openFilePath.clear();

	DeleteAllRecipients();

	totalRecipient->SetMoney(0.0f);
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

	openFilePath = path;

	if (name.empty())
		path += openFileName;
	else
	{
		openFileName.clear();
		openFileName.shrink_to_fit();
		openFileName = name.c_str();
		name.erase(name.end() - format, name.end());
		path += name;
	}

	InternalSave(path.c_str());
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

	InternalSave(savePath.c_str());

}

void EconomyScene::InternalSave(const char* path)
{
	float total = totalRecipient->GetMoney();
	float unasigned = unasignedRecipient->GetMoney();

	file->OpenFile(path).
		// Preferences
		Write("cnfSRT").Bool(showRecipientType).
		Write("cnfSFU").Bool(showFutureUnasigned).
		Write("cnfAFC").Bool(allowFutureCovering).
		Write("cnfTFS").Number(textFieldSize).
		// Generic File
		Write("total").Number(total).
		Write("size").Number((int)recipients.size());

	for (Recipient* r : recipients)
	{
		file->EditFile(path).
			Write("name").String(r->GetName()).
			Write("type").Number((int)r->GetType()).
			Write("money").Number(r->GetMoney());

		switch (r->GetType())
		{
		case RecipientType::FILTER:
			break;

		case RecipientType::LIMIT:
		{
			LimitRecipient* lR = (LimitRecipient*)r;
			file->EditFile(path).
				Write("limit").Number(lR->GetLimit());
			break;
		}
		case RecipientType::FUTURE:
		{
			FutureRecipient* fR = (FutureRecipient*)r;
			int size = fR->GetSize();
			file->EditFile(path).
				Write("size").Number(size);

			for (int i = 0; i < size; ++i)
			{
				file->EditFile(path)
					.Write("name").String(fR->GetFutureName(i))
					.Write("money").Number(fR->GetFutureMoney(i));
			}
		}

		default:
			break;
		}
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
		if (ImGuiFileDialog::Instance()->IsOk() == true && file->Exists(ImGuiFileDialog::Instance()->GetFilePathName().c_str(), false))
		{
			path = ImGuiFileDialog::Instance()->GetCurrentPath() + "\\";
			name = ImGuiFileDialog::Instance()->GetCurrentFileName();
			format = ImGuiFileDialog::Instance()->GetCurrentFilter().size();
			ImGuiFileDialog::Instance()->Close();
		}
		else
		{
			ImGuiFileDialog::Instance()->Close();
			loading = false;
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
		// Preferences
		Read("cnfSRT").AsBool(showRecipientType).
		Read("cnfSFU").AsBool(showFutureUnasigned).
		Read("cnfAFC").AsBool(allowFutureCovering).
		Read("cnfTFS").AsFloat(textFieldSize).
		// General Project
		Read("total").AsFloat(total).
		Read("size").AsInt(size);

	int added = 0;

	for (unsigned int i = 0; i < size; ++i)
	{//                               \/ Change it depending on how many variables readed on top
		int positionToRead = (i * 3) + 6 + added;
		int type = -1;
		float money = 0;
		std::string name;

		file->ViewFile(path.c_str(), positionToRead).
			Read("name").AsString(name).
			Read("type").AsInt(type).
			Read("money").AsFloat(money);

		switch ((RecipientType)type)
		{
		case RecipientType::FILTER:
		{
			CreateRecipient((RecipientType)type, name.c_str(), money);
			break;
		}

		case RecipientType::LIMIT:
		{
			float limit = 1;
			file->ViewFile(path.c_str(), positionToRead + 3).
				Read("limit").AsFloat(limit);

			CreateRecipient((RecipientType)type, name.c_str(), money, limit);

			added++;

			break;
		}

		case RecipientType::FUTURE:
		{
			CreateRecipient((RecipientType)type, name.c_str(), money);
			FutureRecipient* fR = (FutureRecipient*)recipients.back();
			fR->ClearFutures();
			
			int fSize = 0;
			int futurePositionToRead = positionToRead + 3;

			file->ViewFile(path.c_str(), futurePositionToRead).
				Read("size").AsInt(fSize);

			added++;

			for (suint i = 0; i < fSize; ++i)
			{
				std::string fName;
				float fMoney;
				file->ViewFile(path.c_str(), (futurePositionToRead + 1) + (i * 2)).
					Read("name").AsString(fName).
					Read("money").AsFloat(fMoney);

				fR->NewFuture(fName.c_str(), fMoney);

				added += 2;
			}

			break;
		}

		default: break;
		}
	}

	totalRecipient->SetMoney(total);

}

bool EconomyScene::DrawMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New File", "Ctrl + N"))
				NewFile();

			ImGui::Separator();

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
			ImGui::Text("Undo/Redo Future Implementation");
			//ImGui::MenuItem("Undo", "Ctrl + Z");
			//ImGui::MenuItem("Redo", "Ctrl + Shft + Z");
			ImGui::Separator();
			ImGui::Text("Copy/Paste/Cut/Duplicate Future Implementation");
			//ImGui::MenuItem("Copy", "Ctrl + C");
			//ImGui::MenuItem("Paste", "Ctrl + V");
			//ImGui::MenuItem("Cut", "Ctrl + X");
			//ImGui::MenuItem("Duplicate", "Ctrl + D");
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Filter"))
				CreateRecipient(RecipientType::FILTER);

			if (ImGui::MenuItem("Limit "))
				CreateRecipient(RecipientType::LIMIT);

			if (ImGui::MenuItem("Future"))
				CreateRecipient(RecipientType::FUTURE);
			ImGui::EndMenu();
		}
		//if (ImGui::BeginMenu("Window"))
		//{
		//	ImGui::MenuItem("Demo Window", "Ctrl + Shft + D", &demoWindow);
		//	ImGui::EndMenu();
		//}
	}
	ImGui::EndMainMenuBar();

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

	if (ImGui::Begin("Preferences", open, ImGuiWindowFlags_NoDocking))
	{
		AddHelper("Shows, at the side of each recipient,\na text noting it's type.", "?"); ImGui::SameLine();
		ImGui::Checkbox("Show Recipient Typology Name", &showRecipientType); 

		AddHelper("Shows the unsigned money splitted\nin terms of actual income and\nfuture income.\n", "?"); ImGui::SameLine();
		ImGui::Checkbox("Show Unasigned Future Money ", &showFutureUnasigned);
		
		if (showFutureUnasigned)
		{
			ImGui::Dummy(ImVec2{ 6, 0 }); ImGui::SameLine();
			AddHelper("Allows future money to cover\nactual money whenever it goes\nin negative numbers.\nIMPORTANT:\nUse this option if you know for sure\nyou'll receive the future income.", "?"); ImGui::SameLine();
			ImGui::Checkbox("Allow Future Money Covering ", &allowFutureCovering);
		}

		AddHelper("Enlarges the size of the text\nlabels of each recipient.", "?"); ImGui::SameLine();
		ImGui::PushItemWidth(textFieldSize);
		ImGui::DragFloat("Text Fiend Size", &textFieldSize, 0.1f, 1.0f, 1000.0f, "%f pts");
		ImGui::PopItemWidth();

	}
	ImGui::End();

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
		AddSpacing(2);

		totalRecipient->Draw();

		AddSpacing(2);

		size_t size = recipients.size();
		for (suint i = 0; i < size; ++i)
		{
			Recipient* r = recipients[i];
			ImGui::PushID((i / size) * size / r->GetId());
			bool reordered = false;
			if (ImGui::BeginTable("##table", 2, ImGuiTableFlags_SizingStretchProp))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("|");

				ImGui::TableNextColumn();

				ImGui::PushItemWidth(textFieldSize);

				if (ImGui::Button("X"))
				{
					DeleteRecipient(i);
					ImGui::PopItemWidth();
					ImGui::EndTable();
					ImGui::PopID();
					break;
				}
				ImGui::SameLine();
				if (showRecipientType)
				{ 
					ImGui::Text(r->GetTypeString()); 
					ImGui::SameLine();
				}
				ImGui::InputText("##LabelName", r->GetString()); ImGui::PopItemWidth(); ImGui::SameLine();
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
				}
				ImGui::TreePop();
			}
			ImGui::EndTable();
			ImGui::PopID();

			AddSpacing(0);

			if (reordered) break;
		}

		AddSpacing(0);

		unasignedRecipient->Draw();
	}
	ImGui::End();

	return ret;
}

bool EconomyScene::DrawToolbarWindow(bool* open)
{
	bool ret = true;
	if (!(*open)) return ret;

	if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		if (ImGui::Button("FILTER")) CreateRecipient(RecipientType::FILTER);
		if (ImGui::Button("LIMIT ")) CreateRecipient(RecipientType::LIMIT);
		if (ImGui::Button("FUTURE")) CreateRecipient(RecipientType::FUTURE);
	}
	ImGui::End();

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
	n    = input->GetKey(SDL_SCANCODE_N) == KeyState::KEY_DOWN;
}

void EconomyScene::CreateRecipient(RecipientType recipient, const char* name, float money, float limit)
{
	switch (recipient)
	{
	case RecipientType::FILTER: recipients.push_back((Recipient*)(new FilterRecipient(name, money))); break;
	case RecipientType::LIMIT : recipients.push_back((Recipient*)(new  LimitRecipient(name, money, limit))); break;
	case RecipientType::FUTURE: recipients.push_back((Recipient*)(new FutureRecipient(name, money, totalRecipient->GetMoneyPtr()))); break;
	default: break;
	}
}
