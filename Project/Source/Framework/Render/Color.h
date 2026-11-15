#pragma once

struct Color
{
	Color();
	Color(unsigned int r, unsigned int g, unsigned int b, unsigned int a = 255);
	Color(const Color& other) = default;
	Color(Color&& other) noexcept = default;

	static const Color Red;
	static const Color Green;
	static const Color Blue;
	static const Color Black;
	static const Color White;
	static const Color Zero;

	static const Color RedAlpha(unsigned int apha);
	static const Color RedAlphaF(float apha);
	static const Color GreenAlpha(unsigned int apha);
	static const Color GreenAlphaF(float apha);
	static const Color BlueAlpha(unsigned int apha);
	static const Color BlueAlphaF(float apha);
	static const Color BlackAlpha(unsigned int apha);
	static const Color BlackAlphaF(float apha);
	static const Color WhiteAlpha(unsigned int apha);
	static const Color WhiteAlphaF(float apha);

	static const Color Grey(unsigned int intensity);
	static const Color GreyF(float intensity);
	static const Color GreyAlpha(unsigned int intensity, unsigned int alpha);
	static const Color GreyAlphaF(float intensity, float alpha);

	static const Color Lerp(Color a, Color b, float t);

public:

	Color& operator=(const Color&) = default;

	Color& operator=(Color&&) noexcept = default;

private:

	static const unsigned char To255(float value);

public:

	unsigned char r = 0, g = 0, b = 0, a = 0;

};