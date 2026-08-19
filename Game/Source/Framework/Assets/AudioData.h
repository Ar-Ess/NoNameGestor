#pragma once

#include "Framework/Assets/AssetData.h"
#include "Framework/Assets/AssetHandle.h"

namespace SoLoud {
	class AudioSource;
}

class AudioData : public AssetData<AudioData>
{
	friend class AssetsManager;
	friend class Audio;

public:

	~AudioData() override;

private:

	AudioData(const ID& id, const String& path, SoLoud::AudioSource* audio, bool isStreaming, unsigned int sampleCount);

protected:
	
	SoLoud::AudioSource* audio = nullptr;
	const bool isStreaming = true;
	const unsigned int sampleCount = 0;
	const unsigned int sampleRate = 0;

};
