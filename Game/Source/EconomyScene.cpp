#include "EconomyScene.h"
#include <windows.h>
#include <iostream>

#define VERSION "v1.1"
#define EXTENSION ".nng"
#include "External/ImGuiFileDialog/ImGuiFileDialog.h"
#include "imgui/imgui_internal.h"

EconomyScene::EconomyScene(Input* input)
{
	this->input = input;
	this->file = new FileManager(EXTENSION);

	totalContainer = new TotalContainer("Total Money", &logs, &dateFormatType, &maxLogs);
	unasignedContainer = new UnasignedContainer("Unasigned Money", &showFutureUnasigned, &allowFutureCovering, &showArrearUnasigned, &allowArrearsFill, &showConstantTotal);

	openFileName = "New_File";
	openFileName += EXTENSION;
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
	UpdateShortcuts();

	if ((int)method > 0) SetMethod();

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
	float constMoney = 0;

	for (Container* r : containers)
	{
		if (r->hidden) continue;

		r->Update();

		switch (r->GetType())
		{

		case ContainerType::FUTURE: futureMoney += r->GetMoney(); break;
		case ContainerType::ARREAR: arrearMoney -= r->GetMoney(); break;
		case ContainerType::CONSTANT:
			for (int i = 0; i < ((ConstContainer*)r)->GetSize(); ++i) constMoney += ((ConstContainer*)r)->GetLabelLimit(i);
		case ContainerType::FILTER:
		case ContainerType::LIMIT:
		case ContainerType::SAVING:
		default: totalMoney -= r->GetMoney(); break;
		}
	}

	unasignedContainer->SetMoney(totalMoney + futureMoney + arrearMoney, totalMoney, futureMoney, arrearMoney, constMoney);
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
	DeleteAllLogs();

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
		Write("cnfSAU").Bool(showArrearUnasigned).
		Write("cnfAAF").Bool(allowArrearsFill).
		Write("cnfSCT").Bool(showConstantTotal).
		Write("cnfCCU").Bool(createContainerUnified).
		Write("cnfTFS").Number(textFieldSize).
		Write("cnfDFT").Bool(dateFormatType).
		Write("currency").Number(currency).
		Write("maxLogs").Number(maxLogs).
		// Generic File
		Write("containers").Number(total).
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
		case ContainerType::CONSTANT:
		{
			ConstContainer* cC = (ConstContainer*)r;
			int size = cC->GetSize();
			file->EditFile(path).
				Write("size").Number(size);

			for (int i = 0; i < size; ++i)
			{
				file->EditFile(path)
					.Write("name").String(cC->GetLabelName(i))
					.Write("limit").Number(cC->GetLabelLimit(i))
					.Write("money").Number(cC->GetLabelMoney(i));
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
					.Write("name").String(lPR->GetLabelName(i))
					.Write("limit").Number(lPR->GetLabelLimit(i))
					.Write("money").Number(lPR->GetLabelMoney(i));
			}
			break;
		}
		case ContainerType::FILTER:
		case ContainerType::FUTURE:
		case ContainerType::ARREAR:
		case ContainerType::SAVING:
		{
			Container* c = r;
			int size = c->GetSize();
			file->EditFile(path).
				Write("size").Number(size);

			for (int i = 0; i < size; ++i)
			{
				file->EditFile(path)
					.Write("name").String(c->GetLabelName(i))
					.Write("money").Number(c->GetLabelMoney(i));
			}
			break;
		}

		default:
			break;
		}
	}

	file->EditFile(path).
		Write("logs").Number((int)logs.size());

	for (Log* l : logs)
	{
		switch (l->GetType())
		{
		case LogType::MOVEMENT_LOG:
		{
			MovementLog* m = (MovementLog*)l;
			file->EditFile(path).
				Write("name").String(m->GetName()).
				Write("type").Number((int)m->GetType()).
				Write("money").Number(m->money).
				Write("t_money").Number(m->totalInstance).
				Write("date").Date(m->GetDate(0), m->GetDate(1), m->GetDate(2));

			break;
		}
		case LogType::INFORMATIVE_LOG:
		{
			InformativeLog* i = (InformativeLog*)l;
			file->EditFile(path).
				Write("name").String(i->GetName()).
				Write("type").Number((int)i->GetType()).
				Write("old_money").Number(i->totalInstance).
				Write("new_money").Number(i->newInstance);

			break;
		}
		}
	}
}

