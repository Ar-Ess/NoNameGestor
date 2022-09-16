#include "EconomyScene.h"
#include <windows.h>

#define VERSION "v1.1"
#define EXTENSION ".nng"
#include "External/ImGuiFileDialog/ImGuiFileDialog.h"

EconomyScene::EconomyScene(Input* input)
{
	this->input = input;
	this->file = new FileManager(EXTENSION);

	totalContainer = new TotalContainer("Total Money");
	unasignedContainer = new UnasignedContainer("Unasigned Money", &showFutureUnasigned, &allowFutureCovering, &showArrearUnasigned, &allowArrearsFill);

	openFileName = "New_File";
	openFilePath.clear();

}

EconomyScene::~EconomyScene()
{
}

bool EconomyScene::Start()
{
	totalContainer->Start(comboCurrency[currency]);
	unasignedContainer->Start(comboCurrency[currency]);
	for (Container* r : containers) r->Start(comboCurrency[currency]);

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

	totalContainer->Update();
	float totalMoney = totalContainer->GetMoney();
	float futureMoney = 0;
	float arrearMoney = 0;

	for (Container* r : containers)
	{
		if (r->hidden) continue;

		r->Update();

		switch (r->GetType())
		{
		case ContainerType::FUTURE: futureMoney += r->GetMoney(); break;
		case ContainerType::ARREAR: arrearMoney -= r->GetMoney(); break;
		case ContainerType::FILTER:
		case ContainerType::LIMIT:
		default: totalMoney -= r->GetMoney(); break;
		}
	}

	unasignedContainer->SetMoney(totalMoney + futureMoney + arrearMoney, totalMoney, futureMoney, arrearMoney);
	unasignedContainer->Update();

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
	//openFiles.emplace_back(File{"New_File", nullptr});
	openFileName = "New_File";
	openFilePath.clear();

	DeleteAllContainer();

	totalContainer->SetMoney(0.0f);
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
	float total = totalContainer->GetMoney();
	float unasigned = unasignedContainer->GetMoney();

	file->OpenFile(path).
		// Preferences
		Write("version").String(VERSION).
		Write("cnfSRT").Bool(showContainerType).
		Write("cnfSFU").Bool(showFutureUnasigned).
		Write("cnfAFC").Bool(allowFutureCovering).
		Write("cnfCCU").Bool(createContainerUnified).
		Write("cnfTFS").Number(textFieldSize).
		Write("currency").Number(currency).
		// Generic File
		Write("total").Number(total).
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
		case ContainerType::FILTER:
		{
			FilterContainer* fPR = (FilterContainer*)r;
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

		case ContainerType::LIMIT:
		{
			LimitContainer* lPR = (LimitContainer*)r;
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

		case ContainerType::FUTURE:
		{
			FutureContainer* fPR = (FutureContainer*)r;
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

		case ContainerType::ARREAR:
		{
			ArrearContainer* aPR = (ArrearContainer*)r;
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

	DeleteAllContainer();

	// Y aspects
	file->ViewFile(path.c_str(), 1).
		// Preferences
	  //Read("version")
		Read("cnfSRT").AsBool(showContainerType).
		Read("cnfSFU").AsBool(showFutureUnasigned).
		Read("cnfAFC").AsBool(allowFutureCovering).
		Read("cnfCCU").AsBool(createContainerUnified).
		Read("cnfTFS").AsFloat(textFieldSize).
		Read("currency").AsInt(currency).
		// General Project
		Read("total").AsFloat(total).
		Read("size").AsInt(size);

	int added = 0;

	for (unsigned int i = 0; i < size; ++i)
	{//      Change depending on how many X aspects \/     \/ Change it depending on how many Y aspects (count version as one)
		int positionToRead =                   (i * 6)  +  9 + added;
		int type = -1;
		float money = 0;
		bool hidden = false, open = false, unified = true;
		std::string name;

		// X aspects
		file->ViewFile(path.c_str(), positionToRead).
			Read("name").AsString(name).
			Read("type").AsInt(type).
			Read("money").AsFloat(money).
			Read("hide").AsBool(hidden).
			Read("open").AsBool(open).
			Read("unfd").AsBool(unified);

		switch ((ContainerType)type)
		{
		case ContainerType::FILTER:
		{
			CreateContainer((ContainerType)type, name.c_str(), hidden, open);
			FilterContainer* fC = (FilterContainer*)containers.back();
			fC->ClearLabels();
			fC->unified = unified;
			fC->loadOpen = true;

			int fSize = 0; //                           \/ Change depending on X aspects amount
			int futurePositionToRead = positionToRead + 6;

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

				fC->NewLabel(fName.c_str(), fMoney);

				added += 2; // Change depending on how many variables on top
			}

			break;
		}

		case ContainerType::LIMIT:
		{
			CreateContainer((ContainerType)type, name.c_str(), hidden, open);
			LimitContainer* lC = (LimitContainer*)containers.back();
			lC->ClearLabels();
			lC->unified = unified;
			lC->loadOpen = true;

			int lSize = 0; //                           \/ Change depending on X aspects amount
			int futurePositionToRead = positionToRead + 6;

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

				lC->NewLabel(lName.c_str(), lMoney, lLimit);

				added += 3; // Change depending on how many "vars on top"
			}
			break;
		}

		case ContainerType::FUTURE:
		{
			CreateContainer((ContainerType)type, name.c_str(), hidden, open);
			FutureContainer* fC = (FutureContainer*)containers.back();
			fC->ClearLabels();
			fC->unified = unified;
			fC->loadOpen = true;
			
			int fSize = 0; //                           \/ Change depending on X aspects amount
			int futurePositionToRead = positionToRead + 6;

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

				fC->NewLabel(fName.c_str(), fMoney);

				added += 2; // Change depending on how many variables on top
			}

			break;
		}

		case ContainerType::ARREAR:
		{
			CreateContainer((ContainerType)type, name.c_str(), hidden, open);
			ArrearContainer* aC = (ArrearContainer*)containers.back();
			aC->ClearLabels();
			aC->unified = unified;
			aC->loadOpen = true;

			int fSize = 0; //                           \/ Change depending on X aspects amount
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

				aC->NewLabel(fName.c_str(), fMoney);

				added += 2; // Change depending on how many variables on top
			}

			break;
		}

		default: break;
		}

	}

	totalContainer->SetMoney(total);
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

	DeleteAllContainer();

	// Y aspects
	file->ViewFile(path.c_str(), 1).
		// Preferences
		Read("cnfSRT").AsBool(showContainerType).
		Read("cnfSFU").AsBool(showFutureUnasigned).
		Read("cnfAFC").AsBool(allowFutureCovering).
		Read("cnfTFS").AsFloat(textFieldSize).
		Read("currency").AsInt(currency).
		// General Project
		Read("total").AsFloat(total).
		Read("size").AsInt(size);

	int added = 0;

	for (unsigned int i = 0; i < size; ++i)
	{ //Change depednig X aspects \/   \/ Change it depending on how many Y aspects
		int positionToRead = (i * 5) + 8 + added;
		int type = -1;
		float money = 0;
		bool hidden = false, open = false;
		std::string name;

		// X aspects
		file->ViewFile(path.c_str(), positionToRead).
			Read("name").AsString(name).
			Read("type").AsInt(type).
			Read("money").AsFloat(money).
			Read("hide").AsBool(hidden).
			Read("open").AsBool(open);


		switch ((ContainerType)type)
		{
		case ContainerType::FILTER:
		{
			CreateContainer((ContainerType)type, "Filter Name", hidden, open);
			// Change: v1.1 -> v1.0 (Filters are plural forever)
			FilterContainer* fR = (FilterContainer*)containers.back();
			fR->ClearLabels();

			fR->NewLabel(name.c_str(), money);

			break;
		}

		case ContainerType::LIMIT:
		{
			float limit = 1;
			file->ViewFile(path.c_str(), positionToRead + 4).
				Read("limit").AsFloat(limit);

			CreateContainer((ContainerType)type, "Limit Name", hidden, open);
			// Change: v1.1 -> v1.0 (Limits are plural forever)
			LimitContainer* lR = (LimitContainer*)containers.back();
			lR->ClearLabels();

			lR->NewLabel(name.c_str(), money, limit);
			added++;

			break;
		}

		case ContainerType::FUTURE:
		{
			CreateContainer((ContainerType)type, name.c_str(), hidden, open);
			FutureContainer* fR = (FutureContainer*)containers.back();
			fR->ClearLabels();

			int fSize = 0; //                           \/ Change depending on X aspects amount
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

		case ContainerType::ARREAR:
		{
			CreateContainer((ContainerType)type, name.c_str(), hidden, open);
			ArrearContainer* aR = (ArrearContainer*)containers.back();
			aR->ClearLabels();

			int fSize = 0; //                           \/ Change depending on X aspects amount
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

		containers.back()->loadOpen = true;
	}

	totalContainer->SetMoney(total);
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
			if (ImGui::MenuItem("Filter"))
				CreateContainer(ContainerType::FILTER);

			if (ImGui::MenuItem("Limit"))
				CreateContainer(ContainerType::LIMIT);

			if (ImGui::MenuItem("Future"))
				CreateContainer(ContainerType::FUTURE);

			if (ImGui::MenuItem("Arrear"))
				CreateContainer(ContainerType::ARREAR);

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
		if (ImGui::BeginMenu("Window"))
		{
			ImGui::MenuItem("Demo Window", "Ctrl + Shft + D", &demoWindow);
			ImGui::EndMenu();
		}
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
		AddHelper("Shows, at the side of each container,\na text noting it's type.", "?"); ImGui::SameLine();
		ImGui::Checkbox("Show Container Typology Name", &showContainerType); 

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

		AddHelper("Enlarges the size of the text\nlabels of each container.", "?"); ImGui::SameLine();
		ImGui::PushItemWidth(textFieldSize);
		ImGui::DragFloat("Text Fiend Size", &textFieldSize, 0.1f, 1.0f, 1000.0f, "%f pts", ImGuiSliderFlags_AlwaysClamp);
		ImGui::PopItemWidth();

		if (ImGui::Combo("Currency", &currency, comboCurrency, 5))
			UpdateCurrency();

		AddHelper("Creates all containers unified by default", "?"); ImGui::SameLine();
		ImGui::Checkbox("Create Container Unified Default", &createContainerUnified);

	}
	ImGui::End();

	return ret;
}

bool EconomyScene::DrawMainWindow(bool* open)
{
	bool ret = true;
	if (!(*open)) return ret;

	if (ImGui::Begin("##MainWindow", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
	{
		if (ImGui::BeginTabBar("##FileBar"))
		{
			AddSpacing(1);

			if (ImGui::BeginTabBar("##SystemBar"))
			{
				if (ImGui::BeginTabItem("Gestor "))
				{
					DrawGestorSystem();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("  Log  "))
				{
					DrawLogSystem();
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	return ret;
}

void EconomyScene::DrawGestorSystem()
{

	/*ImGui::Text(openFileName.c_str());
	AddSeparator(1);*/
	AddSpacing(2);

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

		if (showContainerType)
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
			if (ImGui::MenuItem("Process"))
			{
				int dif = 1;
				r->GetType() == ContainerType::FUTURE ? dif = 1 : dif = -1;
				float totalResult = totalContainer->GetMoney() + (r->GetMoney() * dif);
				if (totalResult >= 0)
				{
					*totalContainer->GetMoneyPtr() = totalResult;
					DeleteContainer(i);
					ImGui::EndPopup();
					ImGui::PopID();
					break;
				}
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
}

void EconomyScene::DrawLogSystem()
{

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
		//	CreateContainer(ContainerType::FILTER_SINGULAR);
		//	chrono.ChronoStop();
		//}
		//if (ImGui::BeginPopup("Filter Popup T"))
		//{
		//	if (ImGui::MenuItem("Singular"))
		//		CreateContainer(ContainerType::FILTER_SINGULAR);

		//	if (ImGui::MenuItem("Plural"))
		//		CreateContainer(ContainerType::FILTER_PLURAL);
		//	ImGui::EndPopup();
		//}*/

		if (ImGui::Button("FILTER"))
			CreateContainer(ContainerType::FILTER);

		if (ImGui::Button("LIMIT "))
			CreateContainer(ContainerType::LIMIT);

		if (ImGui::Button("FUTURE"))
			CreateContainer(ContainerType::FUTURE);

		if (ImGui::Button("ARREAR"))
			CreateContainer(ContainerType::ARREAR);
		
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
	totalContainer->SetCurrency(comboCurrency[currency]);
	unasignedContainer->SetCurrency(comboCurrency[currency]);
	for (Container* r : containers) r->SetCurrency(comboCurrency[currency]);
}

void EconomyScene::CreateContainer(ContainerType container, const char* name, bool hidden, bool open)
{
	bool unified = createContainerUnified;

	switch (container)
	{
	case ContainerType::FILTER: containers.push_back((Container*)(new FilterContainer(name, hidden, open, unified, totalContainer->GetMoneyPtr()))); break;
	case ContainerType::LIMIT : containers.push_back((Container*)(new  LimitContainer(name, hidden, open, unified, totalContainer->GetMoneyPtr()))); break;
	case ContainerType::FUTURE: containers.push_back((Container*)(new FutureContainer(name, hidden, open, unified, totalContainer->GetMoneyPtr()))); break;
	case ContainerType::ARREAR: containers.push_back((Container*)(new ArrearContainer(name, hidden, open, unified, totalContainer->GetMoneyPtr()))); break;
	default: break;
	}

	containers.back()->SetCurrency(comboCurrency[currency]);
}

void EconomyScene::SetMethod()
{
	float money = totalContainer->GetMoney();

	switch (method)
	{
	case Method::MTHD_PARETO:
	{
		CreateContainer(ContainerType::FILTER, "Pareto", false, false);
		FilterContainer* fC = (FilterContainer*)containers.back();
		fC->ClearLabels();
		fC->NewLabel("Available 80%", money * 0.8f);
		fC->NewLabel("Savings 20%", money * 0.2f);
		break;
	}

	case Method::MTHD_50_15_5:
	{
		CreateContainer(ContainerType::FILTER, "50 / 15 / 5", false, false);
		FilterContainer* fC = (FilterContainer*)containers.back();
		fC->ClearLabels();
		fC->NewLabel("Essential 50%", money * 0.5f);
		fC->NewLabel("Future 15%", money * 0.15f);
		fC->NewLabel("Unexpected 5%", money * 0.05f);
		fC->NewLabel("Free Assign 30%", money * 0.3f);
		break;
	}

	case Method::MTHD_50_30_20:
	{
		CreateContainer(ContainerType::FILTER, "50 / 30 / 20", false, false);
		FilterContainer* fC = (FilterContainer*)containers.back();
		fC->ClearLabels();
		fC->NewLabel("Primary 50%", money * 0.5f);
		fC->NewLabel("Leisure 30%", money * 0.3f);
		fC->NewLabel("Savings 20%", money * 0.2f);
		break;
	}

	case Method::MTHD_HARV_EKER:
	{
		CreateContainer(ContainerType::FILTER, "HARV EKER", false, false);
		FilterContainer* fC = (FilterContainer*)containers.back();
		fC->ClearLabels();
		fC->NewLabel("Primary 55%", money * 0.55f);
		fC->NewLabel("Education 10%", money * 0.1f);
		fC->NewLabel("Leisure 10%", money * 0.1f);
		fC->NewLabel("Donations 5%", money * 0.05f);
		fC->NewLabel("Invest 10%", money * 0.1f);
		fC->NewLabel("Savings 10%", money * 0.1f);
		break;
	}

	default:
		break;
	}

	method = Method::MTHD_NO;
}
