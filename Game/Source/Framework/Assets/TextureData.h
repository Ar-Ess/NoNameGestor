#pragma once

#include "Framework/Assets/AssetData.h"
#include "Framework/Shapes/Point.h"

struct SDL_Texture;

class TextureData : public AssetData<TextureData>
{
	friend class AssetsManager;
	friend class Texture;
	friend class Animation;

public:

	~TextureData() override;

private:

	TextureData(const ID& id, const String& path, SDL_Texture* texture, const Point& size);

protected:

	// Original Texture Size1
	const Point size = Point::Zero;
	// Texture Pointer
	SDL_Texture* texture = nullptr;

};