void EconomyScene::Load()
{
	// Load Logic
	if (!loading)
	{
		loading = true;
		return;
	}

	// Draw File Dialog
	std::string path, name;
	size_t format = 0;
	bool closed = false;
	if (!DrawFileDialog(&versionError, VERSION, &path, &name, &format, &closed)) return;
	else
	{
		if (closed)
		{
			loading = false;
			return; // Return true
		}
	}

	// Version Error Popup
	bool vErr = versionError;
	if (ErrorPopup(&versionError,
		"File version different from program's version",
		"SOLUTION:\nRead the 'ReadMe.md' about how to load\nolder/newer files (Save & Load section).")) loading = false;
	if (vErr) return;

	// Load
	loading = false;

	openFileName = name;
	openFilePath = path;

	path += name;
	path.erase(path.end() - format, path.end());

	LoadInternal(path.c_str());
}

void EconomyScene::LoadInternal(const char* path)
{
	float total = 0;
	int size = 0;

	DeleteAllContainer();
	DeleteAllLogs();

	// Y aspects
	file->ViewFile(path, 1).
		// Preferences
	    //Read("version")
		Read("cnfSRT").AsBool(showContainerType).
		Read("cnfSFU").AsBool(showFutureUnasigned).
		Read("cnfAFC").AsBool(allowFutureCovering).
		Read("cnfSAU").AsBool(showArrearUnasigned).
		Read("cnfAAF").AsBool(allowArrearsFill).
		Read("cnfSCT").AsBool(showConstantTotal).
		Read("cnfCCU").AsBool(createContainerUnified).
		Read("cnfTFS").AsFloat(textFieldSize).
		Read("cnfDFT").AsBool(dateFormatType).
		Read("currency").AsInt(currency).
		Read("maxLogs").AsInt(maxLogs).
		// General Project
		Read("containers").AsFloat(total).
		Read("size").AsInt(size);

	const int yAspects = 14; // Update if more preferences added
	const int xAspects = 6; // Update if more properties added (X aspects below)
	int added = 0;

	for (unsigned int i = 0; i < size; ++i)
	{
		int positionToRead = (i * xAspects) + yAspects + added;
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

		const int xAspects = 6; // Update if more properties added

		switch ((ContainerType)type)
		{
		case ContainerType::CONSTANT:
		{
			CreateContainer((ContainerType)type, name.c_str(), hidden, open);
			ConstContainer* cC = (ConstContainer*)containers.back();
			cC->ClearLabels();
			cC->unified = unified;
			cC->loadOpen = true;

			int lSize = 0;
			int futurePositionToRead = positionToRead + xAspects;

			file->ViewFile(path, futurePositionToRead).
				Read("size").AsInt(lSize);

			added++;

			for (suint i = 0; i < lSize; ++i)
			{
				std::string cName;
				float cMoney, cLimit;  //Change depending on "vars top"\/
				file->ViewFile(path, (futurePositionToRead + 1) + (i * 3)).
					Read("name").AsString(cName). // "Vars on top"
					Read("limit").AsFloat(cLimit).
					Read("money").AsFloat(cMoney);

				cC->NewLabel(cName.c_str(), cMoney, cLimit);

				added += 3; // Change depending on how many "vars on top"
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

			int lSize = 0;
			int futurePositionToRead = positionToRead + xAspects;

			file->ViewFile(path, futurePositionToRead).
				Read("size").AsInt(lSize);

			added++;

			for (suint i = 0; i < lSize; ++i)
			{
				std::string lName;
				float lMoney, lLimit; //     Change depending on "vars on top" \/
				file->ViewFile(path, (futurePositionToRead + 1) + (i * 3)).
					Read("name").AsString(lName). // "Vars on top"
					Read("limit").AsFloat(lLimit).
					Read("money").AsFloat(lMoney);

				lC->NewLabel(lName.c_str(), lMoney, lLimit);

				added += 3; // Change depending on how many "vars on top"
			}
			break;
		}
		case ContainerType::FILTER:
		case ContainerType::FUTURE:
		case ContainerType::ARREAR:
		case ContainerType::SAVING:
		{
			CreateContainer((ContainerType)type, name.c_str(), hidden, open);
			Container* c = containers.back();
			c->ClearLabels();
			c->unified = unified;
			c->loadOpen = true;

			int fSize = 0;
			int futurePositionToRead = positionToRead + xAspects;

			file->ViewFile(path, futurePositionToRead).
				Read("size").AsInt(fSize);

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
			}

			break;
		}
		default: break;
		}

	}

	int movSize = 0;
	int movPos = file->ViewFile(path, yAspects).Search("logs");
	added = 0;

	file->ViewFile(path, movPos)
		.Read("logs").AsInt(movSize);

	++added;

	for (unsigned int i = 0; i < movSize; ++i)
	{
		std::string name;
		float money = 0;
		float newMoney = 0;
		int movType = 0;
		float totalMoneyInstance = 0;
		int day = 0, month = 0, year = 0;

		file->ViewFile(path, movPos + added).
			Read("name").AsString(name).
			Read("type").AsInt(movType);

		added += 2;

		switch ((LogType)movType)
		{
		case LogType::MOVEMENT_LOG:
		{
			file->ViewFile(path, movPos + added).
				Read("money").AsFloat(money).
				Read("t_money").AsFloat(totalMoneyInstance).
				Read("date").AsDate(day, month, year);

			logs.emplace_back(new MovementLog(money, totalMoneyInstance, name.c_str(), &dateFormatType));
			if (day > 0) logs.back()->SetDate(day, month, year);

			added += 3;
			break;
		}
		case LogType::INFORMATIVE_LOG:
		{
			file->ViewFile(path, movPos + added).
				Read("old_money").AsFloat(money).
				Read("new_money").AsFloat(newMoney);

			logs.emplace_back(new InformativeLog(money, newMoney, name.c_str()));

			added += 2;
			break;
		}
		}
	}

	totalContainer->SetMoney(total);
	UpdateCurrency();
}

