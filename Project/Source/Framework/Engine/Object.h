#pragma once

#include "Framework/Shapes/Rect.h"
#include "Framework/Engine/ID.h"
#include "Framework/Data/Flag.h"

class Object
{
public:

	Object() : Object(Rect::Null) {}
	Object(const Object& other);
	Object(Object&& other) noexcept;
	Object(const Rect& rect);

	virtual ~Object();
	
	virtual Point Position(Alignment align = Alignment::BOTTOM_LEFT) const;
	virtual void Position(float x, float y, Alignment align = Alignment::BOTTOM_LEFT)
	{
		return Object::Position({ x, y }, align);
	}
	virtual void Position(const Point& pos, Alignment align = Alignment::BOTTOM_LEFT);

	virtual Point LocalPosition(Alignment align = Alignment::BOTTOM_LEFT) const;
	virtual void LocalPosition(float x, float y, Alignment align = Alignment::BOTTOM_LEFT)
	{
		Object::LocalPosition({ x, y }, align);
	}
	virtual void LocalPosition(const Point& pos, Alignment align = Alignment::BOTTOM_LEFT);

	virtual void Move(float x, float y);
	virtual void Move(const Point& move)
	{
		Object::Move(move.x, move.y);
	}

	virtual Point Size() const;
	virtual void Size(float w, float h);
	virtual void Size(const Point& size)
	{
		Object::Size(size.x, size.y);
	}

	virtual Point LocalSize() const;
	virtual Point GlobalSize() const;

	virtual Point Scale() const;
	virtual void Scale(float v, Alignment align = Alignment::BOTTOM_LEFT)
	{
		Object::Scale({ v, v }, align);
	}
	virtual void Scale(float x, float y, Alignment align = Alignment::BOTTOM_LEFT)
	{
		Object::Scale({ x, y }, align);
	}
	virtual void Scale(const Point& scale, Alignment align = Alignment::BOTTOM_LEFT);

	virtual Point LocalScale() const;
	virtual void LocalScale(float v, Alignment align = Alignment::BOTTOM_LEFT)
	{
		Object::LocalScale({ v, v }, align);
	}
	virtual void LocalScale(float x, float y, Alignment align = Alignment::BOTTOM_LEFT)
	{
		Object::LocalScale({ x, y }, align);
	}
	virtual void LocalScale(const Point& scale, Alignment align = Alignment::BOTTOM_LEFT);

	virtual bool Draw(float dt);

	bool AnchoredToCam() const;
	void AnchoredToCam(bool anchored);

	// Attach the position of this audio to an object
	// align determines which part of the attachment will this object attach
	// respectCurrentPosition determines if this object will snap to the attachment position (false) or respect this object's current position (true)
	bool AttachToObject(Object* attachment, bool respectCurrentPosition);
	// Attach the position of this audio to an object
	// align determines which part of the attachment will this object attach
	// respectCurrentPosition determines if this object will snap to the attachment position (false) or respect this object's current position (true)
	bool AttachToObject(Object* attachment, Alignment align = Alignment::BOTTOM_LEFT);
	// Attach the position of this audio to an object
	// parentAlign determines which part of the attachment will this object attach respect to the childAlign
	// respectCurrentPosition determines if this object will snap to the attachment position (false) or respect this object's current position (true)
	bool AttachToObject(Object* attachment, Alignment parentAlign, Alignment childAlign);
	void Detach(bool respectCurrentPosition = false);

public: // Operators

	Object& operator=(const Object& other);

	Object& operator=(Object&& other) noexcept;

	bool operator==(const Object& other) const;

	bool operator==(const Object* other) const;

public:

	Rect rect = Rect::Null;
	const ID id = ID::Empty;
	bool active = true;
	bool debug = false;

protected:

	bool anchoredToCam = false;
	Point scale = Point::One;

private:
	
	Object* parent = nullptr;
	Alignment align = Alignment::BOTTOM_LEFT;
	Alignment parentAlign = Alignment::BOTTOM_LEFT;
};

class LogicObject : public Object
{
public:

	LogicObject();
	LogicObject(const LogicObject& other) = default;
	LogicObject(LogicObject&& other) = default;
	LogicObject(const Rect& rect);

	virtual ~LogicObject() override;

	virtual bool Update(float dt);

public: // Operators

	LogicObject& operator=(const LogicObject& other) = default;

	LogicObject& operator=(LogicObject&& other) = default;

	bool operator==(const LogicObject& other) const = default;

};

class GameObject : public LogicObject
{
public:

	GameObject();
	GameObject(const GameObject& other) = default;
	GameObject(GameObject&& other) = default;
	GameObject(const Rect& rect);

	virtual ~GameObject() override;

	virtual bool Awake();
	virtual bool Start();
	virtual bool End();
	virtual bool CleanUp();

public: // Operators

	GameObject& operator=(const GameObject& other) = default;

	GameObject& operator=(GameObject&& other) = default;

	bool operator==(const GameObject& other) const = default;

};
