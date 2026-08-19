#pragma once

#include "Framework/Assets/AssetData.h"
#include "Framework/Shapes/Point.h"
#include "Framework/Data/Map.h"
#include "Framework/Render/Text.h"
#include "Framework/Render/TextStyle.h"

struct TTF_Font;

class Font : public BaseAssetData
{
	friend class AssetsManager;

public:

	~Font() override;

	// Generate a text
	// Add a TextStyle to modify the text. Usage: { .size = 60, .color = Color::Red }
	Text Write(const String& text, const TextStyle& style = {}) const;

private:

	Font(const ID& id, const String& path, void* data, const int64_t& dataSize);

	TTF_Font* OfSize(unsigned int size, bool& newSize) const;

	bool Remove(unsigned int size) const;

private:

	mutable Map<unsigned int, TTF_Font*> fontCache;
	void* data = nullptr;
	const int64_t& size = 0;
};
