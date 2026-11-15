#pragma once

#include "Framework/Shapes/Rect.h"
#include "Framework/Render/BlendMode.h"
#include "Framework/Render/Color.h"
#include "Framework/Utils/RandParam.h"
#include "Framework/Utils/Nullable.h"
#include "Framework/Render/Animation.h"

class AssetsManager;

struct EmitterData
{

	friend class AssetsManager;

public:

	static const EmitterData Fire();
	static const EmitterData FirePurple();
	static const EmitterData Flame();
	static const EmitterData Smoke();
	static const EmitterData SmokePixel();
	static const EmitterData Burst();
	static const EmitterData Wave1();
	static const EmitterData Wave2();
	static const EmitterData Bubble();
	static const EmitterData Spark();

	EmitterData(const Texture& texture, const RandParam& angleRange, const RandParam& rotSpeed,
		const RandParam& startSpeed, const RandParam& endSpeed, const RandParam& startSize, const RandParam& endSize,
		const RandParam& emitVariance, const RandParam& maxLife, const Rect& atlasMask,
		const Color& startColor, const Color& endColor, BlendMode blendMode);

	EmitterData(const Animation& animation, const RandParam& angleRange, const RandParam& rotSpeed,
		const RandParam& startSpeed, const RandParam& endSpeed, const RandParam& startSize, const RandParam& endSize,
		const RandParam& emitVariance, const RandParam& maxLife, const Color& startColor, const Color& endColor, BlendMode blendMode);

	EmitterData() = default;

	bool IsValid() const;

private:

	static Texture SampleTexture();
	
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
	Nullable<Animation> animation = nullptr;
	Nullable<Texture> texture = nullptr;

private:

	static AssetsManager* assets;

};