bool EconomyScene::DrawFileDialog(bool* vError, const char* v, std::string* path, std::string* name, size_t* format, bool* closed)
{
	if (!*vError)
	{
		std::string version;
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose a path", ".nng", ".");
		if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
		{
			// action if OK
			if (ImGuiFileDialog::Instance()->IsOk() == true && file->Exists(ImGuiFileDialog::Instance()->GetFilePathName().c_str(), false))
			{
				*path = ImGuiFileDialog::Instance()->GetCurrentPath() + "\\";
				*name = ImGuiFileDialog::Instance()->GetCurrentFileName();
				*format = ImGuiFileDialog::Instance()->GetCurrentFilter().size();
				ImGuiFileDialog::Instance()->Close();

				// Check if the version file is the same as the program version
				std::string checkPath = *path;
				checkPath += *name;
				checkPath.erase(checkPath.end() - *format, checkPath.end());
				file->ViewFile(checkPath.c_str()).
					Read("version").AsString(version);
				*vError = !SameString(v, version);
				return true;
			}
			else
			{
				ImGuiFileDialog::Instance()->Close();
				*closed = true;
				return true; // Return true
			}
		}
		else return false; // Return false
	}
}

bool EconomyScene::ErrorPopup(bool* open, const char* title, const char* description)
{
	if (*open)
	{
		ImGui::OpenPopup("Error");
		ImGui::SetNextWindowPos(ImVec2((1280 / 2) - 180, (720 / 2) - 90));
		ImGui::SetNextWindowSize(ImVec2(360, 180));
		if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Text(title);
			AddSpacing();
			AddSeparator(2);
			AddSpacing(1);
			ImGui::Text(description);
			AddSpacing(2);
			if (ImGui::Button("Documentation"))
			{
				ShellExecute(NULL, NULL, "https://github.com/Ar-Ess/NoNameGestor/blob/main/README.md", NULL, NULL, SW_SHOWNORMAL);
				ImGui::EndPopup();
				*open = false;
				return true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Okey"))
			{
				ImGui::EndPopup();
				*open = false;
				return true;
			}
		}
		ImGui::EndPopup();
		return false;
	}

	return false;
}

