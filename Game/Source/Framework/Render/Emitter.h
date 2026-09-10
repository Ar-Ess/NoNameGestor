#pragma once

#include "Framework/Render/Texture.h"
#include "Framework/Render/EmitterData.h"
#include "Framework/Data/Vector.h"
#include "Framework/Data/Flag.h"
#include "Framework/Time/PerfTimer.h"
#include "Framework/Render/Particle.h"

class Emitter : public Texture
{
	friend class Particle;
	friend class AssetsManager;

public:

	struct Vortex
	{
		Vortex(Point position, float speed, float radius) :
			position(position), speed(speed), radius(radius)
		{}

		bool operator==(const Vortex& other) const
		{
			return position == other.position && speed == other.speed && radius == other.radius;
		}

		Point position;
		float speed;
		float radius;
	};

public:
	
	Emitter();
	Emitter(const Emitter& other);
	Emitter(Emitter&& other) noexcept;

	~Emitter() override;

	bool Draw(float dt) override;

	// Starts the emission of particles
	// time defines how long does it lasts
	// time = 0 means infinite
	void StartEmission(float time = 0.0f);
	// Pauses the emission of particles
	// time defines for how long does it pause the emission
	// time = 0 means infinite
	void PauseEmission(float time = 0.0f);
	// Resumes the emission if it was paused
	void ResumeEmission();
	// Stops the emission of particles
	// If "forceStop" is true, it will stop the emission immediately.
	// Otherwise, it will wait until all the particles are dead to stop the emission.
	void StopEmission(bool forceStop = false);
	
	// Returns the index of the vortex in the array
	// Returns -1 if the vortex was invalid and not added
	const Vortex& AddVortex(const Point& position, float speed, float radius);
	// Returns false if the index wasn't found
	bool RemoveVortex(const Vortex& vortex);

public: // Operators

	Emitter& operator=(const Emitter& other);

	Emitter& operator=(Emitter&& other) noexcept;

private:

	Emitter(TextureData* texture, const EmitterData& data);

	bool Process(float dt);

private:

	enum class EmitterState
	{
		STOPPED,
		PAUSED,
		STOPPING,
		PLAYING
	} state = EmitterState::STOPPED;

	enum class EmitterConfig
	{
		// There is a specified time until the particles stop emitting
		EMISSION_TIME_DEFINED,
		// There is a specified time until pause is resumed
		PAUSE_TIME_DEFINED,
		// A texture is used for the particles
		TEXTURE_DEFINED,
		// An animation is used for the particles
		ANIMATION_DEFINED,
	};

	// Emitter
	EmitterData data;

	// Pool
	Array<Particle> pool;
	Particle* firstAvailable = nullptr;

	// Emission time
	PerfTimer emissionTimer;
	PerfTimer pauseTimer;
	float emissionTime = 0;
	float pauseTime = 0;

	// Vortex
	Vector<Vortex> vortexs;

	// Config
	Flag config;

};