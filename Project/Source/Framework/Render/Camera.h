#pragma once

#include "Framework/Engine/Object.h"
#include "Framework/Render/FollowMode.h"

//TODO: Camera should be able to be moved by the user, not only followed!
class Camera : private LogicObject
{

	friend struct FollowMode;
	friend class Render;

public:

	~Camera() override;

	void Reset();

	void Follow(const FollowMode& mode);

	Point Position(Alignment align = Alignment::BOTTOM_LEFT) const override;

	void Move(const Point& move) override;

	void Move(float x, float y) override;

	Point Size() const override;

private:

	Camera() = default;

	Camera(const Point& size);

	bool Update(float dt) override;

	void SetPosition();

private:

	FollowMode::Mode mode = FollowMode::Mode::NO_MODE;

	// Used Values
	Object* target = nullptr;
	bool2 follow = false;
	bool followHorizontal = false;
	bool followVertical = false;
	Point offset = Point::Zero;
	float softness = 0;
	float factor = 0;

	Rect limits = Rect::Null;
	bool2 limit = false;

	// Modifying values
	Alignment align = Alignment::BOTTOM_LEFT;
	Alignment targetAlign = Alignment::BOTTOM_LEFT;

	Point followState = Point::Zero;

};
