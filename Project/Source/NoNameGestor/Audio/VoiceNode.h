#pragma once

#include "Framework/Engine/ID.h"
#include "NoNameGestor/Audio/DetailLevel.h"
#include <cstdint>
#include <string_view>
#include <string>
#include <vector>

class VoiceNode
{
public:

	VoiceNode(std::string&& name, std::string&& type) :
		name(std::move(name)), type(std::move(type)), id(ID::New())
	{ }
	virtual ~VoiceNode() = default;
	virtual std::vector<VoiceNode*> Children() const = 0;
	virtual std::string Describe(DetailLevel level) const = 0;
	virtual std::string Locate(DetailLevel level) const = 0;
	
	const std::string_view Type() const { return type; };
	const std::string_view Name() const { return name; };
	const ID Id() const { return id; };

	std::string name;

protected:

	std::string type;
	ID id;

};

