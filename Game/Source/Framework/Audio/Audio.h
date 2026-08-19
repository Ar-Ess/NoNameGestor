#pragma once

#include "Framework/Engine/Object.h"
#include "Framework/Assets/Asset.h"
#include "Framework/Utils/RandParam.h"
#include "Framework/Audio/Spatialization.h"
#include "Framework/Assets/AudioData.h"


namespace SoLoud 
{ class AudioSource; }

class Audio : public Object, public Asset<AudioData>
{

	friend class AssetsManager;
	friend class AudioSystem;

public:

	Audio();
	Audio(const Audio& other);
	Audio(Audio&& other) noexcept;

	virtual ~Audio() override;

	bool IsStreaming() const;

	bool IsPlayable() const;

	unsigned int SampleRate() const;

	float Duration() const;

	unsigned int SampleCount() const;

	virtual Point Size() const override;
	void Size(float w, float h) override;
	void Size(const Point& size) override;

public: // Operators

	Audio& operator=(const Audio& other);

	Audio& operator=(Audio&& other) noexcept;

private:

	Audio(AudioData* data);

	SoLoud::AudioSource* Internal() const;

	bool PauseOnPlay() const;

public:

	// From 0 to 1
	RandParam volume = 1;
	// From -1 to 1
	RandParam pan = 0;
	// From -48 to 48 (semitones)
	RandParam transpose = 0;
	// Start offset from 0 to 1 (0 = start, 1 = end)
	RandParam startOffset = 0;
	// From 0 to inf (seconds)
	float delay = 0;
	// Spatialize sound
	Spatialization spatialize = Spatialization::None;
	// Amount of loops from 0 to inf
	// Set 0 for infinite loop, set 1 to disable looping
	unsigned int loopCount = 1;

private:

	unsigned int audioHandle = 0;

};
