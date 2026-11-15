#pragma once

namespace Maths
{
	const float Epsilon = 1.19209290e-7f;
	const float Pi = 3.14159265359f;
	const float E = 2.718281828459045f;

	float Sqrt(float num);

	float Pow(float num, float power);

	float RadToDeg(float rad);

	float DegToRad(float deg);

	float SemitonesToRate(float semitones);

	float RateToSemitones(float rate);

	int Ceil(float num);

	int Floor(float num);

	bool Approximately(float a, float b);

	// Threshold must range between 0 and 1
	int Round(float num, float threshold);

	// Fixed threshold to 0.5, more optimized
	int Round(float num);

	float Sin(float num, bool radians = true);

	float Cos(float num, bool radians = true);

	float Distance(float x1, float y1, float x2, float y2);

	float Ln(float num);

	float Log(float num, float base = 10);

	// Converts a value from a linear range into a geometric (multiplicative/log-spaced) range.
	//
	// Typical use:
	// - Map linear controls into multiplicative domains.
	// - Examples:
	//   Slider 0..1 -> 20 Hz..20 kHz
	//   Slider 0..100 -> 1x..1000x zoom
	//   Linear parameter -> exponential growth scale
	//
	// Input:
	// - value in [minLinear .. maxLinear], distributed linearly.
	//
	// Output:
	// - Corresponding value in [minLog .. maxLog], distributed geometrically.
	//
	// Inverse of GeometricToLinear().
	float LinearToGeometric(float value, float minLinear, float maxLinear, float minLog, float maxLog);

	// Converts a value from a geometric (multiplicative/log-spaced) range into a linear range.
	//
	// Typical use:
	// - Convert multiplicative domains into linear positions.
	// - Examples:
	//   20 Hz -> 20 kHz frequency ranges
	//   1x -> 1000x zoom scales
	//   0.001 -> 1 scientific parameter ranges
	//
	// Input:
	// - value in [minLog .. maxLog], distributed geometrically.
	//
	// Output:
	// - Corresponding value in [minLinear .. maxLinear], distributed linearly.
	//
	// Inverse of LinearToGeometric().
	float GeometricToLinear(float value, float minLog, float maxLog, float minLinear, float maxLinear);

	// Converts a value from a normalized linear scale into a normalized logarithmic/perceptual scale.
	//
	// Typical use:
	// - Apply perceptual response to sliders (audio gain, brightness, mouse sensitivity).
	// - Example: a linear value 0.5 may map to ~0.8 on a perceptual/log scale.
	//
	// Input/Output:
	// - Input: value in [0..1], linearly distributed.
	// - Output: corresponding value in [0..1], logarithmically distributed.
	//
	// Inverse of LogToLinear().
	// 
	// Extended Input: curvature
	// Controls how "logaríthmic" is the curve.
	// 10 gives a veri similar-traditional log curve.
	float LinearToLog(float num, float minLinear, float maxLinear, float curvature = 10.0f);

	// Converts a value from a normalized logarithmic/perceptual scale into a normalized linear scale.
	//
	// Typical use:
	// - Audio/UI sliders where perception is logarithmic (volume, brightness, sensitivity).
	// - Example: a perceptual volume value 0.8 may map to ~0.5 linear.
	//
	// Input/Output:
	// - Input: value in [0..1], interpreted as logarithmically distributed.
	// - Output: corresponding value in [0..1], linearly distributed.
	//
	// Inverse of LinearToLog().
	// 
	// Extended Input: curvature
	// Controls how "logaríthmic" is the curve.
	// 10 gives a veri similar-traditional log curve.
	float LogToLinear(float value, float min, float max, float curvature = 10.0f);

	float Exp(float num);

	float Min(float a, float b);

	float Max(float a, float b);

	float Abs(float num);

	float Clamp(float value, float min, float max);

	float Clamp01(float value);

	// Given an "a" and "b" defined range, and a 0-1 ratio "t", return the expected value
	float Lerp(float a, float b, float t);

	// Given an "a" and "b" defined range, and a "c" value in that range, return the proportional 0-1 ratio value
	float LerpInverse(float a, float b, float c);

	bool NearlyEqual(float a, float b, float threshold = 0.001f);

};
