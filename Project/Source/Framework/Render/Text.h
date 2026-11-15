#pragma once

#include "Framework/Engine/Object.h"
#include "Framework/Assets/VisualAsset.h"
#include "Framework/Render/TextData.h"

class Text : public Object, public VisualAsset<TextData>
{
	friend class Font;

public:

	Text();

	Text(const Text& other);

	Text(Text&& other) noexcept;

	~Text() override;

	bool Draw(float dt) override;

	void ResetColor() override;

	// Reset to the original texture size
	void ResetSize();

	// Align the texture to the object size
	void Align(Alignment align);

public: // Operators

	Text& operator=(const Text& other);
	
	Text& operator=(Text&& other) noexcept;

private:

	Text(TextData* data, const Color& color, const Color& outlineColor, const Color& shadeColor);

public:

	// Text shade color - if text has shade
	Color shadeColor = Color::Black;
	// Text shade color - if text has outline
	Color outlineColor = Color::Black;

};

