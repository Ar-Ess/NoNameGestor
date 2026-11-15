#pragma once

#include "Framework/Utils/Easing.h"

class Spatialization
{

	friend class AudioSystem;

public:

	static const Spatialization None;
	static const Spatialization Lineal(float min, float max, bool enablePan = true, bool attachToAudio = false, float dopplerFactor = 0.f);
	static const Spatialization Quadratic(float min, float max, bool enablePan = true, bool attachToAudio = false, float dopplerFactor = 0.f);
	static const Spatialization Cubic(float min, float max, bool enablePan = true, bool attachToAudio = false, float dopplerFactor = 0.f);
	static const Spatialization Sinoidal(float min, float max, bool enablePan = true, bool attachToAudio = false, float dopplerFactor = 0.f);
	static const Spatialization Exponential(float min, float max, bool enablePan = true, bool attachToAudio = false, float dopplerFactor = 0.f);
	static const Spatialization Constant(bool attachToAudio = false, float dopplerFactor = 0.f);

	const Spatialization& operator=(const Spatialization& other);

private:

	Spatialization() = default;
	Spatialization(Easing::EaseType type, const float min, const float max, const bool enablePan, const bool attachToAudio, const float dopplerFactor);

private:

	const Easing::EaseType type = Easing::EaseType::CONSTANT;
	const float minDistance = 0;
	const float maxDistance = 1;
	const float dopplerFactor = 0;
	const bool enabled = false;
	const bool enablePan = false;
	const bool attachToAudio = false;

};

