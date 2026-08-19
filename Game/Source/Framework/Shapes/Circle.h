#pragma once

#include "Framework/Shapes/Point.h"

struct Circle
{
public:

	Circle();

	Circle(const Circle& other) = default;

	Circle(Circle&& other) noexcept = default;

	Circle(float x, float y, float radius);

	Circle(const Point& pos, float radius);

	static const Circle Unitary;

	bool IsNull() const;

	Point Position() const;
	void Position(const Point& position);

	float Diameter() const;

public: // Operators

	Circle& operator=(const Circle& other) = default;

	Circle& operator=(Circle&& other) noexcept = default;

public:

	float x = 0, y = 0;
	float radius = 0.0f;
};
