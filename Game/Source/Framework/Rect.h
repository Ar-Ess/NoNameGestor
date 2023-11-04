#pragma once

#include "Point.h"
#include "Alignment.h"

class Rect
{
public:
	Rect()
	{
		this->x = 0;
		this->y = 0;
		this->w = 0;
		this->h = 0;
	}

	Rect(float x, float y, float w, float h) 
	{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}

	Rect(Point position, float w = 0, float h = 0)
	{
		this->x = position.x;
		this->y = position.y;
		this->w = w;
		this->h = h;
	}

	Rect(float x, float y, Point size)
	{
		this->x = x;
		this->y = y;
		this->w = size.x;
		this->h = size.y;
	}

	Rect(Point position, Point size)
	{
		this->x = position.x;
		this->y = position.y;
		this->w = size.x;
		this->h = size.y;
	}

	void Position(Point position)
	{
		x = position.x;
		y = position.y;
	}

	void Position(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	void Position(Point position, Alignment align)
	{
		switch (align)
		{
		case Alignment::CENTER: 
			x = position.x - (w / 2);
			y = position.y - (h / 2);
			break;

		case Alignment::CENTER_LEFT:
			x = position.x;
			y = position.y - (h / 2);
			break;

		case Alignment::CENTER_RIGHT:
			x = position.x - w;
			y = position.y - (h / 2);
			break;

		case Alignment::TOP_CENTER:
			x = position.x - (w / 2);
			y = position.y;
			break;

		case Alignment::TOP_LEFT:
			x = position.x;
			y = position.y;
			break;

		case Alignment::TOP_RIGHT:
			x = position.x - w;
			y = position.y;
			break;

		case Alignment::BOTTOM_CENTER:
			x = position.x - (w / 2);
			y = position.y - h;
			break;

		case Alignment::BOTTOM_LEFT:
			x = position.x;
			y = position.y - h;
			break;

		case Alignment::BOTTOM_RIGHT:
			x = position.x - w;
			y = position.y - h;
			break;

		default: break;
		}
	}

	void Size(Point size)
	{
		w = size.x;
		h = size.y;
	}

	void Size(float w, float h)
	{
		this->w = w;
		this->h = h;
	}

	Point Position() const
	{
		return Point{x, y};
	}

	Point Position(Alignment align) const
	{
		Point ret = { 0.0f, 0.0f };

		switch (align)
		{
		case Alignment::CENTER: ret = { x + w / 2, y + h / 2 }; break;

		case Alignment::CENTER_LEFT: ret = { x, y + h / 2 }; break;

		case Alignment::CENTER_RIGHT: ret = { x + w, y + h / 2 }; break;

		case Alignment::TOP_CENTER: ret = { x + w / 2, y }; break;

		case Alignment::TOP_LEFT: ret = {x, y}; break;

		case Alignment::TOP_RIGHT: ret = { x + w, y }; break;

		case Alignment::BOTTOM_CENTER: ret = { x + w / 2, y + h }; break;

		case Alignment::BOTTOM_LEFT: ret = { x, y + h }; break;

		case Alignment::BOTTOM_RIGHT: ret = { x + w, y + h }; break;

		default: break;
		}

		return ret;
	}

	Point Size() const
	{
		return Point{ w, h };
	}

	float Area() const
	{
		return w * h;
	}

	// Returns if the rectancle does not exist (width = 0 ot height = 0)
	bool IsNull() const
	{
		return (w == 0 || h == 0);
	}

public:

	float x = 0, y = 0, w = 0, h = 0;
};