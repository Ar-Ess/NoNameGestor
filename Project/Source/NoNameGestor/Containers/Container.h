#pragma once

#include "Framework/Data/Vector.h"
#include "Framework/Render/Color.h"

#include "NoNameGestor/Audio/VoiceNode.h"
#include "NoNameGestor/Containers/ContainerEnum.h"
#include "NoNameGestor/Containers/Label.h"
#include "NoNameGestor/Gestor/Currency.h" //TODO: Use this class instead of format
#include "NoNameGestor/Utils/FileManager.h"

struct Configuration;
class Coroutine;
struct Aggregate;

constexpr auto MAX_MONEY = 340282000000000000000000000000000000000.0f;

class Container : public VoiceNode
{
public: // Functions

	virtual ~Container() override;

	std::vector<VoiceNode*> Children() const override;
	virtual std::string Describe(DetailLevel level) const override;
	virtual std::string Locate(DetailLevel level) const override;

	virtual void Awake() {}

	virtual bool Update(Aggregate& agg);

	void DrawBase(float maxWidth, bool& erase, ID& move);

	virtual void Draw() {}

	virtual bool DrawExport() const;

	virtual void DrawCashFlow(float widthRatio, float initX) const {}

	virtual void Save(FileManager::FileNode node) const;

	std::string* CurrentName();

	const std::string* CurrentName() const;

	virtual std::string TypeName() const;

	virtual ContainerType TypeEnum() const;

	float Money() const;

	unsigned int Size() const;

	void Export(std::ofstream& exp) const;

protected: // Functions

	Container(std::string&& name, bool hidden, bool open, bool unified, String* format, Configuration* config);

	Container(const FileManager::FileNode& node, String* format, Configuration* config);

	void NewLabel(Label* label);

	void DrawUserMessage(Label* label);

	void SetWarningMessage(Label* label, const char* message, float time = 12.f, float fade = 4.f);
	void SetErrorMessage(Label* label, const char* message, float time = 12.f, float fade = 4.f);

private:

	void SetMessage(Label* label, const char* message, float time, float fade, const Color& color);
	Coroutine RunMessage(float time, float fade);

public: // Variables

	static bool UpdateOpenState;
	
	bool hidden = false;
	bool unified = true;
	bool open = false;
	bool exporting = false;

protected: // Variables

	std::vector<Label*> labels;
	float money = 0.0f;
	String* format = nullptr;
	Configuration* config = nullptr;

	String message;
	Color color;
	Nullable<ID> labelDrawID;

	float maxWidth = 0;

};