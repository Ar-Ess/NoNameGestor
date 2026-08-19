#pragma once

class Easing
{
public:

	enum class EaseType
	{
		CONSTANT,
		LINEAR,
		QUADRATIC,
		CUBIC,
		QUARTIC,
		QUINTIC,
		SINOIDAL,
		EXPONENTIAL,
		CIRCULAR,
		BACK,
		ELASTIC,
		BOUNCE
	};

	enum class EaseMode
	{
		IN_MODE,
		OUT_MODE,
		IN_OUT_MODE
	};

	// Returns the value of v on the type of ease selected
	static float Lerp(EaseType type, EaseMode mode, float start, float end, float t);

private:

	// Eases
	static double Constant        (double t, double d);
	static double LinearEaseIn    (double t, double d);
	static double LinearEaseOut   (double t, double d);
	static double LinearEaseInOut (double t, double d);
	static double QuadEaseIn      (double t, double d);
	static double QuadEaseOut     (double t, double d);
	static double QuadEaseInOut   (double t, double d);
	static double CubicEaseIn     (double t, double d);
	static double CubicEaseOut    (double t, double d);
	static double CubicEaseInOut  (double t, double d);
	static double QuartEaseIn     (double t, double d);
	static double QuartEaseOut    (double t, double d);
	static double QuartEaseInOut  (double t, double d);
	static double QuintEaseIn     (double t, double d);
	static double QuintEaseOut    (double t, double d);
	static double QuintEaseInOut  (double t, double d);
	static double SineEaseIn      (double t, double d);
	static double SineEaseOut     (double t, double d);
	static double SineEaseInOut   (double t, double d);
	static double ExpoEaseIn      (double t, double d);
	static double ExpoEaseOut     (double t, double d);
	static double ExpoEaseInOut   (double t, double d);
	static double CircEaseIn      (double t, double d);
	static double CircEaseOut     (double t, double d);
	static double CircEaseInOut   (double t, double d);
	static double BackEaseIn      (double t, double d);
	static double BackEaseOut     (double t, double d);
	static double BackEaseInOut   (double t, double d);
	static double ElasticEaseIn   (double t, double d);
	static double ElasticEaseOut  (double t, double d);
	static double ElasticEaseInOut(double t, double d);
	static double BounceEaseIn    (double t, double d);
	static double BounceEaseOut   (double t, double d);
	static double BounceEaseInOut (double t, double d);

private:

	float timer = 0;
	static constexpr double c1 = 1.70158;
	static constexpr double c2 = c1 * 1.525;
	static constexpr double c3 = c1 + 1;
	static constexpr double r = 7.5625;

};