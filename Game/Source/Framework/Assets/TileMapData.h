#pragma once

#include "Framework/Assets/AssetData.h"
#include "Framework/Assets/AssetHandle.h"
#include "Framework/Render/Texture.h"

class TileMapData : public AssetData<TileMapData>
{
	friend class AssetsManager;
	friend class TileMap;

private:

	enum class MapOrientation
	{
		ORTHOGONAL,
		ISOMETRIC,
		STAGGERED, // ISOMETRIC
		HEXAGONAL, // STAGGERED
		OBLIQUE
	};

	struct TileSet
	{
		TileSet() = default;
		TileSet(const String& name, int firstGID, const Point& tilesetSize, float margin, float spacing, const Texture& texture);

		// Name of the Tile Set
		String name = nullptr;

		// First Tile GID
		int firstGID = -1;

		// Last Tile GID
		int lastGID = -1;

		// The amount of tiles in the tileset
		Point size = Point::Zero;

		float margin = 0;
		float spacing = 0;

		Texture texture;
	};

	struct TileLayer
	{
		TileLayer() = default;
		TileLayer(const String& name, const Point& layerSize, const Array<unsigned int>& data, float opacity);

		// Name of the Tile Layer
		String name = nullptr;

		// The amount of tiles in the tileset
		Point layerSize = Point::Zero;

		// Data of the id of the tile to draw in each position
		Array<unsigned int> data = Array<unsigned int>();

		// Opacity of the drawing layer
		float opacity = 1;
	};

public:

	~TileMapData() override;

private:

	TileMapData(const ID& id, const String& path, const Point& mapSize, const Point& tileSize, const String& orientation, const Array<TileSet>& tilesets, const Array<TileLayer>& layers);

	MapOrientation ParseOrientation(const String& orientation);

protected:

	// The amount of tiles in the map
	const Point mapSize = Point::Zero;

	// Tile pixel size
	const Point tileSize = Point::Zero;

	// Tile Map orientation
	const MapOrientation orientation = MapOrientation::ORTHOGONAL;

	// Tilesets of the map
	Array<TileSet> tilesets = Array<TileSet>();

	// Layers of the map
	const Array<TileLayer> layers = Array<TileLayer>();

};

