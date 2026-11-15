#pragma once

#include "Framework/Assets/AssetData.h"
#include "Framework/Assets/AssetHandle.h"
#include "Framework/Engine/Object.h"

struct SDL_Texture;

class TextData : public AssetData<TextData>
{

	friend class Text;
	friend class Font;

private:

	struct TextLayer : public Object
	{
		TextLayer(const Rect& layerRect, SDL_Texture* texture) : Object(layerRect), texture(texture) {}

	public:

		SDL_Texture* texture = nullptr;
		Point outlineOffset;

		bool Exists() const
		{
			return texture != nullptr;
		};
	};

public:

	~TextData();

private:

	TextData(TextLayer normal, TextLayer outline, TextLayer shade);

private:

	TextLayer normal;
	TextLayer outline;
	TextLayer shade;

};

