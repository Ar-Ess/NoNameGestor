#include "Maths.h"

const double Maths::Pi()
{
	return 3.14159265359;
}

const double Maths::E()
{
	return Exp(1);
}

bool Maths::CheckCollision(Rect r1, Rect r2)
{
	return !((r1.x >= r2.x + r2.w) || (r1.x + r1.w < r2.x) || (r1.y >= r2.y + r2.h) || (r1.y + r1.h < r2.y));
}

float Maths::Sqrt(float num)
{
	if (num <= 0) return -1;

	return (float)sqrt((double)num);
}

float Maths::Pow(float num, float power)
{
	if (power == 1) return num;
	else if (power == 2) return num * num;
	else if (power == 3) return num * num * num;
	else if (power == 4) return num * num * num * num;
	else if (power == 5) return num * num * num * num * num;

	return (float)pow((double)num, (double)power);
}

float Maths::RadToDeg(float rad)
{
	return rad * (180 / Pi());
}

float Maths::DegToRad(float deg)
{
	return deg * (Pi() / 180);
}

int Maths::Ceil(float num)
{
	return (int)ceilf(num);
}

int Maths::Floor(float num)
{
	return (int)floorf(num);
}

int Maths::Aproximate(float num, float threshold)
{
	int ret = 0;
	threshold = Clamp(threshold, 0, 1);

	int floorNum = Floor(num);

	if (num >= (floorNum + threshold)) ret = Ceil(num);
	else ret = floorNum;
	
	return ret;
}

float Maths::Sin(float num, bool radians)
{
	if (radians) return sin(num);

	return sin(RadToDeg(num));
}

float Maths::Cos(float num, bool radians)
{
	if (radians) return cos(num);

	return cos(RadToDeg(num));
}

float Maths::Distance(float x1, float y1, float x2, float y2)
{
	double dx = double(x2) - double(x1);
	double dy = double(y2) - double(y1);
	return float(Sqrt((double)Pow((float)dx, 2) + (double)Pow((float)dy, 2)));
}

float Maths::Distance(Point a, Point b)
{
	double dx = double(b.x) - double(a.x);
	double dy = double(b.y) - double(a.y);
	return float(Sqrt((double)Pow((float)dx, 2) + (double)Pow((float)dy, 2)));
}

float Maths::Ln(float num)
{
	return (float)log((double)num);
}

float Maths::Log(float num, float base)
{
	if (base == 10) return (float)log10((double)num);

	return Ln(num) / Ln(base);
}

float Maths::LogToLinear(float num, float minLinear, float maxLinear)
{
	float log = Log(minLinear, 10) + num * (Log(maxLinear, 10) - Log(minLinear, 10));

	return Pow(10, log);
}

float Maths::LinearToLog(float num, float minLog, float maxLog)
{
	float log = Log(num, 10);
	return (log - Log(minLog, 10)) / (Log(maxLog, 10) - Log(minLog, 10));
}

float Maths::Exp(float num)
{
	return exp(num);
}

float Maths::Min(float a, float b)
{
	if (a <= b) return a;
	return b;
}

float Maths::Max(float a, float b)
{
	if (a >= b) return a;
	return b;
}

float Maths::Abs(float num)
{
	if (num < 0) num *= -1;
	return num;
}

float Maths::Clamp(float value, float min, float max)
{
	if (value < min) value = min;
	else if (value > max) value = max;

	return value;
}

Rect Maths::IntersectRectangle(Rect r1, Rect r2)
{
	float x = Max(r1.x, r2.x);
	float y = Max(r1.y, r2.y);
	float w = Min(r1.x + r1.w, r2.x + r2.w) - x;
	float h = Min(r1.y + r1.h, r2.y + r2.h) - y;

	if (w <= 0 || h <= 0) return { 0, 0, 0, 0 };

	return { x, y, w, h };
}

Point Maths::ClosestRectIntersectionFromOutsidePoint(Point point, Rect rect)
{
	Point ret = { 0.0f, 0.0f };

	ret.x = Max(rect.x, Min(point.x, rect.x + rect.w));
	ret.y = Min(rect.y + rect.h, Max(point.y, rect.y));

	return ret;
}

Point Maths::ClosestRectIntersectionFromInsidePoint(Point point, Rect rect)
{
	Point ret = { 0.0f, 0.0f };

	Point distanceToPositiveBounds = rect.Position(Alignment::TOP_RIGHT) - point;
	Point distanceToNegativeBounds = rect.Position(Alignment::BOTTOM_LEFT) - point;

	float smallestX = Min(distanceToPositiveBounds.x, distanceToNegativeBounds.x);
	float smallestY = Min(distanceToPositiveBounds.y, distanceToNegativeBounds.y);
	float smallestDistance = Min(smallestX, smallestY);

	if (smallestDistance == distanceToPositiveBounds.x) ret = { rect.x + rect.w, point.y };
	else if (smallestDistance == distanceToNegativeBounds.x) ret = { rect.x, point.y };
	else if (smallestDistance == distanceToPositiveBounds.y) ret = { point.x, rect.y };
	else if (smallestDistance == distanceToNegativeBounds.y) ret = { point.x, rect.y + rect.h };

	return ret;
}

Point Maths::ClosestRectIntersectionFromPoint(Point point, Rect rect)
{
	bool inside = Maths::CheckCollision({ point.Apply(-1.0f, -1.0f), 2, 2 }, rect);
	Point ret = { 0.0f, 0.0f };

	inside ? ret = Maths::ClosestRectIntersectionFromInsidePoint(point, rect) : ret = Maths::ClosestRectIntersectionFromOutsidePoint(point, rect);

	return ret;
}

float Maths::Soften(float value, float factor, Point start, Point end)
{
	if (factor == 1) return value;

	return ((end.y - start.y) * ( (Pow(factor, value - start.x) - 1) / (Pow(factor, end.x - start.x) - 1) ) ) + start.y;
}

float Maths::Soften(float value, float factor, float x1, float y1, float x2, float y2)
{
	if (factor == 1) return value;

	return ((y2 - y1) * ((Pow(factor, value - x1) - 1) / (Pow(factor, x2 - x1) - 1))) + y1;
}