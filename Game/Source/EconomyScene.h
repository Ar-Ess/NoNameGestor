#ifndef __ECONOMY_SCENE_H__
#define __ECONOMY_SCENE_H__

#include <vector>
#include "imgui/imgui.h"
#include "Input.h"
#include "RecipientHeader.h"
#include "FileManager.h"

class EconomyScene
{
public:

	EconomyScene(Input* input);
	~EconomyScene();

	bool Start();
	bool Update();
	bool Draw();

	bool CleanUp();

private: // Functions

	void NewFile();
	void SaveAs();
	void Save();
	void InternalSave(const char* path);
	void Load();

	bool DrawMenuBar();
	bool DrawDocking();

	bool DrawPreferencesWindow(bool* open);
	bool DrawMainWindow(bool* open);
		bool DrawRecipient(Recipient* recipient, size_t size, suint i);

	bool DrawToolbarWindow(bool* open);

	void UpdateShortcuts();
	void UpdateCurrency();

	void CreateRecipient(RecipientType recipient, const char* name = "New Recipient", float money = 0.0f, bool hidden = false, bool open = false);

	// Input from 0 (smallest spacing) to whatever you need
	void AddSpacing(unsigned int spaces = 1)
	{
		short int plus = 0;
		if (spaces == 0) plus = 1;
		for (unsigned int i = 0; i < spaces * 2 + plus; i++) ImGui::Spacing();
	}

	// Input from 1 to whatever you need
	void AddSeparator(unsigned int separator = 1)
	{
		if (separator == 0) return;
		for (unsigned int i = 0; i < separator; i++) ImGui::Separator();
	}

	// Create helper pop up
	void AddHelper(const char* desc, const char* title = "(?)")
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

	void DeleteRecipient(suint index)
	{
		assert(index >= 0 && index < recipients.size());

		Recipient* r = recipients[index];
		recipients.erase(recipients.begin() + index);
		RELEASE(r);
	}

	void DeleteAllRecipients()
	{
		for (Recipient* r : recipients) RELEASE(r);
		recipients.clear();
		recipients.shrink_to_fit();
	}

	void MoveRecipient(suint index, suint position)
	{
		if (index == position) return;
		suint size = recipients.size();
		assert(index >= 0 && index < size);
		assert(position >= 0 && position < size);

		Recipient* r = recipients[index];
		recipients.erase(recipients.begin() + index);

		recipients.insert(recipients.begin() + position, r);
	}

	int ReturnRecipientIndex(intptr_t id)
	{
		size_t size = recipients.size();
		int i = 0;
		for (std::vector<Recipient*>::const_iterator it = recipients.begin(); it != recipients.end(); ++it)
		{
			Recipient* rTarget = (*it);
			if (id == rTarget->GetId()) return i;
			++i;
		}

		i = -1;
		assert(i != -1); // There is not a recipient like "r"

		return i;
	}

	//// Given an Image enum type, width and height, draw an image
	//void AddImage(Image img, unsigned int w = 0, unsigned int h = 0)
	//{
	//	int width = (int)img.GetImageDimensions().x;
	//	int height = (int)img.GetImageDimensions().y;
	//	if (w != 0 || h != 0)
	//	{
	//		width = w;
	//		height = h;
	//	}
	//	ImGui::Image(ImTextureID(img.GetTextureId()), ImVec2(float(width), float(height)));
	//}
	//void AddImage(Image img, int proportion = 100)
	//{
	//	int width = (int)img.GetImageDimensions().x;
	//	int height = (int)img.GetImageDimensions().y;
	//	if (proportion <= 0) proportion = 100;
	//	if (proportion != 100)
	//	{
	//		width = int(ceil((width * proportion) / 100));
	//		height = int(ceil((height * proportion) / 100));
	//	}
	//	ImGui::Image(ImTextureID(img.GetTextureId()), ImVec2(float(width), float(height)));
	//}
	//// Given an Image enum type, width and height, draw an image
	//bool AddImageButton(Image img, unsigned int w = 0, unsigned int h = 0)
	//{
	//	int width = (int)img.GetImageDimensions().x;
	//	int height = (int)img.GetImageDimensions().y;
	//	if (w != 0 || h != 0)
	//	{
	//		width = w;
	//		height = h;
	//	}
	//	if (ImGui::ImageButton(ImTextureID(img.GetTextureId()), ImVec2(float(width), float(height)), ImVec2(0, 0), ImVec2(1, 1), 0)) return true;
	//	return false;
	//}
	//bool AddImageButton(Image img, int proportion = 100)
	//{
	//	int width = (int)img.GetImageDimensions().x;
	//	int height = (int)img.GetImageDimensions().y;
	//	if (proportion <= 0) proportion = 100;
	//	if (proportion != 100)
	//	{
	//		width = int(ceil((width * proportion) / 100));
	//		height = int(ceil((height * proportion) / 100));
	//	}
	//	if (ImGui::ImageButton(ImTextureID(img.GetTextureId()), ImVec2(float(width), float(height)), ImVec2(0, 0), ImVec2(1, 1))) return true;
	//	return false;
	//}

public: // Variables

private: // Variables

	TotalMoneyRecipient* totalRecipient = nullptr;
	UnasignedMoneyRecipient* unasignedRecipient = nullptr;
	std::vector<Recipient*> recipients;

	Input* input = nullptr;
	FileManager* file = nullptr;

	bool demoWindow = false;
	bool preferencesWindow = false;

	// Shortcuts
	bool ctrl = false, shft = false, d = false, 
		 p    = false, s    = false, l = false,
		 n    = false;

	// Preferences
	bool showRecipientType = true;
	bool showFutureUnasigned = false;
	bool allowFutureCovering = false;
	bool showArrearUnasigned = false;
	bool allowArrearsFill = false;

	int currency = 0;
	const char* comboCurrency[5] = { "EUR", "USD", "COP", "ARS", "PEN"};

	// Save & Load
	bool saving = false, loading = false, savingAs = false;
	bool versionError = false;
	std::string openFileName;
	std::string openFilePath;

};

#endif //__ECONOMY_SCENE_H__
