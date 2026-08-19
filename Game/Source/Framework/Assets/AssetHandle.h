#pragma once

template<class T>
class AssetHandle
{
public:

    AssetHandle(T* data)
        : data(data)
    {
    }

public:

	T* data;
	unsigned int instanceCount = 0;

};
