#pragma once

#include "Framework/Shapes/Point.h"

struct Line
{
public:

	Line();
	Line(const Line& other) = default;
	Line(Line&& other) noexcept = default;
	Line(float x1, float y1, float x2, float y2);
	Line(const Point& start, float x2, float y2);
	Line(float x1, float y1, const Point& end);
	Line(const Point& start, const Point& end);

	Point Start() const;
	Point End() const;

	void Start(float x, float y);
	void End(float x, float y);

	void Start(const Point& start);
	void End(const Point& end);

	float Distance() const;

	float Soften(float value, float factor, const Line& line);

public: // Operators

	bool operator==(const Line& other) const;
	bool operator!=(const Line& other) const;

	Line operator+(const Point& point) const;
	Line operator-(const Point& point) const;

	Line operator+(float val) const;
	Line operator-(float val) const;
	Line operator*(float val) const;
	Line operator/(float val) const;

	void operator+=(const Point& point);
	void operator-=(const Point& point);
	void operator*=(const Point& point);
	void operator/=(const Point& point);

	void operator+=(float val);
	void operator-=(float val);
	void operator*=(float val);
	void operator/=(float val);

	Line operator-() const;

	Line& operator=(const Line& other) = default;

	Line& operator=(Line&& other) noexcept = default;

public:

	float x1 = 0, y1 = 0, x2 = 0, y2 = 0;
};
