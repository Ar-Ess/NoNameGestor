#pragma once

#include "Framework/Engine/Module.h"

#include "Framework/Data/Map.h"
#include "Framework/Engine/ID.h"
#include "Framework/Render/Animation.h"
#include "Framework/Render/Emitter.h"
#include "Framework/Render/TileMap.h"
#include "Framework/Audio/Audio.h"
#include "Framework/Assets/Font.h" //TODO: Fonts should have a FontData as well to protect the original data instance!
#include "Framework/Assets/AudioData.h"
#include "Framework/Assets/TextureData.h"
#include "Framework/Assets/TileMapData.h"

class Render;
class BaseAssetData;
typedef struct SDL_RWops;

class AssetsManager : public Module<AssetsManager>
{
	friend class App;

public:

	static const String audioPath;
	static const String fontPath;
	static const String texturePath;
	static const String mapPath;

	static String GetFileName(const String& filePath, const char& separator = '\\');
	static const char* GetFileName(const char* filePath, const char& separator = '\\');
	static bool FileExists(const String& filePath);
	static bool FileExists(const char* filePath);

	// Audio must be found in Assets/Audio/
	static Audio GetAudio(const String& path, bool streaming = false);

	// Font must be found in Assets/Fonts/
	static Font* GetFont(const String& path);
	
	// Image must be found in Assets/Textures/
	static Texture GetTexture(const String& path);

	// Spritesheet must be found in Assets/Textures/
	static Animation GetAnimation(const String& path, const SpriteSheet& sheet);

	// Particle Atlas must be found in Assets/Textures/
	// Only for new EmitterData. Use EmitterData::New(...) here.
	static Emitter GetParticleEmitter(const String& path, const EmitterData& data);

	// Particle Atlas must be found in Assets/Textures/
	// Only for sample EmitterData. Do not use EmitterData::New(...) here.
	static Emitter GetParticleEmitter(const EmitterData& emitterSample);

	// Map must be found in Assets/Maps/
	// Currently supporting Tiled json generated maps.
	// All tilesets used in the map must be found under Assets/Textures
	static TileMap GetTileMap(const String& path);

	static bool FreeAudio(const Audio& audio);
	
	static bool FreeFont(const Font* font);

	static bool FreeTexture(const Texture& texture);

	static bool FreeAnimation(const Animation& animation);

	static bool FreeParticleEmitter(const Emitter& emitter);

private:

	// Mount is the name of the compressed file if running in release mode
	AssetsManager(const String& mount);
	
	virtual ~AssetsManager();

	bool Awake() override;

	bool CleanUp() override;

	AudioData* NewAudio(const ID& id, const String& path, bool streaming);

	Font* NewFont(const ID& id, const String& path);

	TextureData* NewTexture(const ID& id, const String& path);
	
	TileMapData* NewTileMap(const ID& id, const String& path);

	static const String GetFullPath(const String& path, const AssetType type);

	SDL_RWops* RetrieveRWops(const String& path, size_t& size);

private:

	Map<ID, BaseAssetData*, true> assets;
	const String mount = nullptr;

};