void EconomyScene::ExportGestor(std::vector<Container*>* exporting)
{
	std::fstream file;
	std::string filePath = openFilePath;
	if (filePath.empty()) filePath = "Exports\\";
	filePath += openFileName;
	filePath.erase(filePath.end() - 4, filePath.end());
	filePath += "_Gestor.txt";

	file.open(filePath, std::ios::out);

	assert(file.is_open()); // File is not open

	std::vector<Container*>::const_iterator it = exporting->begin();
	for (it; it != exporting->end(); ++it)
	{
		file << (*it)->GetName() << ":";
		if ((*it)->unified)
		{
			file << " " << (*it)->GetMoney() << " " << comboCurrency[currency] << std::endl << std::endl;
		}
		else
		{
			file << std::endl;
			unsigned int size = (*it)->GetSize();
			for (unsigned int i = 0; i < size; ++i)
			{
				file << " - " << (*it)->GetLabelName(i) << ": " << (*it)->GetLabelMoney(i) << " " << comboCurrency[currency] << std::endl << std::endl;
			}
		}
	}

	file.close();
}

void EconomyScene::ExportLogs(uint start, uint end)
{
	std::fstream file;
	std::string filePath = openFilePath;
	if (filePath.empty()) filePath = "Exports\\";
	filePath += openFileName;
	filePath.erase(filePath.end() - 4, filePath.end());
	filePath += "_Logs.txt";

	file.open(filePath, std::ios::out);

	assert(file.is_open()); // File is not open

	for (uint i = start; i != end + 1; ++i)
	{
		Log* log = logs[i];
		file << log->GetName() << ": ";
		char a = '+', b = '-';
		char* sign = nullptr;
		log->totalInstance > 0 ? sign = &a : sign = &b;

		if (log->GetDate(0) != 0)
			file << log->GetDate(0) << " / " << log->GetDate(1) << " / " << log->GetDate(2) << std::endl;
		else
			file << std::endl;

		file << log->GetOldInstance() << " -> " << log->GetNewInstance() << " | " << *sign << log->totalInstance << std::endl << std::endl;
	}

	file.close();
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

			if (ImGui::BeginMenu("Export"))
			{
				if (ImGui::BeginMenu("Gestor"))
				{
					bool empty = containers.empty();
					bool selected = false;
					if (!empty)
					{
						ImGui::Text("Select the containers:");
						AddSpacing(1);
						for (Container* c : containers)
						{
							ImGui::Text("  - ");
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

					if (ImGui::Selectable("  Export", false, ImGuiSelectableFlags_None, {70, 14}))
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
				
				if (ImGui::BeginMenu("Logs"))
				{
					size_t size = logs.size();
					static int from = 1, to = 1;
					
					bool empty = (size == 0);
					if (!empty)
					{
						ImGui::Text("Set a range:");
						AddSpacing(0);

						ImGui::Dummy({ 6, 0 });
						ImGui::SameLine();
						ImGui::Text("From:");
						ImGui::SameLine();
						ImGui::Dummy({4, 0});
						ImGui::SameLine();
						ImGui::Text("To:");

						ImGui::PushItemWidth(40);
						ImGui::Dummy({6, 0});
						ImGui::SameLine();
						ImGui::DragInt("##FromDrag", &from, (size + 1) / 100, 1, size );
						if (ImGui::IsItemEdited()) if (from > to) to = from;
						ImGui::SameLine();
						ImGui::Dummy({ 4, 0 });
						ImGui::SameLine();
						ImGui::DragInt("##ToDrag", &to, (size+1) / 100, 1, size );
						if (ImGui::IsItemEdited()) if (from > to) from = to;
						ImGui::PopItemWidth();

					}
					else
					{
						ImGui::TextDisabled("No logs yet:");
						AddSpacing(3);
					}
					
					AddSpacing(1);
					AddSeparator();
					if (empty) ImGui::BeginDisabled();
					if (ImGui::Selectable("  Export", false, ImGuiSelectableFlags_None, { 70, 14 }))
					{
						ExportLogs(from - 1, to - 1);
						from = 1;
						to = size;
					}
					ImGui::SameLine();
					ImGui::Text("|");
					ImGui::SameLine();
					if (ImGui::Selectable("Export All", false, ImGuiSelectableFlags_None, { 70, 14 }))
					{
						ExportLogs(0, size - 1);
						from = 1;
						to = size;
					}
					if (empty) ImGui::EndDisabled();

					ImGui::EndMenu();
				}

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

			if (ImGui::MenuItem("Const"))
				CreateContainer(ContainerType::CONSTANT);

			if (ImGui::MenuItem("Saving"))
				CreateContainer(ContainerType::SAVING);

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

	ImGuiDockNodeFlags dockspace_flags = (ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoResize);
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
		ImGui::Spacing();

		ImGui::Text("Currency:");
		if (ImGui::Combo("##Currency", &currency, comboCurrency, 5))
			UpdateCurrency();

		ImGui::Spacing();

		if (ImGui::BeginTabBar("##PreferencesTabBar"))
		{
			if (ImGui::BeginTabItem("Gestor"))
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

				AddHelper("Shows the total money in terms\nof constant outcome.", "?"); ImGui::SameLine();
				ImGui::Checkbox("Show Total Constrant Money ", &showConstantTotal);

				AddHelper("Enlarges the size of the text\nlabels of each container.", "?"); ImGui::SameLine();
				ImGui::PushItemWidth(textFieldSize);
				ImGui::DragFloat("Text Fiend Size", &textFieldSize, 0.1f, 1.0f, 1000.0f, "%f pts", ImGuiSliderFlags_AlwaysClamp);
				ImGui::PopItemWidth();

				AddHelper("Creates all containers unified by default", "?"); ImGui::SameLine();
				ImGui::Checkbox("Create Container Unified Default", &createContainerUnified);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(" Log  "))
			{
				ImGui::Spacing();
				ImGui::Text("Date Format:");
				static bool otherDate = !dateFormatType;
				if (ImGui::Checkbox("d/m/y", &dateFormatType))
				{
					dateFormatType = true;
					otherDate = false;
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("m/d/y", &otherDate))
				{
					dateFormatType = false;
					otherDate = true;
				}

				ImGui::Spacing();

				ImGui::Text("Maximum Ammount of Logs:");
				ImGui::PushItemWidth(180);
				ImGui::SliderInt("##MALSlider", &maxLogs, 5, 40, "%d Logs", ImGuiSliderFlags_AlwaysClamp);
				ImGui::PopItemWidth();
				ImGui::Text("5                       40");

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

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
		ImGui::Text(openFileName.c_str());
		AddSeparator(1);
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
				DrawLogSystem(ImGui::IsItemDeactivated());
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	return ret;
}

void EconomyScene::DrawGestorSystem()
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
			if (r->GetType() != ContainerType::CONSTANT && ImGui::MenuItem("Process"))
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

void EconomyScene::DrawLogSystem(bool checkMismatch)
{
	if (checkMismatch) CheckLogMismatch();

	AddSpacing(1);

	int logsLeft = maxLogs - logs.size();
	if (logsLeft < 5 && logsLeft > 0)
	{
		ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 1.0f, 0.4f }, "Warning! You are %d logs from the maximum log size.", logsLeft); ImGui::SameLine();
		AddHelper("Overcoming this maximum will delete automatically\nthe last log. Upgrade the maximum log amount\nor export the logs in a file.\n[File > Export > Logs]");
	}
	else if (logsLeft == 0)
	{
		ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 1.0f, 0.4f }, "Warning! You are %d logs from the maximum log size.", logsLeft); ImGui::SameLine();
		AddHelper("Overcoming this maximum will delete automatically\nthe last log. Upgrade the maximum log amount\nor export the logs in a file.\n[File > Export > Logs]");
	}
	
	AddSpacing(2);
	AddSeparator();

	std::vector<Log*>::reverse_iterator it;
	int i = 0;
	for (it = logs.rbegin(); it != logs.rend(); ++it) (*it)->Draw(comboCurrency[currency], ++i);

	AddSeparator();
}

bool EconomyScene::DrawToolbarWindow(bool* open)
{
	bool ret = true;
	if (!(*open)) return ret;

	if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		bool action = false;

		if (ImGui::Button("FILTER"))
		{
			CreateContainer(ContainerType::FILTER);
			action = true;
		}

		if (ImGui::Button("LIMIT "))
		{
			CreateContainer(ContainerType::LIMIT);
			action = true;
		}

		if (ImGui::Button("FUTURE"))
		{
			CreateContainer(ContainerType::FUTURE);
			action = true;
		}

		if (ImGui::Button("ARREAR"))
		{
			CreateContainer(ContainerType::ARREAR);
			action = true;
		}

		if (ImGui::Button("CONST "))
		{
			CreateContainer(ContainerType::CONSTANT);
			action = true;
		}

		if (ImGui::Button("SAVING"))
		{
			CreateContainer(ContainerType::SAVING);
			action = true;
		}

		if (action) SwitchLoadOpen();
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
	case ContainerType::FILTER  : containers.emplace_back((Container*)(new FilterContainer(name, hidden, open, unified, totalContainer->GetMoneyPtr()))); break;
	case ContainerType::LIMIT   : containers.emplace_back((Container*)(new  LimitContainer(name, hidden, open, unified, totalContainer->GetMoneyPtr()))); break;
	case ContainerType::FUTURE  : containers.emplace_back((Container*)(new FutureContainer(name, hidden, open, unified, totalContainer->GetMoneyPtr()))); break;
	case ContainerType::ARREAR  : containers.emplace_back((Container*)(new ArrearContainer(name, hidden, open, unified, totalContainer->GetMoneyPtr()))); break;
	case ContainerType::CONSTANT: containers.emplace_back((Container*)(new  ConstContainer(name, hidden, open, unified, totalContainer->GetMoneyPtr()))); break;
	case ContainerType::SAVING  : containers.emplace_back((Container*)(new SavingContainer(name, hidden, open, unified, totalContainer->GetMoneyPtr()))); break;
	default: break;
	}

	containers.back()->SetCurrency(comboCurrency[currency]);
}

