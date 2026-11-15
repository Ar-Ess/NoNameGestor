#pragma once

#include "Framework/Shapes/Point.h"

struct SpriteSheet
{
	friend class Animation;

	enum class ReadDir
	{
		L_TO_R_DOWNWARDS,
		L_TO_R_UPWARDS,
		R_TO_L_DOWNWARDS,
		R_TO_L_UPWARDS,
		UP_TO_DOWN_LEFTWARDS,
		UP_TO_DOWN_RIGHTWARDS,
		DOWN_TO_UP_LEFTWARDS,
		DOWN_TO_UP_RIGHTWARDS
	};

public:

	SpriteSheet() = default;

	// frameMap: amount of frames in a row and column
	// frameAmount: amount of frames that the animation has, 0 means all frames.
	// startFrame: which frame does the animation start (previous frames will be excluded)
	// frameMargin: margins around each frame (can't be irregular)
	// readDirection: in which direction are the frames sequentially red
	// The spritesheet's frames must fit exactly with its width & height
	SpriteSheet(const Point& frameMap, const unsigned int frameAmount, const unsigned int startFrame = 0, const Point& frameMargin = Point::Zero, ReadDir readDirection = ReadDir::L_TO_R_DOWNWARDS);

	// frameMap: amount of frames in a row and column
	// frameMargin: margins around each frame (can't be irregular)
	// readDirection: in which direction are the frames sequentially red
	// The spritesheet's frames must fit exactly with its width & height
	SpriteSheet(const Point& frameMap, const Point& frameMargin = { 0, 0 }, ReadDir readDirection = ReadDir::L_TO_R_DOWNWARDS);

private:

	SpriteSheet(const SpriteSheet& other) = default;

	SpriteSheet(SpriteSheet&&) noexcept = default;

public: //Operator

	SpriteSheet& operator=(const SpriteSheet& other);

	SpriteSheet& operator=(SpriteSheet&& other) noexcept;

public:

	const unsigned int frameAmount = 0;
	const Point frameMap = Point::Zero;
	const unsigned int startFrame = 0;
	const Point frameMargin = Point::Zero;
	const ReadDir readDirection = ReadDir::L_TO_R_DOWNWARDS;

};
