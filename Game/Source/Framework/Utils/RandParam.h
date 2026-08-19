#pragma once

#include "Framework/Utils/Random.h"
#include "Framework/Shapes/Point.h"

struct RandParam
{
public:

	enum class RandParamMode
	{
		JUST_VALUE,
		ADDITIVE,
		RELATIVE
	};

public:

	static RandParam Null;

	// Additive: Random will be generated between value + min and value + max
	static RandParam Additive(float min, float max, float initial = 0);
	// Additive: Random will be generated between value + min and value + max
	static RandParam Additive(const Point& minMax, float initial = 0);

	// Relative: Random will be generated between value * min and value * max
	static RandParam Relative(float min, float max, float initial = 1);
	// Relative: Random will be generated between value * min and value * max
	static RandParam Relative(const Point& minMax, float initial = 1);

	// Additive: Random will be generated between value + min and value + max
	static RandParam AdditiveInt(int min, int max, int initial = 0);
	// Additive: Random will be generated between value + min and value + max
	static RandParam AdditiveInt(const Point& minMax, int initial = 0);

	// Relative: Random will be generated between value * min and value * max
	static RandParam RelativeInt(int min, int max, int initial = 1);
	// Relative: Random will be generated between value * min and value * max
	static RandParam RelativeInt(const Point& minMax, int initial = 1);

	RandParam(float value);
	RandParam(const RandParam& other) = default;
	RandParam(RandParam&& other) = default;

	// Returns a random number
	float Random() const;

	// Returns a random number between min and max, excluding the initial value
	float Range() const;

	// Returns the minimum possible random value
	float Minimum() const;

	// Returns the maximum possible random value
	float Maximum() const;

public: // Operators

	RandParam& operator=(const RandParam& other);

	RandParam& operator=(RandParam&& other) noexcept;

	RandParam& operator=(float v);

	bool operator==(float value) const;

	bool operator!=(float value) const;

	// Is value bigger than the Maximum of this rand param?
	bool operator<(float value) const;

	// Is value smaller than the Minimum of this rand param?
	bool operator>(float value) const;

	// Is value bigger or equal to the Maximum of this rand param?
	bool operator<=(float value) const;

	// Is value bigger or equal to the Minimum of this rand param?
	bool operator>=(float value) const;

private:

	RandParam(float value, float min, float max, RandParamMode mode, bool asInt);

	RandParam() = delete;

	float RangeInternal() const;

public:

	const float value = 0;
	const float min = 0;
	const float max = 0;
	const RandParamMode mode;
	const bool asInt = false;
};