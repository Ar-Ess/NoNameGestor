#pragma once

#include "Framework/Shapes/Rect.h"
#include "Framework/Utils/bool2.h"

class Object;
class Camera;

struct FollowMode
{
	enum class Mode
	{
		NO_MODE,
		OFFSET_MODE,
		OFFSET_SOFT_MODE,
		OFFSET_INV_SOFT_MODE,
		LIMITS_MODE,
		LIMITS_SOFT_MODE,
		LIMITS_INV_SOFT_MODE,
	};

	friend class Camera;

public:

	// Sets the OffsetFollow mode to the game camera
	// target is the object to follow
	// offset is the offset aplied to the asigned object position
	// offsetAlign is from which part of the camera rectangle is the offset applied
	// targetAlign is from which part of the target rectangle is the offset applied
	// followInAxis(Horizontal/Vertical) allows to disable the camera following on those axis
	static const FollowMode OffsetFollow(Object* target, Point offset, Alignment offsetAlign = Alignment::CENTER,
		Alignment targetAlign = Alignment::CENTER, bool2 followInAxis = true);

	// Sets the OffsetSoftFollow mode to the game camera
	// target is the object to follow
	// offset is the offset applied to the assigned object position
	// softness is a 0 to 1 ratio value where 1 is maximum softness which means super slow movement (and viceversa)
	// factor is a o to inf value that exaggerates the smooth maximum distance without affecting the softness velocity
	// offsetAlign is from which part of the camera rectangle is the offset applied
	// targetAlign is from which part of the target rectangle is the offset applied
	// followInAxis(Horizontal/Vertical) allows to disable the camera following on those axis
	static const FollowMode OffsetSoftFollow(Object* target, Point offset, float softness, float factor,
		Alignment offsetAlign = Alignment::CENTER, Alignment targetAlign = Alignment::CENTER, bool2 followInAxis = true);

	// Sets the OffsetInvSoftFollow mode to the game camera
	// target is the object to follow
	// offset is the offset applied to the assigned object position
	// softness is a 0 to 1 ratio value where 1 is maximum softness which means super slow movement (and viceversa)
	// factor is a o to inf value that exaggerates the smooth maximum distance without affecting the softness velocity
	// offsetAlign is from which part of the camera rectangle is the offset applied
	// targetAlign is from which part of the target rectangle is the offset applied
	// followInAxis(Horizontal/Vertical) allows to disable the camera following on those axis
	static const FollowMode OffsetInvSoftFollow(Object* target, Point offset, float softness, float factor,
		Alignment offsetAlign = Alignment::CENTER, Alignment targetAlign = Alignment::CENTER, bool2 followInAxis = true);

	// Sets the LimitFollow mode to the game camera
	// target is the object to follow
	// limits is a rectangle that defines a bounding box where the cam can't escape
	// offset is the offset aplied to the asigned object position
	// offsetAlign is from which part of the camera rectangle is the offset applied
	// targetAlign is from which part of the target rectangle is the offset applied
	// limitInAxis(Horizontal/Vertical) allows to disable the limiting on those axis
	// followInAxis(Horizontal/Vertical) allows to disable the camera following on those axis
	static const FollowMode LimitFollow(Object* target, Rect limits, Point offset,
		Alignment offsetAlign = Alignment::CENTER, Alignment targetAlign = Alignment::CENTER,
		bool2 limitInAxis = true, bool2 followInAxis = true);

	// Sets the LimitSoftFollow mode to the game camera
	// target is the object to follow
	// limits is a rectangle that defines a bounding box where the cam can't escape
	// offset is the offset applied to the assigned object position
	// softness is a -1 to 1 ratio value where 1 is maximum softness which means super slow movement (and viceversa), and -1 the maximum inversed softness (in advance softness)
	// offsetAlign is from which part of the camera rectangle is the offset applied
	// targetAlign is from which part of the target rectangle is the offset applied
	// limitInAxis(Horizontal/Vertical) allows to disable the limiting on those axis
	// followInAxis(Horizontal/Vertical) allows to disable the camera following on those axis
	static const FollowMode LimitSoftFollow(Object* target, Rect limits, Point offset,
		float softness, float factor, Alignment offsetAlign = Alignment::CENTER, Alignment targetAlign = Alignment::CENTER,
		bool2 limitInAxis = true, bool2 followInAxis = true);

	// Sets the LimitInvSoftFollow mode to the game camera
	// target is the object to follow
	// limits is a rectangle that defines a bounding box where the cam can't escape
	// offset is the offset applied to the assigned object position
	// softness is a -1 to 1 ratio value where 1 is maximum softness which means super slow movement (and viceversa), and -1 the maximum inversed softness (in advance softness)
	// offsetAlign is from which part of the camera rectangle is the offset applied
	// targetAlign is from which part of the target rectangle is the offset applied
	// limitInAxis(Horizontal/Vertical) allows to disable the limiting on those axis
	// followInAxis(Horizontal/Vertical) allows to disable the camera following on those axis
	static const FollowMode LimitInvSoftFollow(Object* target, Rect limits, Point offset,
		float softness, float factor, Alignment offsetAlign = Alignment::CENTER, Alignment targetAlign = Alignment::CENTER,
		bool2 limitInAxis = true, bool2 followInAxis = true);

private:

	FollowMode() = default;
	FollowMode(Object* target, Rect limits, Point offset, float softness, float factor,
		Alignment offsetAlign, Alignment targetAlign, bool2 follow, bool2 limit, Mode type);

private:

	// Used Values
	Object* target = nullptr;
	bool2 follow = false;
	Point offset = Point::Zero;
	float softness = 0;
	float factor = 0;

	Rect limits = Rect::Null;
	bool2 limit = false;
	
	// Modifying values
	Alignment align = Alignment::BOTTOM_LEFT;
	Alignment targetAlign = Alignment::BOTTOM_LEFT;

	Mode mode = Mode::NO_MODE;

};