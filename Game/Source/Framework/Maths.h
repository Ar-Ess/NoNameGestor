#pragma once

#include "Rect.h"

namespace Maths
{
	const double Pi();
	
	const double E();

	// This function does not consider a collision if rects touches their bounds
	bool CheckCollision(Rect r1, Rect r2);

	float Sqrt(float num);

	float Pow(float num, float power);

	float RadToDeg(float rad);

	float DegToRad(float deg);

	int Ceil(float num);

	int Floor(float num);

	// Threshold must range between 0 and 1
	int Aproximate(float num, float threshold = 0.5f);

	float Sin(float num, bool radians = true);

	float Cos(float num, bool radians = true);

	float Distance(float x1, float y1, float x2, float y2);

	float Distance(Point a, Point b);

	float Ln(float num);

	float Log(float num, float base = 10);

	float LinearToLog(float num, float minLinear, float maxLinear);

	float LogToLinear(float num, float minLog, float maxLog);

	float Exp(float num);

	float Min(float a, float b);

	float Max(float a, float b);

	float Abs(float num);

	float Clamp(float value, float min, float max);

	Rect IntersectRectangle(Rect r1, Rect r2);
	
	Point ClosestRectIntersectionFromOutsidePoint(Point point, Rect rect);

	Point ClosestRectIntersectionFromInsidePoint(Point point, Rect rect);

	Point ClosestRectIntersectionFromPoint(Point point, Rect rect);

	float Soften(float value, float factor, Point start = { 0, 0 }, Point end = { 1, 1 });
	
	float Soften(float value, float factor, float x1 = 0, float y1 = 0, float x2 = 1, float y2 = 0);

};
