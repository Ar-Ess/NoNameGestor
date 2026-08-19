#pragma once

#include "Framework/Engine/Object.h"
#include "Framework/Assets/VisualAsset.h"
#include "Framework/Assets/TileMapData.h"

class TileMap : public LogicObject, public VisualAsset<TileMapData>
{

	friend class AssetsManager;
	friend class Render;

public:

	TileMap();

	TileMap(const TileMap& other);

	TileMap(TileMap&& other) noexcept;

	virtual ~TileMap() override;

	bool Draw(float dt) override;

public: // Operators

	TileMap& operator=(const TileMap& other);

	TileMap& operator=(TileMap&& other) noexcept;

protected:

	TileMap(TileMapData* data);

	void AttachTileSetTextures();

};

