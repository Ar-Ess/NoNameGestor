#pragma once

#include "Framework/Assets/AssetHandle.h"
#include "Framework/Engine/ID.h"
#include "Framework/Data/String.h"
#include "Framework/Engine/Debug.h"

#define NULL_ASSET_ACCESS_ASSERT() Debug::Assert(IsValid(), "Accessing an invalid Asset!");
#define NULL_ASSET_BUILD_ASSERT(x) Debug::Assert(x.IsValid(), "Can't build an Asset from an invalid Asset!");

template<class T>
class Asset
{
public:

    Asset()
    {

    }

    Asset(AssetHandle<T>* handle) :
        handle(handle)
    {
        ++this->handle->instanceCount;
    }

    Asset(const Asset& other)
    {
        CopyHandle(other);
    }

    Asset(Asset&& other) noexcept
    {
        MoveHandle(std::move(other));
    }

    virtual ~Asset()
    {
        ReleaseHandle();
    }

    bool IsValid() const
    {
        return handle != nullptr && handle->data != nullptr;
    }

    const ID& DataID() const
    {
        return Data()->id;
    }

    String Path() const
    {
        return Data()->path;
    }

public:

    Asset& operator=(const Asset& other)
    {
        ReleaseHandle();
        CopyHandle(other);

        return *this;
    }

    Asset& operator=(Asset&& other) noexcept
    {
        ReleaseHandle();
        MoveHandle(std::move(other));

        return *this;
    }

protected:

    T* Data() const
    {
        NULL_ASSET_ACCESS_ASSERT();
        return handle->data;
    }

    void ReleaseHandle()
    {
        if (handle && --handle->instanceCount == 0)
            delete handle;

        handle = nullptr;
    }

    void CopyHandle(const Asset& other)
    {
        NULL_ASSET_BUILD_ASSERT(other);

        handle = other.handle;
        if (handle)
            ++handle->instanceCount;
    }

    void MoveHandle(Asset&& other)
    {
        NULL_ASSET_BUILD_ASSERT(other);
        handle = other.handle;
        other.handle = nullptr;
    }

protected:

	AssetHandle<T>* handle = nullptr;

};
