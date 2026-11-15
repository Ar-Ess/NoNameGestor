#pragma once

struct Point
{
	Point() = default;
	Point(const Point& other) = default;
	Point(Point&& other) = default;
	Point(float x, float y);

	static const Point Zero;
	static const Point One;
	static const Point Up;
	static const Point Down;
	static const Point Left;
	static const Point Right;

	static Point Lerp(const Point& a, const Point& b, float t);

	float x = 0, y = 0;

	float DistanceTo(const Point& to) const;
	float DistanceTo(float x, float y) const;
	float DistanceFrom(const Point& to) const;
	float DistanceFrom(float x, float y) const;
	static float Distance(const Point& a, const Point& b);

	// Same as DistanceTo but without sqrt. Not exactly precise.
	float DistanceApproxTo(const Point& to) const;
	// Same as DistanceTo but without sqrt. Not exactly precise.
	float DistanceApproxTo(float x, float y) const;
	// Same as DistanceFrom but without sqrt. Not exactly precise.
	float DistanceApproxFrom(const Point& from) const;
	// Same as DistanceFrom but without sqrt. Not exactly precise.
	float DistanceApproxFrom(float x, float y) const;
	// Same as Distance but without sqrt. Not exactly precise.
	static float DistanceAprox(const Point& a, const Point& b);

	float Module() const;

	bool IsZero() const;

	// Negates the point internally, modifying its values
	void Negate();

	// Returns both coordinates are negative.
	bool IsNegative() const;
	// Returns one of the coordinates is negative.
	bool IsAnyNegative() const;

	// Function: It applies the number inputed to the actual Point value and the result is the output.
	// It does not modify the value stored in the class
	Point Apply(const Point& sum) const;
	Point Apply(float x, float y) const;

	Point Multiply(const Point& multi) const;
	Point Multiply(float x, float y) const;

	float Max() const;
	float MaxAbs() const;
	float Min() const;
	float MinAbs() const;

	static Point Min(const Point& a, const Point& b);
	static Point Max(const Point& a, const Point& b);

	static Point Clamp(const Point& point, const Point& minPoint, const Point& maxPoint);
	static Point Clamp00(const Point& point, const Point& maxPoint);

public: // Operators

	// Get operators
	float& operator[](int i) { return i == 0 ? x : y; }
	const float& operator[](int i) const { return i == 0 ? x : y; }

	// Conditional operators
	bool operator==(const Point& b) const;
	bool operator!=(const Point& b) const;
	bool operator< (const Point& b) const;
	bool operator<=(const Point& b) const;
	bool operator> (const Point& b) const;
	bool operator>=(const Point& b) const;

	// Modification operators
	void operator+=(const Point& i);
	void operator-=(const Point& i);
	void operator*=(const Point& i);
	void operator/=(const Point& i);

	void operator+=(float i);
	void operator-=(float i);
	void operator*=(float i);
	void operator/=(float i);

	Point operator+(const Point& i) const;
	Point operator+(float i) const;
	Point operator-(const Point& i) const;
	Point operator-(float i) const;
	Point operator-() const;
	Point operator*(const Point& i) const;
	Point operator*(float i) const;
	Point operator/(const Point& i) const;
	Point operator/(float i) const;

	// Assign operators
	Point& operator=(const Point&) = default;
	Point& operator=(Point&&) = default;

};