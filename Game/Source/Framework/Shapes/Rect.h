#pragma once

#include "Framework/Shapes/Point.h"
#include "Framework/Utils/Alignment.h"

class Rect
{
public:

	static const Rect Null;

	Rect() = default;
	Rect(const Rect& other) = default;
	Rect(Rect&& other) noexcept = default;
	Rect(float x, float y, float w, float h, Alignment align = Alignment::BOTTOM_LEFT);
	Rect(const Point& position, float w, float h, Alignment align = Alignment::BOTTOM_LEFT);
	Rect(float x, float y, const Point& size, Alignment align = Alignment::BOTTOM_LEFT);
	Rect(const Point& position, const Point& size, Alignment align = Alignment::BOTTOM_LEFT);

	void Position(const Point& position, Alignment align = Alignment::BOTTOM_LEFT);
	void Position(float x, float y, Alignment align = Alignment::BOTTOM_LEFT);

	void Move(float x, float y);
	
	void Move(const Point& move);

	void Size(const Point& size);

	void Size(float w, float h);

	Point Position(Alignment align = Alignment::BOTTOM_LEFT) const;

	Point Size() const;

	Point OffsetTo(Alignment align) const;
	Point OffsetFrom(Alignment align) const;
	Point Offset(Alignment from, Alignment to) const;

	float Area() const;

	// Returns if the rectancle does not exist (width = 0 ot height = 0)
	bool IsNull() const;

	// This function does not consider a collision if rects touches their bounds
	bool CheckCollision(const Rect& rect) const;
	static bool CheckCollision(const Rect& r1, const Rect& r2);
	bool CheckCollision(const Point& point) const;
	static bool CheckCollision(const Point& p, const Rect& r);

	Rect RectIntersection(const Rect& rect) const;
	static Rect RectIntersection(const Rect& r1, const Rect& r2);

	Rect BoundingRectangle(const Rect& rect) const;
	static Rect BoundingRectangle(const Rect& r1, const Rect& r2);

	// Returns the closest point to p in this rectangle perimeter
	Point ClosestRectPerimeterPoint(const Point& p) const;
	// Returns the closest point to p in rect perimeter
	static Point ClosestRectPerimeterPoint(const Point& p, const Rect& rect);

public: // Operators

	Rect& operator=(const Rect& other) = default;

	Rect& operator=(Rect&& other) noexcept = default;

private:

	static Point AlignmentFactor(Alignment align);

public:

	float x = 0, y = 0, w = 0, h = 0;
};