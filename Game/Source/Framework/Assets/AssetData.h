#pragma once

#include "Framework/Engine/ID.h"
#include "Framework/Assets/AssetType.h"
#include "Framework/Data/String.h"
#include "Framework/Assets/AssetHandle.h"

class BaseAssetData
{
public:

	virtual ~BaseAssetData() = default;

protected:

	BaseAssetData(AssetType type, const ID& id, const String& path) :
		type(type), id(id), path(path)
	{
	}

public:

	// Asset Type
	const AssetType type;
	// Asset ID
	const ID id;
	// Path to the file
	const String path;

};

template<class T>
class AssetData : public BaseAssetData
{
public:

	virtual ~AssetData() override
	{
		handle->data = nullptr;
		if (handle->instanceCount == 0)
			delete handle;
	}

protected:

	AssetData(AssetType type, const ID& id, const String& path, T* data) :
		BaseAssetData(type, id, path), handle(new AssetHandle<T>(data))
	{
	}

protected:

	// Texture Handle
	AssetHandle<T>* handle = nullptr;

};