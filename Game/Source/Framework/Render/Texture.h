#pragma once

#include "Framework/Engine/Object.h"
#include "Framework/Assets/VisualAsset.h"
#include "Framework/Assets/TextureData.h"

struct SDL_Texture;

class Texture : public Object, public VisualAsset<TextureData>
{

	friend class AssetsManager;
	friend class Render;

public:

	Texture();

	Texture(const Texture& other);

	Texture(Texture&& other) noexcept;

	virtual ~Texture() override;

	virtual bool Draw(float dt) override;

	Point Size() const override;
	void Size(float w, float h) override;
	void Size(const Point& size) override;

public: // Operators

	Texture& operator=(const Texture& other);

	Texture& operator=(Texture&& other) noexcept;

protected:

	Texture(TextureData* data);

};

