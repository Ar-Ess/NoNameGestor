#pragma once

#include "Framework/Render/Color.h"
#include "Framework/Utils/RandParam.h"
#include "Framework/Data/String.h"
#include "Framework/Render/BlendMode.h"
#include "Framework/Shapes/Rect.h"

class AssetsManager;

struct EmitterData
{

	friend class AssetsManager;

public:

	static const EmitterData Fire;
	static const EmitterData FirePurple;
	static const EmitterData Flame;
	static const EmitterData Smoke;
	static const EmitterData SmokePixel;
	static const EmitterData Burst;
	static const EmitterData Wave1;
	static const EmitterData Wave2;
	static const EmitterData Bubble;
	static const EmitterData Spark;
	static const EmitterData New(
		const RandParam& angleRange, const RandParam& rotSpeed, const RandParam& startSpeed, const RandParam& endSpeed,
		const RandParam& startSize, const RandParam& endSize, const RandParam& emitVariance, const RandParam& maxLife,
		const Color& startColor, const Color& endColor);

public:

	EmitterData() = default;
	EmitterData(const EmitterData& other) = default;
	EmitterData(EmitterData&& other) noexcept = default;

private:

	EmitterData(const RandParam& angleRange, const RandParam& rotSpeed, const RandParam& startSpeed, const RandParam& endSpeed,
		const RandParam& startSize, const RandParam& endSize, const RandParam& emitVariance, const RandParam& maxLife,
		const Color& startColor, const Color& endColor, BlendMode mode, const Rect& mask, bool isSample = true);

public: // Operators

	EmitterData& operator=(const EmitterData& other) = default;
	EmitterData& operator=(EmitterData&& other) noexcept = default;
	
public:

	RandParam angleRange = RandParam::Null;
	RandParam rotSpeed = RandParam::Null;
	RandParam startSpeed = RandParam::Null;
	RandParam endSpeed = RandParam::Null;
	RandParam startSize = RandParam::Null;
	RandParam endSize = RandParam::Null;
	RandParam emitVariance = RandParam::Null;
	RandParam maxLife = RandParam::Null;
	Color startColor = Color::Zero;
	Color endColor = Color::Zero;
	BlendMode mode = BlendMode::INVALID;
	Rect mask = Rect::Null;

private:

	bool isSamle = true;
	static const String samplePath;

};
