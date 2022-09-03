#include "EconomyScene.h"
#include <windows.h>

#define VERSION "v1.1"
#define EXTENSION ".nng"
#include "External/ImGuiFileDialog/ImGuiFileDialog.h"

EconomyScene::EconomyScene(Input* input)
{
	this->input = input;
	this->file = new FileManager(EXTENSION);

	totalRecipient = new TotalMoneyRecipient("Total Money", 0.0f);
	unasignedRecipient = new UnasignedMoneyRecipient("Unasigned Money", 0.0f, &showFutureUnasigned, &allowFutureCovering, &showArrearUnasigned, &allowArrearsFill);

	openFileName = "New_File";
	openFilePath.clear();
}

EconomyScene::~EconomyScene()
{
}

bool EconomyScene::Start()
{
	totalRecipient->Start(comboCurrency[currency]);
	unasignedRecipient->Start(comboCurrency[currency]);
	for (Recipient* r : recipients) r->Start(comboCurrency[currency]);

	return true;
}

bool EconomyScene::Update()
{
	if ((int)method > 0) SetMethod();

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
	float arrearMoney = 0;

	for (Recipient* r : recipients)
	{
		if (r->hidden) continue;

		r->Update();

		switch (r->GetType())
		{
		case RecipientType::FUTURE_SINGULAR:
		case RecipientType::FUTURE_PLURAL: futureMoney += r->GetMoney(); break;
		case RecipientType::ARREAR_SINGULAR:
		case RecipientType::ARREAR_PLURAL: arrearMoney -= r->GetMoney(); break;
		case RecipientType::FILTER_SINGULAR:
		case RecipientType::FILTER_PLURAL:
		case RecipientType::LIMIT_SINGULAR:
		case RecipientType::LIMIT_PLURAL:
		default: totalMoney -= r->GetMoney(); break;
		}
	}

	unasignedRecipient->SetMoney(totalMoney + futureMoney + arrearMoney, totalMoney, futureMoney, arrearMoney);
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
	if (loadingV1_0) Loadv1_0();
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
		Write("version").String(VERSION).
		Write("cnfSRT").Bool(showRecipientType).
		Write("cnfSFU").Bool(showFutureUnasigned).
		Write("cnfAFC").Bool(allowFutureCovering).
		Write("cnfTFS").Number(textFieldSize).
		Write("currency").Number(currency).
		// Generic File
		Write("total").Number(total).
		Write("size").Number((int)recipients.size());

	for (Recipient* r : recipients)
	{
		file->EditFile(path).
			Write("name").String(r->GetName()).
			Write("type").Number((int)r->GetType()).
			Write("money").Number(r->GetMoney()).
			Write("hide").Bool(r->hidden).
			Write("open").Bool(r->open);

		switch (r->GetType())
		{
		case RecipientType::FILTER_SINGULAR:
			break;

		case RecipientType::FILTER_PLURAL:
		{
			FilterPlrRecipient* fPR = (FilterPlrRecipient*)r;
			int size = fPR->GetSize();
			file->EditFile(path).
				Write("size").Number(size);

			for (int i = 0; i < size; ++i)
			{
				file->EditFile(path)
					.Write("name").String(fPR->GetLabelName(i))
					.Write("money").Number(fPR->GetLabelMoney(i));
			}
			break;
		}

		case RecipientType::LIMIT_SINGULAR:
		{
			LimitRecipient* lR = (LimitRecipient*)r;
			file->EditFile(path).
				Write("limit").Number(lR->GetLimit());
			break;
		}

		case RecipientType::LIMIT_PLURAL:
		{
			LimitPlrRecipient* lPR = (LimitPlrRecipient*)r;
			int size = lPR->GetSize();
			file->EditFile(path).
				Write("size").Number(size);

			for (int i = 0; i < size; ++i)
			{
				file->EditFile(path)
					.Write("name ").String(lPR->GetLabelName (i))
					.Write("limit").Number(lPR->GetLabelLimit(i))
					.Write("money").Number(lPR->GetLabelMoney(i));
			}
			break;
		}

		case RecipientType::FUTURE_SINGULAR:
			break;

		case RecipientType::FUTURE_PLURAL:
		{
			FuturePlrRecipient* fPR = (FuturePlrRecipient*)r;
			int size = fPR->GetSize();
			file->EditFile(path).
				Write("size").Number(size);

			for (int i = 0; i < size; ++i)
			{
				file->EditFile(path)
					.Write("name").String(fPR->GetLabelName(i))
					.Write("money").Number(fPR->GetLabelMoney(i));
			}
			break;
		}

		case RecipientType::ARREAR_SINGULAR:
			break;

		case RecipientType::ARREAR_PLURAL:
		{
			ArrearPlrRecipient* aPR = (ArrearPlrRecipient*)r;
			int size = aPR->GetSize();
			file->EditFile(path).
				Write("size").Number(size);

			for (int i = 0; i < size; ++i)
			{
				file->EditFile(path)
					.Write("name").String(aPR->GetLabelName(i))
					.Write("money").Number(aPR->GetLabelMoney(i));
			}
			break;
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
	if (!versionError) ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose a path", ".nng", ".");
	std::string path;
	std::string name;
	std::string version;
	size_t format = 0;
	//display
	if (!versionError && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk() == true && file->Exists(ImGuiFileDialog::Instance()->GetFilePathName().c_str(), false))
		{
			path = ImGuiFileDialog::Instance()->GetCurrentPath() + "\\";
			name = ImGuiFileDialog::Instance()->GetCurrentFileName();
			format = ImGuiFileDialog::Instance()->GetCurrentFilter().size();
			ImGuiFileDialog::Instance()->Close();

			// Check if the version file is the same as the program version
			std::string checkPath = path;
			checkPath += name;
			checkPath.erase(checkPath.end() - format, checkPath.end());
			file->ViewFile(checkPath.c_str()).
				Read("version").AsString(version);
			versionError = !SameString(VERSION, version);
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
		if (!versionError) return;
	}

	// Version Error Popup
	if (versionError)
	{
		ImGui::OpenPopup("Version Error");
		ImGui::SetNextWindowPos(ImVec2((1280 / 2) - 180, (720 / 2) - 90));
		ImGui::SetNextWindowSize(ImVec2(360, 180));
		if (ImGui::BeginPopupModal("Version Error", nullptr, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Text("File version different from program's version");
			AddSpacing();
			AddSeparator(2);
			AddSpacing(1);
			ImGui::Text("SOLUTION:");
			ImGui::Text("Read the 'ReadMe.md' about how to load\nolder/newer files (Save & Load section).");
			AddSpacing(2);
			if (ImGui::Button("Go To 'ReadMe'"))
			{
				ShellExecute(NULL, NULL, "https://github.com/Ar-Ess/NoNameGestor/blob/main/README.md", NULL, NULL, SW_SHOWNORMAL);
				ImGui::EndPopup();
				loading = false;
				versionError = false;
				return;
			}
			ImGui::SameLine();
			if (ImGui::Button("Okey"))
			{
				ImGui::EndPopup();
				loading = false;
				versionError = false;
				return;
			}
		}
		ImGui::EndPopup();
		//assert(false && "ERROR: The loaded file version is different from the program's one. SOLUTION: Read the 'ReadMe.md' about how to load older/newer files (Save & Load section). Find the info here: https://github.com/Ar-Ess/NoNameGestor");
		
		return;
	}

	loading = false;

	openFileName = name;
	openFilePath = path;

	path += name;
	path.erase(path.end() - format, path.end());

	float total = 0;
	int size = 0;

	DeleteAllRecipients();

	// Top aspects
	file->ViewFile(path.c_str(), 1).
		// Preferences
		Read("cnfSRT").AsBool(showRecipientType).
		Read("cnfSFU").AsBool(showFutureUnasigned).
		Read("cnfAFC").AsBool(allowFutureCovering).
		Read("cnfTFS").AsFloat(textFieldSize).
		Read("currency").AsInt(currency).
		// General Project
		Read("total").AsFloat(total).
		Read("size").AsInt(size);

	int added = 0;

	for (unsigned int i = 0; i < size; ++i)
	{//  Change depending on how many below aspects \/     \/ Change it depending on how many top aspects
		int positionToRead =                   (i * 5)  +  8 + added;
		int type = -1;
		float money = 0;
		bool hidden = false, open = false;
		std::string name;

		// Below aspects
		file->ViewFile(path.c_str(), positionToRead).
			Read("name").AsString(name).
			Read("type").AsInt(type).
			Read("money").AsFloat(money).
			Read("hide").AsBool(hidden).
			Read("open").AsBool(open);

		switch ((RecipientType)type)
		{
		case RecipientType::FILTER_SINGULAR:
		{
			CreateRecipient((RecipientType)type, name.c_str(), money, hidden, open);
			break;
		}

		case RecipientType::FILTER_PLURAL:
		{
			CreateRecipient((RecipientType)type, name.c_str(), money, hidden, open);
			FilterPlrRecipient* fR = (FilterPlrRecipient*)recipients.back();
			fR->ClearLabels();

			int fSize = 0; //                           \/ Change depending on below aspects amount
			int futurePositionToRead = positionToRead + 5;

			file->ViewFile(path.c_str(), futurePositionToRead).
				Read("size").AsInt(fSize);

			added++;

			for (suint i = 0; i < fSize; ++i)
			{
				std::string fName;
				float fMoney;
				file->ViewFile(path.c_str(), (futurePositionToRead + 1) + (i * 2)).
					Read("name").AsString(fName). // Variables on top
					Read("money").AsFloat(fMoney);

				fR->NewLabel(fName.c_str(), fMoney);

				added += 2; // Change depending on how many variables on top
			}

			break;
		}

		case RecipientType::LIMIT_SINGULAR:
		{
			float limit = 1;
			file->ViewFile(path.c_str(), positionToRead + 4).
				Read("limit").AsFloat(limit);

			CreateRecipient((RecipientType)type, name.c_str(), money, hidden, open);

			((LimitRecipient*)recipients.back())->SetLimit(limit);
			added++;

			break;
		}

		case RecipientType::LIMIT_PLURAL:
		{
			CreateRecipient((RecipientType)type, name.c_str(), money, hidden, open);
			LimitPlrRecipient* lPR = (LimitPlrRecipient*)recipients.back();
			lPR->ClearLabels();

			int lSize = 0; //                           \/ Change depending on below aspects amount
			int futurePositionToRead = positionToRead + 5;

			file->ViewFile(path.c_str(), futurePositionToRead).
				Read("size").AsInt(lSize);

			added++;

			for (suint i = 0; i < lSize; ++i)
			{
				std::string lName;
				float lMoney, lLimit; //     Change depending on "vars on top" \/
				file->ViewFile(path.c_str(), (futurePositionToRead + 1) + (i * 3)).
					Read("name").AsString(lName). // "Vars on top"
					Read("limit").AsFloat(lLimit).
					Read("money").AsFloat(lMoney);

				lPR->NewLabel(lName.c_str(), lMoney, lLimit);

				added += 3; // Change depending on how many "vars on top"
			}
			break;
		}

		case RecipientType::FUTURE_SINGULAR:
		{
			CreateRecipient((RecipientType)type, name.c_str(), money, hidden, open);
			break;
		}

		case RecipientType::FUTURE_PLURAL:
		{
			CreateRecipient((RecipientType)type, name.c_str(), money, hidden, open);
			FuturePlrRecipient* fR = (FuturePlrRecipient*)recipients.back();
			fR->ClearLabels();
			
			int fSize = 0; //                           \/ Change depending on below aspects amount
			int futurePositionToRead = positionToRead + 5;

			file->ViewFile(path.c_str(), futurePositionToRead).
				Read("size").AsInt(fSize);

			added++;

			for (suint i = 0; i < fSize; ++i)
			{
				std::string fName;
				float fMoney;
				file->ViewFile(path.c_str(), (futurePositionToRead + 1) + (i * 2)).
					Read("name").AsString(fName). // Variables on top
					Read("money").AsFloat(fMoney);

				fR->NewLabel(fName.c_str(), fMoney);

				added += 2; // Change depending on how many variables on top
			}

			break;
		}

		case RecipientType::ARREAR_SINGULAR:
		{
			CreateRecipient((RecipientType)type, name.c_str(), money, hidden, open);
			break;
		}

		case RecipientType::ARREAR_PLURAL:
		{
			CreateRecipient((RecipientType)type, name.c_str(), money, hidden, open);
			ArrearPlrRecipient* aR = (ArrearPlrRecipient*)recipients.back();
			aR->ClearLabels();

			int fSize = 0; //                           \/ Change depending on below aspects amount
			int futurePositionToRead = positionToRead + 5;

			file->ViewFile(path.c_str(), futurePositionToRead).
				Read("size").AsInt(fSize);

			added++;

			for (suint i = 0; i < fSize; ++i)
			{
				std::string fName;
				float fMoney;
				file->ViewFile(path.c_str(), (futurePositionToRead + 1) + (i * 2)).
					Read("name").AsString(fName). // Variables on top
					Read("money").AsFloat(fMoney);

				aR->NewLabel(fName.c_str(), fMoney);

				added += 2; // Change depending on how many variables on top
			}

			break;
		}

		default: break;
		}

		recipients.back()->loadOpen = true;
	}

	totalRecipient->SetMoney(total);
	UpdateCurrency();

}

void EconomyScene::Loadv1_0()
{
	if (!loadingV1_0)
	{
		loadingV1_0 = true;
		return;
	}

	// open Dialog Simple
	if (!versionError) ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose a path", ".nng", ".");
	std::string path;
	std::string name;
	std::string version;
	size_t format = 0;
	//display
	if (!versionError && ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk() == true && file->Exists(ImGuiFileDialog::Instance()->GetFilePathName().c_str(), false))
		{
			path = ImGuiFileDialog::Instance()->GetCurrentPath() + "\\";
			name = ImGuiFileDialog::Instance()->GetCurrentFileName();
			format = ImGuiFileDialog::Instance()->GetCurrentFilter().size();
			ImGuiFileDialog::Instance()->Close();

			// Check if the version file is the same as the program version
			std::string checkPath = path;
			checkPath += name;
			checkPath.erase(checkPath.end() - format, checkPath.end());
			file->ViewFile(checkPath.c_str()).
				Read("version").AsString(version);
			versionError = !SameString("v1.0", version);
		}
		else
		{
			ImGuiFileDialog::Instance()->Close();
			loadingV1_0 = false;
			return;
		}
	}
	else
	{
		if (!versionError) return;
	}

	// Version Error Popup
	// -TODO: Check if this error still has sense (Loadv1.0())
	if (versionError)
	{
		ImGui::OpenPopup("Version Error");
		ImGui::SetNextWindowPos(ImVec2((1280 / 2) - 180, (720 / 2) - 90));
		ImGui::SetNextWindowSize(ImVec2(360, 180));
		if (ImGui::BeginPopupModal("Version Error", nullptr, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Text("File version different from program's version");
			AddSpacing();
			AddSeparator(2);
			AddSpacing(1);
			ImGui::Text("SOLUTION:");
			ImGui::Text("Read the 'ReadMe.md' about how to load\nolder/newer files (Save & Load section).");
			AddSpacing(2);
			if (ImGui::Button("Go To 'ReadMe'"))
			{
				ShellExecute(NULL, NULL, "https://github.com/Ar-Ess/NoNameGestor/blob/main/README.md", NULL, NULL, SW_SHOWNORMAL);
				ImGui::EndPopup();
				loadingV1_0 = false;
				versionError = false;
				return;
			}
			ImGui::SameLine();
			if (ImGui::Button("Okey"))
			{
				ImGui::EndPopup();
				loadingV1_0 = false;
				versionError = false;
				return;
			}
		}
		ImGui::EndPopup();
		//assert(false && "ERROR: The loaded file version is different from the program's one. SOLUTION: Read the 'ReadMe.md' about how to load older/newer files (Save & Load section). Find the info here: https://github.com/Ar-Ess/NoNameGestor");

		return;
	}

	loadingV1_0 = false;

	openFileName = name;
	openFilePath = path;

	path += name;
	path.erase(path.end() - format, path.end());

	float total = 0;
	int size = 0;

	DeleteAllRecipients();

	// Top aspects
	file->ViewFile(path.c_str(), 1).
		// Preferences
		Read("cnfSRT").AsBool(showRecipientType).
		Read("cnfSFU").AsBool(showFutureUnasigned).
		Read("cnfAFC").AsBool(allowFutureCovering).
		Read("cnfTFS").AsFloat(textFieldSize).
		Read("currency").AsInt(currency).
		// General Project
		Read("total").AsFloat(total).
		Read("size").AsInt(size);

	int added = 0;

	for (unsigned int i = 0; i < size; ++i)
	{//  Change depending on how many below aspects \/     \/ Change it depending on how many top aspects
		int positionToRead = (i * 5) + 8 + added;
		int type = -1;
		float money = 0;
		bool hidden = false, open = false;
		std::string name;

		// Below aspects
		file->ViewFile(path.c_str(), positionToRead).
			Read("name").AsString(name).
			Read("type").AsInt(type).
			Read("money").AsFloat(money).
			Read("hide").AsBool(hidden).
			Read("open").AsBool(open);

		// Change: v1.1 -> v1.0
		if (type == 4) type = 5;
		else if (type == 5) type = 8;
		else if (type == 6) type = 10;

		switch ((RecipientType)type)
		{
		case RecipientType::FILTER_SINGULAR:
		{
			CreateRecipient((RecipientType)type, name.c_str(), money, hidden, open);
			break;
		}

		case RecipientType::LIMIT_SINGULAR:
		{
			float limit = 1;
			file->ViewFile(path.c_str(), positionToRead + 4).
				Read("limit").AsFloat(limit);

			CreateRecipient((RecipientType)type, name.c_str(), money, hidden, open);

			((LimitRecipient*)recipients.back())->SetLimit(limit);
			added++;

			break;
		}

		case RecipientType::FUTURE_PLURAL:
		{
			CreateRecipient((RecipientType)type, name.c_str(), money, hidden, open);
			FuturePlrRecipient* fR = (FuturePlrRecipient*)recipients.back();
			fR->ClearLabels();

			int fSize = 0; //                           \/ Change depending on below aspects amount
			int futurePositionToRead = positionToRead + 5;

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

				fR->NewLabel(fName.c_str(), fMoney);

				added += 2;
			}

			break;
		}

		case RecipientType::ARREAR_PLURAL:
		{
			CreateRecipient((RecipientType)type, name.c_str(), money, hidden, open);
			ArrearPlrRecipient* aR = (ArrearPlrRecipient*)recipients.back();
			aR->ClearLabels();

			int fSize = 0; //                           \/ Change depending on below aspects amount
			int futurePositionToRead = positionToRead + 5;

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

				aR->NewLabel(fName.c_str(), fMoney);

				added += 2;
			}

			break;
		}

		default: break;
		}

		recipients.back()->loadOpen = true;
	}

	totalRecipient->SetMoney(total);
	UpdateCurrency();

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

			if (ImGui::BeginMenu("Old Loads"))
			{
				if (ImGui::MenuItem("Load v1.0"))
					Loadv1_0();

				ImGui::EndMenu();
			}

			ImGui::Separator();

			ImGui::MenuItem("Preferences", "Ctrl + Shft + P", &preferencesWindow);

			ImGui::EndMenu();
		}
		/*if (ImGui::BeginMenu("Edit"))
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
		}*/
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::BeginMenu("Filter"))
			{
				if (ImGui::MenuItem("Singular"))
					CreateRecipient(RecipientType::FILTER_SINGULAR);

				if (ImGui::MenuItem("Plural"))
					CreateRecipient(RecipientType::FILTER_PLURAL);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Limit "))
			{
				if (ImGui::MenuItem("Singular"))
					CreateRecipient(RecipientType::LIMIT_SINGULAR);

				if (ImGui::MenuItem("Plural"))
					CreateRecipient(RecipientType::LIMIT_PLURAL);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Future"))
			{
				if (ImGui::MenuItem("Singular"))
					CreateRecipient(RecipientType::FUTURE_SINGULAR);

				if (ImGui::MenuItem("Plural"))
					CreateRecipient(RecipientType::FUTURE_PLURAL);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Arrear"))
			{
				if (ImGui::MenuItem("Singular"))
					CreateRecipient(RecipientType::ARREAR_SINGULAR);

				if (ImGui::MenuItem("Plural"))
					CreateRecipient(RecipientType::ARREAR_PLURAL);

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Methods"))
		{
			if (ImGui::MenuItem("Harv Eker"))
				method = Method::MTHD_HARV_EKER;

			if (ImGui::MenuItem("50/30/20 Rule"))
				method = Method::MTHD_50_30_20;

			if (ImGui::MenuItem("Pareto"))
				method = Method::MTHD_PARETO;

			if (ImGui::MenuItem("50/15/5 Rule"))
				method = Method::MTHD_50_15_5;

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("About"))
		{
			ImGui::Text("No Name Gestor %s", VERSION); ImGui::SameLine();

			if (ImGui::Selectable(">")) 
				ShellExecute(NULL, NULL, "https://github.com/Ar-Ess/NoNameGestor", NULL, NULL, SW_SHOWNORMAL);

			if (ImGui::BeginMenu("Third Parties"))
			{
				if (ImGui::MenuItem("SDL - App Loop"))
					ShellExecute(NULL, NULL, "https://www.libsdl.org/", NULL, NULL, SW_SHOWNORMAL);

				if (ImGui::MenuItem("ImGui - Graphic UI"))
					ShellExecute(NULL, NULL, "https://github.com/ocornut/imgui", NULL, NULL, SW_SHOWNORMAL);

				if (ImGui::MenuItem("ImGuiFileDialog - FileDialog"))
					ShellExecute(NULL, NULL, "https://github.com/aiekick/ImGuiFileDialog", NULL, NULL, SW_SHOWNORMAL);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Methods Sources"))
			{
				if (ImGui::BeginMenu("Harv Eker"))
				{
					if(ImGui::MenuItem("Official Source"))
						ShellExecute(NULL, NULL, "https://www.harveker.com/blog/6-step-money-managing-system/", NULL, NULL, SW_SHOWNORMAL);
					
					if (ImGui::MenuItem("Method Book"))
						ShellExecute(NULL, NULL, "https://www.amazon.com/Secrets-Millionaire-Mind-Mastering-Wealth/dp/0060763280", NULL, NULL, SW_SHOWNORMAL);

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("50/30/20 Rule"))
				{
					if (ImGui::MenuItem("Non-Official Source"))
						ShellExecute(NULL, NULL, "https://n26.com/en-eu/blog/50-30-20-rule#:~:text=The%2050%2F30%2F20%20rule%20originates%20from%20the%202005%20book,her%20daughter%2C%20Amelia%20Warren%20Tyagi.", NULL, NULL, SW_SHOWNORMAL);

					if (ImGui::MenuItem("Method Book"))
						ShellExecute(NULL, NULL, "https://www.amazon.es/All-Your-Worth-Ultimate-Lifetime/dp/0743269888", NULL, NULL, SW_SHOWNORMAL);

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Pareto"))
				{
					if (ImGui::MenuItem("Non-Official Source"))
						ShellExecute(NULL, NULL, "https://asana.com/resources/pareto-principle-80-20-rule", NULL, NULL, SW_SHOWNORMAL);

					if (ImGui::MenuItem("Method Book"))
						ShellExecute(NULL, NULL, "https://www.amazon.com/80-20-Principle-Secret-Achieving/dp/0385491743", NULL, NULL, SW_SHOWNORMAL);

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("50/15/5 Rule"))
				{
					if (ImGui::MenuItem("Non-Official Source"))
						ShellExecute(NULL, NULL, "https://blog.avadiancu.com/explaining-the-50/15/5-savings-and-budgeting-rule", NULL, NULL, SW_SHOWNORMAL);

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

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

		AddHelper("Shows the unsigned money in terms\nof future income.", "?"); ImGui::SameLine();
		ImGui::Checkbox("Show Unasigned Future Money ", &showFutureUnasigned);
		
		if (showFutureUnasigned)
		{
			ImGui::Dummy(ImVec2{ 6, 0 }); ImGui::SameLine();
			AddHelper("Allows future money to cover\nactual money whenever it goes\nin negative numbers.\nIMPORTANT:\nUse this option if you know for sure\nyou'll receive the future income.", "?"); ImGui::SameLine();
			ImGui::Checkbox("Allow Future Money Covering ", &allowFutureCovering);
		}

		AddHelper("Shows the unsigned money in terms\nof arrears outcome.", "?"); ImGui::SameLine();
		ImGui::Checkbox("Show Unasigned Arrears Money ", &showArrearUnasigned);

		if (showArrearUnasigned)
		{
			ImGui::Dummy(ImVec2{ 6, 0 }); ImGui::SameLine();
			AddHelper("Allows unasigned actual money to fill\nunasigned arrears whenever they exist.", "?"); ImGui::SameLine();
			ImGui::Checkbox("Allow Arrears Money Filling", &allowArrearsFill);
		}

		AddHelper("Enlarges the size of the text\nlabels of each recipient.", "?"); ImGui::SameLine();
		ImGui::PushItemWidth(textFieldSize);
		ImGui::DragFloat("Text Fiend Size", &textFieldSize, 0.1f, 1.0f, 1000.0f, "%f pts");
		ImGui::PopItemWidth();

		if (ImGui::Combo("Currency", &currency, comboCurrency, 5))
			UpdateCurrency();

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
			ImGui::PushID(r->GetId() / ((i * size) + size * size));
			bool reordered = false;

			bool hidden = r->hidden;
			if (hidden) ImGui::BeginDisabled();

			ImGui::Dummy({ 20, 0 }); ImGui::SameLine();

			if (showRecipientType)
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
					DeleteRecipient(i);
					ImGui::EndPopup();
					ImGui::EndTable();
					ImGui::PopID();
					break;
				}
				if (ImGui::MenuItem("Process"))
				{
					int dif = 1;
					r->GetType() == RecipientType::FUTURE_PLURAL ? dif = 1 : dif = -1;
					float totalResult = totalRecipient->GetMoney() + (r->GetMoney() * dif);
					if (totalResult >= 0)
					{
						*totalRecipient->GetMoneyPtr() = totalResult;
						DeleteRecipient(i);
						ImGui::EndPopup();
						ImGui::EndTable();
						ImGui::PopID();
						break;
					}
				}
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

				ImGui::Dummy({ 15, 0 }); ImGui::SameLine();
				r->Draw();

				ImGui::TreePop();
			}
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
		// Filter button timer
		/*ImGui::Button("FILTER T");
		//if (ImGui::IsItemActive())
		//{
		//	if (chrono.ChronoStart(1.0f))
		//	{
		//		ImGui::OpenPopup("Filter Popup T");
		//		chrono.ChronoStop();
		//	}
		//}
		//if (ImGui::IsItemDeactivated())
		//{
		//	CreateRecipient(RecipientType::FILTER_SINGULAR);
		//	chrono.ChronoStop();
		//}
		//if (ImGui::BeginPopup("Filter Popup T"))
		//{
		//	if (ImGui::MenuItem("Singular"))
		//		CreateRecipient(RecipientType::FILTER_SINGULAR);

		//	if (ImGui::MenuItem("Plural"))
		//		CreateRecipient(RecipientType::FILTER_PLURAL);
		//	ImGui::EndPopup();
		//}*/

		if (ImGui::Button("FILTER"))
			ImGui::OpenPopup("Filter Popup");
		if (ImGui::BeginPopup("Filter Popup"))
		{
			if (ImGui::MenuItem("Singular"))
				CreateRecipient(RecipientType::FILTER_SINGULAR);

			if (ImGui::MenuItem("Plural"))
				CreateRecipient(RecipientType::FILTER_PLURAL);
			ImGui::EndPopup();
		}

		if (ImGui::Button("LIMIT "))
			ImGui::OpenPopup("Limit Popup");
		if (ImGui::BeginPopup("Limit Popup"))
		{
			if (ImGui::MenuItem("Singular"))
				CreateRecipient(RecipientType::LIMIT_SINGULAR);

			if (ImGui::MenuItem("Plural"))
				CreateRecipient(RecipientType::LIMIT_PLURAL);
			ImGui::EndPopup();
		}

		if (ImGui::Button("FUTURE"))
			ImGui::OpenPopup("Future Popup");
		if (ImGui::BeginPopup("Future Popup"))
		{
			if (ImGui::MenuItem("Singular"))
				CreateRecipient(RecipientType::FUTURE_SINGULAR);

			if (ImGui::MenuItem("Plural"))
				CreateRecipient(RecipientType::FUTURE_PLURAL);
			ImGui::EndPopup();
		}

		if (ImGui::Button("ARREAR"))
			ImGui::OpenPopup("Arrear Popup");
		if (ImGui::BeginPopup("Arrear Popup"))
		{
			if (ImGui::MenuItem("Singular"))
				CreateRecipient(RecipientType::ARREAR_SINGULAR);

			if (ImGui::MenuItem("Plural"))
				CreateRecipient(RecipientType::ARREAR_PLURAL);
			ImGui::EndPopup();
		}
		
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

void EconomyScene::UpdateCurrency()
{
	totalRecipient->SetCurrency(comboCurrency[currency]);
	unasignedRecipient->SetCurrency(comboCurrency[currency]);
	for (Recipient* r : recipients) r->SetCurrency(comboCurrency[currency]);
}

void EconomyScene::CreateRecipient(RecipientType recipient, const char* name, float money, bool hidden, bool open)
{
	switch (recipient)
	{
	case RecipientType::FILTER_SINGULAR: recipients.push_back((Recipient*)(new  FilterRecipient(name, money, hidden, open))); break;
	case RecipientType::FILTER_PLURAL: recipients.push_back((Recipient*)(new FilterPlrRecipient(name, money, hidden, open, totalRecipient->GetMoneyPtr()))); break;
	case RecipientType::LIMIT_SINGULAR: recipients.push_back((Recipient*)(new    LimitRecipient(name, money, hidden, open))); break;
	case RecipientType::LIMIT_PLURAL: recipients.push_back((Recipient*)(new   LimitPlrRecipient(name, money, hidden, open, totalRecipient->GetMoneyPtr()))); break;
	case RecipientType::FUTURE_SINGULAR: recipients.push_back((Recipient*)(new  FutureRecipient(name, money, hidden, open))); break;
	case RecipientType::FUTURE_PLURAL: recipients.push_back((Recipient*)(new FuturePlrRecipient(name, money, hidden, open, totalRecipient->GetMoneyPtr()))); break;
	case RecipientType::ARREAR_SINGULAR: recipients.push_back((Recipient*)(new  ArrearRecipient(name, money, hidden, open))); break;
	case RecipientType::ARREAR_PLURAL: recipients.push_back((Recipient*)(new ArrearPlrRecipient(name, money, hidden, open, totalRecipient->GetMoneyPtr()))); break;
	default: break;
	}

	recipients.back()->SetCurrency(comboCurrency[currency]);
}

void EconomyScene::SetMethod()
{
	float money = totalRecipient->GetMoney();

	NewFile();

	totalRecipient->SetMoney(money);

	switch (method)
	{
	case Method::MTHD_PARETO:
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Available", money * 0.8f, false, false);
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Savings", money * 0.2f, false, false);
		break;

	case Method::MTHD_50_15_5:
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Essential", money * 0.5f, false, false);
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Future", money * 0.15f, false, false);
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Unexpected", money * 0.05f, false, false);
		break;

	case Method::MTHD_50_30_20:
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Primary", money * 0.5f, false, false);
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Leisure", money * 0.3f, false, false);
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Savings", money * 0.2f, false, false);
		break;

	case Method::MTHD_HARV_EKER:
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Primary", money * 0.55f, false, false);
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Education", money * 0.1f, false, false);
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Leisure", money * 0.1f, false, false);
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Donations", money * 0.06f, false, false);
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Invest", money * 0.1f, false, false);
		CreateRecipient(RecipientType::FILTER_SINGULAR, "Savings", money * 0.1f, false, false);
		break;

	default:
		break;
	}

	method = Method::MTHD_NO;
}
