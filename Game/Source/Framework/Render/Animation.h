#pragma once

#include "Framework/Render/Texture.h"
#include "Framework/Render/SpriteSheet.h"

class Rect;

class Animation : public Texture
{

	friend class AssetsManager;

public:

	enum class Style 
	{
		STOPED,
		LOOP,
		STRAIGHT,
		PINGPONG,
		PINGPONG_NO_REPEAT
	};

public:

	Animation();
	Animation(const Animation& other);
	Animation(Animation&& other) noexcept;

	~Animation() override;
	
	bool Draw(float dt) override;

	float Duration() const;
	void Duration(float seconds);

	bool HasFinished() const;

	void Reset();

	void SetSprite(int index);
	void SetSprite(const Point& coords);

public: // Operators

	Animation& operator=(const Animation& other);

	Animation& operator=(Animation&& other) noexcept;

private:

	Animation(TextureData* texture, const SpriteSheet& sheet);

	const Rect& Process(float dt);

public:
	
	// Animation internal configuration
	SpriteSheet sheet;
	// Animation style
	Style style = Style::PINGPONG_NO_REPEAT;
	// Amount of iterations of the animation
	// 0 means infinite
	unsigned int times = 0;

private:

	float frameDuration = 0.1f;
	float time = 0.0f;

	Point fullSize = Point::Zero; // full texture size

	bool pingpong = false;
	unsigned int timesCount = 0;

};
