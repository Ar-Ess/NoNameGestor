#include "EconomyScene.h"
#include <windows.h>
#include <iostream>
#include "Page.h"
#include "Framework/ID.h"
#include "External/ImGuiFileDialog/ImGuiFileDialog.h"
#include "imgui/imgui_internal.h"

#define VERSION "v1.0"

EconomyScene::EconomyScene()
{
	pages.Add(new Page());
	index = 0;

	openFileName = "New_File";
	openFileName += EXTENSION;
	openFilePath.clear();

}

EconomyScene::~EconomyScene()
{
}

bool EconomyScene::Start()
{
	pages.At(index)->Start(comboCurrency[currency]);

	return true;
}

bool EconomyScene::Update(float dt)
{
	pages.At(index)->Update();

	return true;
}

bool EconomyScene::Draw(float dt)
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

	return ret;
}

bool EconomyScene::CleanUp()
{
	pages.Clear();
	return true;
}

void EconomyScene::NewFile()
{
	Page* p = new Page();
	pages.Add(p);
	p->Start(comboCurrency[currency]);
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
			ImGui::Spacing();
			ImGui::Separator(2);
			ImGui::Spacing(1);
			ImGui::Text(description);
			ImGui::Spacing(2);
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

//void EconomyScene::ExportGestor(std::vector<Container*>* exporting)
//{
//	////std::fstream file;
//	////std::string filePath = openFilePath;
//	////if (filePath.empty()) filePath = "Exports\\";
//	////filePath += openFileName;
//	////filePath.erase(filePath.end() - 4, filePath.end());
//	////filePath += "_Gestor.txt";
//
//	////file.open(filePath, std::ios::out);
//
//	////assert(file.is_open()); // File is not open
//
//	////std::vector<Container*>::const_iterator it = exporting->begin();
//	////for (it; it != exporting->end(); ++it)
//	////{
//	////	file << (*it)->GetName() << ":";
//	////	if ((*it)->unified)
//	////	{
//	////		file << " " << (*it)->GetMoney() << " " << comboCurrency[currency] << std::endl << std::endl;
//	////	}
//	////	else
//	////	{
//	////		file << std::endl;
//	////		unsigned int size = (*it)->GetSize();
//	////		for (unsigned int i = 0; i < size; ++i)
//	////		{
//	////			file << " - " << (*it)->GetLabelName(i) << ": " << (*it)->GetLabelMoney(i) << " " << comboCurrency[currency] << std::endl << std::endl;
//	////		}
//	////	}
//	////}
//
//	////file.close();
//}

bool EconomyScene::DrawMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File", true))
		{
			if (ImGui::MenuItem("New File", "Ctrl + N"))
				NewFile();

			ImGui::Separator();

			//if (ImGui::MenuItem("Save", "Ctrl + S"))
			//	Save();

			//if (ImGui::MenuItem("Save As", "Ctrl + Shft + S"))
			//	SaveAs();

			//if (ImGui::MenuItem("Load", "Ctrl + L"))
			//	Load();

			ImGui::Separator();

			//if (ImGui::BeginMenu("Export"))
			//{
			//	if (ImGui::BeginMenu("Gestor"))
			//	{
			//		bool empty = containers.empty();
			//		bool selected = false;
			//		if (!empty)
			//		{
			//			ImGui::Text("Select the containers:");
			//			AddSpacing(1);
			//			for (Container* c : containers)
			//			{
			//				ImGui::Text("  - ");
			//				ImGui::SameLine();
			//				ImGui::PushID(c->id->Value());
			//				ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
			//				ImGui::MenuItem(c->GetName(), "", &c->exporting);
			//				ImGui::PopItemFlag();
			//				ImGui::PopID();
			//				if (!selected && c->exporting) selected = true;
			//			}
			//		}
			//		else
			//		{
			//			ImGui::TextDisabled("No containers yet:");
			//			AddSpacing(3);
			//		}
			//		AddSpacing(1);
			//		AddSeparator();
			//		if (!selected || empty) ImGui::BeginDisabled();
			//		if (ImGui::Selectable("  Export", false, ImGuiSelectableFlags_None, {70, 14}))
			//		{
			//			std::vector<Container*> toExport;
			//			for (Container* c : containers)
			//			{
			//				if (!c->exporting) continue;
			//				toExport.emplace_back(c);
			//			}
			//			ExportGestor(&toExport);
			//		}
			//		if (!selected && !empty) ImGui::EndDisabled();
			//		ImGui::SameLine();
			//		ImGui::Text("|");
			//		ImGui::SameLine();
			//		if (ImGui::Selectable("Export All", false, ImGuiSelectableFlags_None, { 70, 14 }))
			//			ExportGestor(&containers);
			//		if (empty) ImGui::EndDisabled();
			//		ImGui::EndMenu();
			//	}
			//	ImGui::EndMenu();
			//}

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
		//if (ImGui::BeginMenu("Create"))
		//{
		//	if (ImGui::MenuItem("Filter"))
		//		CreateContainer(ContainerType::FILTER);

		//	if (ImGui::MenuItem("Limit"))
		//		CreateContainer(ContainerType::LIMIT);

		//	if (ImGui::MenuItem("Future"))
		//		CreateContainer(ContainerType::FUTURE);

		//	if (ImGui::MenuItem("Arrear"))
		//		CreateContainer(ContainerType::ARREAR);

		//	if (ImGui::MenuItem("Const"))
		//		CreateContainer(ContainerType::CONSTANT);

		//	if (ImGui::MenuItem("Saving"))
		//		CreateContainer(ContainerType::SAVING);

		//	ImGui::EndMenu();
		//}
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
				ImGui::Helper("Shows, at the side of each container,\na text noting it's type.", "?"); ImGui::SameLine();
				ImGui::Checkbox("Show Container Typology Name", &showContainerType);

				ImGui::Helper("Shows the unsigned money in terms\nof future income.", "?"); ImGui::SameLine();
				ImGui::Checkbox("Show Unasigned Future Money ", &showFutureUnasigned);

				if (showFutureUnasigned)
				{
					ImGui::Dummy(ImVec2{ 6, 0 }); ImGui::SameLine();
					ImGui::Helper("Allows future money to cover\nactual money whenever it goes\nin negative numbers.\nIMPORTANT:\nUse this option if you know for sure\nyou'll receive the future income.", "?"); ImGui::SameLine();
					ImGui::Checkbox("Allow Future Money Covering ", &allowFutureCovering);
				}

				ImGui::Helper("Shows the unsigned money in terms\nof arrears outcome.", "?"); ImGui::SameLine();
				ImGui::Checkbox("Show Unasigned Arrears Money ", &showArrearUnasigned);

				if (showArrearUnasigned)
				{
					ImGui::Dummy(ImVec2{ 6, 0 }); ImGui::SameLine();
					ImGui::Helper("Allows unasigned actual money to fill\nunasigned arrears whenever they exist.", "?"); ImGui::SameLine();
					ImGui::Checkbox("Allow Arrears Money Filling", &allowArrearsFill);
				}

				ImGui::Helper("Shows the total money in terms\nof constant outcome.", "?"); ImGui::SameLine();
				ImGui::Checkbox("Show Total Constrant Money ", &showConstantTotal);

				ImGui::Helper("Enlarges the size of the text\nlabels of each container.", "?"); ImGui::SameLine();
				//ImGui::PushItemWidth(textFieldSize);
				//ImGui::DragFloat("Text Fiend Size", &textFieldSize, 0.1f, 1.0f, 1000.0f, "%f pts", ImGuiSliderFlags_AlwaysClamp);
				//ImGui::PopItemWidth();

				ImGui::Helper("Creates all containers unified by default", "?"); ImGui::SameLine();
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
		if (ImGui::BeginTabBar("##FilesBar"))
		{
			index = 0;
			unsigned int i = 0;
			pages.Iterate<unsigned int*, unsigned int*>
			(
				&index,
				&i,
				[](Page* p, unsigned int* index, unsigned int* i)
				{
					ImGui::PushID(p->id->Value());
					if (ImGui::BeginTabItem(p->name.c_str()))
					{
						p->Draw();
						*index = *i;
						ImGui::EndTabItem();
					}
					ImGui::PopID();
					(*i)++;
				}
			);

			ImGui::EndTabBar();
		}
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
		bool action = false;

		if (ImGui::Button("FILTER"))
		{
			//CreateContainer(ContainerType::FILTER);
			action = true;
		}

		if (ImGui::Button("LIMIT "))
		{
			//CreateContainer(ContainerType::LIMIT);
			action = true;
		}

		if (ImGui::Button("FUTURE"))
		{
			//CreateContainer(ContainerType::FUTURE);
			action = true;
		}

		if (ImGui::Button("ARREAR"))
		{
			//CreateContainer(ContainerType::ARREAR);
			action = true;
		}

		if (ImGui::Button("CONST "))
		{
			//CreateContainer(ContainerType::CONSTANT);
			action = true;
		}

		if (ImGui::Button("SAVING"))
		{
			//CreateContainer(ContainerType::SAVING);
			action = true;
		}

		//if (action) SwitchLoadOpen();
	}
	ImGui::End();

	return ret;
}

void EconomyScene::UpdateCurrency()
{
	//totalContainer->SetCurrency(comboCurrency[currency]);
	//unasignedContainer->SetCurrency(comboCurrency[currency]);
	//for (Container* r : containers) r->SetCurrency(comboCurrency[currency]);
}
