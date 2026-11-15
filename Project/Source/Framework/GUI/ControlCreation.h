#pragma once

#include <concepts>
#include "Framework/Shapes/Rect.h"
#include "Framework/Data/Vector.h"

class Font;
class Button;
class Checkbox;
class Slider;
class Control;
class Animation;

class ControlCreation
{
public:

	Button* NewButton(const Animation& spritesheet);

	Checkbox* NewCheckbox(const Animation& spritesheet, bool initialState = false);

	// moveStep is a value clamped from 0 to 0.5. Is the % of value increment when manipulating the slider while navigating
	Slider* NewSlider(const Animation& spritesheet, const Animation& sliderSpritesheet, const Point& sliderSize, float min = 0, float max = 1, float initialValue = 0, float moveStep = 0.1f);

private:

	friend class GuiManager;

	ControlCreation* Access(const Rect& rectangle, bool dragOutside);

	ControlCreation(Vector<Control*, true>* controls, Control** selected, Font* defaultFont);

private:

	Vector<Control*, true>* controls = nullptr;
	Font* defaultFont = nullptr;
	Control** selected = nullptr;
	Rect rectangle = Rect::Null;
	bool dragOutside = true;
};
