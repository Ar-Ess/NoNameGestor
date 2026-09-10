#pragma once

#include "Framework/Data/Vector.h"
#include "Framework/Engine/ID.h"
#include "Framework/Render/Color.h"

#include "NoNameGestor/Containers/ContainerEnum.h"
#include "NoNameGestor/Containers/Label.h"
#include "NoNameGestor/Gestor/Currency.h" //TODO: Use this class instead of format
#include "NoNameGestor/Utils/FileManager.h"

class Configuration;
class Coroutine;
struct Aggregate;

constexpr auto MAX_MONEY = 340282000000000000000000000000000000000.0f;

class Container
{
public: // Functions

	virtual ~Container();

	virtual void Awake() {}

	virtual bool Update(Aggregate& agg);

	bool DrawBase(float maxWidth, bool& erase, ID& move);

	virtual void Draw() {}

	virtual bool DrawExport() const;

	virtual void DrawCashFlow(float widthRatio, float initX) const {}

	virtual void Save(FileManager::FileNode node) const;

	std::string* CurrentName();

	const std::string* CurrentName() const;

	virtual const char* TypeName() const;

	virtual ContainerType Type() const;

	float Money() const;

	unsigned int Size() const;

	void Export(std::ofstream& exp) const;

protected: // Functions

	Container(const std::string& name, bool hidden, bool open, bool unified, String* format, Configuration* config);

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
	ID id;

protected: // Variables

	Vector<Label*, true> labels;
	float money = 0.0f;
	std::string name;
	String* format = nullptr;
	Configuration* config = nullptr;

	String message;
	Color color;
	Nullable<ID> labelDrawID;

	float maxWidth = 0;

};