void EconomyScene::CheckLogMismatch()
{
	Log* prev = nullptr;
	bool first = true;
	size_t size = logs.size();
	for (unsigned int i = 0; i < size; ++i)
	{
		if (first)
		{
			prev = logs[i];
			first = false;
			continue;
		}
		Log* curr = logs[i];

		if ((curr->GetOldInstance() - prev->GetNewInstance()) > 0.001f)
		{
			logs.emplace(logs.begin() + i, new InformativeLog(prev->GetNewInstance(), curr->GetOldInstance(), "Unlogged movement"));
			CheckLogLeaking();
		}

		prev = logs[i];
	}

	if (!logs.empty() && (logs.back()->GetNewInstance() - totalContainer->GetMoney()) > 0.001f)
	{
		CreateInformative(logs.back()->GetNewInstance(), "Unlogged movement");
	}
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

// Old Loads -------------------------------------------
void EconomyScene::Loadv1_0()
{
	// Load Logic
	if (!loadingV1_0)
	{
		loadingV1_0 = true;
		return;
	}

	// Draw File Dialog
	std::string path, name;
	size_t format = 0;
	bool closed = false;
	if (!DrawFileDialog(&versionError, "v1.0", &path, &name, &format, &closed)) return;
	else
	{
		if (closed)
		{
			loadingV1_0 = false;
			return; // Return true
		}
	}

	// Version Error Popup
	bool vErr = versionError;
	if (ErrorPopup(&versionError,
		"A v1.0 Loader can only load v1.0 files",
		"SOLUTION:\nGo to File > Load. If an error occurs shows again,\nfollow the instructions of that error.\nThis method only loads v1.0 files.")) loadingV1_0 = false;
	if (vErr) return;

	// Load
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
