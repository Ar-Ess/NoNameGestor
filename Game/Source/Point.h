#ifndef __POINT_H__
#define __POINT_H__

#include <math.h>

struct Point
{
	Point() {}
	Point(int x, int y) { this->x = float(x); this->y = float(y); }
	Point(float x, float y) { this->x = x; this->y = y; }
	Point(double x, double y) { this->x = float(x); this->y = float(y); }

	Point(int a) { x = float(a); y = float(a); }
	Point(float a) { x = a; y = a; }
	Point(double a) { x = float(a); y = float(a); }

	float x = 0, y = 0;

	float Distance(Point to)
	{
		float x = (float)pow(to.x - double(this->x), 2);
		float y = (float)pow(to.y - double(this->y), 2);

		return (float)sqrt(double(x) + y);
	}
	float Distance(float x, float y)
	{
		float xx = (float)pow(x - double(this->x), 2);
		float yy = (float)pow(y - double(this->y), 2);

		return (float)sqrt(double(xx) + yy);
	}
	float Distance(double x, double y)
	{
		float xx = (float)pow(x - double(this->x), 2);
		float yy = (float)pow(y - double(this->y), 2);

		return (float)sqrt(double(xx) + yy);
	}
	float Distance(int x, int y)
	{
		float xx = (float)pow(double(x) - double(this->x), 2);
		float yy = (float)pow(double(y) - double(this->y), 2);

		return (float)sqrt(double(xx) + yy);
	}

	// Function: It applies the number inputed to the actual Point value and the result is the output.
	// It does not modify the value stored in the class
	Point Apply(Point sum) const
	{
		return Point{this->x + sum.x, this->y + sum.y};
	}
	Point Apply(float x, float y) const
	{
		return Point{ this->x + x, this->y + y };
	}
	Point Apply(double x, double y) const
	{
		return Point{ this->x + x, this->y + y };
	}
	Point Apply(int x, int y) const
	{
		return Point{ this->x + float(x), this->y + float(y) };
	}

	Point Multiply(Point multi) const
	{
		return Point{ this->x * multi.x, this->y * multi.y };
	}
	Point Multiply(float x, float y) const
	{
		return Point{ this->x * x, this->y * y };
	}
	Point Multiply(double x, double y) const
	{
		return Point{ this->x * x, this->y * y };
	}
	Point Multiply(int x, int y) const
	{
		return Point{ this->x * float(x), this->y * float(y) };
	}

	float GetMax()
	{
		return (float)fmax(x, y);
	}

	float GetMaxAbs()
	{
		return (float)fmax(fabsf(x), fabsf(y));
	}

	float operator[](int i) 
	{
		float ret = 0;

		//--Cond--  --True--  ------------False------------
		//   ||        ||     --Cond--  --True--  --False--
		//   \/        \/        \/        \/         \/
		(i == 0) ? ret = x : ((i == 1) ? ret = y : ret = 0);

		return ret;
	}

	bool operator==(Point b) { return (x == b.x && y == b.y); }
	bool operator==(int i) { return (x == i || y == i); }
	bool operator!=(Point b) { return !(x == b.x && y == b.y); }
	bool operator!=(int i) { return !(x == i && y == i); }

	void operator+=(int i) { x += i; y += i; }
	void operator-=(int i) { x -= i; y -= i; }
	void operator*=(int i) { x *= i; y *= i; }
	void operator/=(int i) { if (i == 0) return; x /= i; y /= i; }

	void operator+=(Point i) { x += i.x; y += i.y; }
	void operator-=(Point i) { x -= i.x; y -= i.y; }
	void operator*=(Point i) { x *= i.x; y *= i.y; }
	void operator/=(Point i) { if (i == 0) return; x /= i.x; y /= i.y; }

	void operator+=(float i) { x += i; y += i; }
	void operator-=(float i) { x -= i; y -= i; }
	void operator*=(float i) { x *= i; y *= i; }
	void operator/=(float i) { if (i == 0) return; x /= i; y /= i; }

	Point operator+(int i) { return Point{ x + i, y + i }; }
	Point operator-(int i) { return Point{ x - i, y - i }; }
	Point operator*(int i) { return Point{ x * i, y * i }; }

	Point operator+(float i) { return Point{ x + i, y + i }; }
	Point operator-(float i) { return Point{ x - i, y - i }; }
	Point operator*(float i) { return Point{ x * i, y * i }; }

	Point operator/(int div) { return Point{ x / div, y / div }; }

	bool operator<(int i) { return (x < i&& y < i); }
	bool operator<=(int i) { return (x <= i && y <= i); }
	bool operator>(int i) { return (x > i && y > i); }
	bool operator>=(int i) { return (x >= i && y >= i); }

	bool operator<(Point b) { return (x < b.x&& y < b.y); }
	bool operator<=(Point b) { return (x <= b.x && y <= b.y); }
	bool operator>(Point b) { return (x > b.x && y > b.y); }
	bool operator>=(Point b) { return (x >= b.x && y >= b.y); }

	Point& operator = (const Point& u)
	{
		x = u.x; y = u.y; return *this;
	}

};

#endif //__POINT_H__