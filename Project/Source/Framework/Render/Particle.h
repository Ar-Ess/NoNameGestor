#pragma once

#include "Framework/Shapes/Rect.h"
#include "Framework/Render/Color.h"

class Emitter;

class Particle
{

	friend class Render;

public:

	Particle();
	~Particle();

	void Init(Emitter* emitter, const Point& position, float startSpeed, float endSpeed, float angle, double rotSpeed, float startSize, float endSize, unsigned int life);

	void Interpolate(float dt);

	bool IsAlive() const;
	
	Point Position() const;

public:

	Particle* next = nullptr;
	Color currentColor = {};
	float currentSize = 0;
	double currentAngle = 0;
	unsigned int currentLife = 0;

private:

	Emitter* emitter = nullptr;

	Point position = Point::Zero;

	// Velocity
	Point currentVelocity = Point::Zero;
	Point startVelocity = Point::Zero;
	Point endVelocity = Point::Zero;

	// Rotation
	double rotationSpeed = 0;

	// Size
	float startSize = 0;
	float endSize = 0;


	// Life
	unsigned int totalLife = 0;

};