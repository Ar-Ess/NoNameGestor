#pragma once

#include "Framework/Engine/Module.h"
#include "Framework/Data/Vector.h"
#include "Framework/Audio/Audio.h" // Added to avoid adding it on scene cpps
#include "Framework/Audio/Mixer.h" // Added to avoid adding it on scene cpps

namespace SoLoud {
	class Soloud;
}

class AudioSystem : public Module<AudioSystem>
{
	friend class App;

	struct Audio3DOperation 
	{
		Audio3DOperation(const unsigned int handle, const float volume, const Spatialization& mode, const Object* object) :
			handle(handle), volume(volume), mode(mode), object(object) {}

		const unsigned int handle = 0;
		const float volume = 0;
		const Spatialization mode = Spatialization::None;
		const Object* object = nullptr;
		Point prevPos = Point::Zero;
		float prevVol = 0;
	};

	struct AudioLoopOperation
	{
		AudioLoopOperation(const unsigned int handle, const unsigned int loopCount) :
			handle(handle), loopCount(loopCount) {}

		const unsigned int handle = 0;
		const unsigned int loopCount = 0;
	};

public:

	static bool Play(Audio& audio, unsigned int mixerIndex = 0);

	static bool Stop(Audio& audio);

	static int CreateMixer(unsigned int parentMixerIndex = 0);

	// Master Mixer has index 0
	static void MixerVolume(float volume, unsigned int mixerIndex = 0);
	// Master Mixer has index 0
	static float MixerVolume(unsigned int mixerIndex = 0);

	// Master Mixer has index 0
	static void MixerPan(float pan, unsigned int mixerIndex = 0);
	// Master Mixer has index 0
	static float MixerPan(unsigned int mixerIndex = 0);

	static void Listener(const Object* listener);

	static const char* GetError(unsigned int result);

private:
	
	AudioSystem();
	
	virtual ~AudioSystem();
	
	bool Awake() override;
	
	bool Start() override;

	bool Update(float dt) override;
	
	bool CleanUp() override;

	float ComputeAttenuation(const Point& position, const Spatialization& mode, float baseVolume, const Point& listenerPosition) const;

private:

	SoLoud::Soloud* engine = nullptr;
	Object const* listener = nullptr;
	Vector<Mixer*, true> mixers;
	Vector<Audio3DOperation> ops3DAudios;
	Vector<AudioLoopOperation> opsLoopAudios;
	Point prevListenerPosition = Point::Zero;

};