#pragma once

#include "Framework/Gui/Control.h"

class Slider : public Control
{

	friend class ControlCreation;

public:

    virtual ~Slider();
    
    bool Draw(float dt) override;

    float Value() const;
    void Value(float value, bool notify = true);

    void SetSpriteIndexes(int normalBar, int normalSlider, int focussedBar, int focussedSlider, int pressedBar, int pressedSlider, int disabledBar, int disabledSlider, int selectedBar, int selectedSlider);

    Point Size() const override
    {
        return Control::Size();
    }
    void Size(float x, float y) override
    {
        Slider::Size({ x, y });
    }
    void Size(const Point& size) override;

    Point Scale() const override
    {
        return Control::Scale();
    }
    void Scale(float v, Alignment align = Alignment::BOTTOM_LEFT) override
    {
        Slider::Scale({ v, v }, align);
    }
    void Scale(float x, float y, Alignment align = Alignment::BOTTOM_LEFT) override
    {
        Slider::Scale({ x, y }, align);
    }
    void Scale(const Point& scale, Alignment align = Alignment::BOTTOM_LEFT) override;

    void SliderScale(float v)
    {
        return SliderScale({ v, v });
    }
    void SliderScale(float x, float y)
    {
        return SliderScale({ x, y });
    }
    void SliderScale(const Point& scale);

    void ColorMod(const Color& color) override;
    void ResetColorMod() override;

private:

    // moveStep is a value that goes from 0 to 1. It means the percentage of the range it will move if the slider is manipulated
    Slider(const Rect& rectangle,const Animation& spritesheet, const Animation& sliderSpritesheet, const Point& sliderSize, const Point& range, float initialValue, float moveStep, bool dragOutside, Control** selected);
    
    bool Update(float dt) override;

    bool Manipulate() override;

    bool UpdateState(ControlState state, bool force = false) override;

public:

    // Set the key to augment with the slider when selected
    KeyCode augmentKey = KeyCode::RIGHT;
    // Set the key to decrease with the slider when selected
    KeyCode decreaseKey = KeyCode::LEFT;
    // Set the key to slow the moveStep of slider movement
    KeyCode focusKey = KeyCode::LEFT;
    // Calls when the user releases the click button
    Action<> onRelease;
    // Calls when the user removes the pointer from the button while clicking. This won't call if DGSO enabled
    Action<> onRemove;
    // Calls when the user puts the mouse on the button
    Action<> onFocus;
    // Calls when the user puts the mouse on the button
    Action<> onUnfocus;
    // Calls when the user submits or clicks on the button.
    Action<> onClick;
    // Calls when the value of the slider has changed
    Action<float> onValueChange;
    // Calls when the value of the slider is changing
    Action<float> onValueChangeRuntime;
    // Calls when the value of the slider has changed when it was selected
    Action<float> onValueSubmitted;

    Flip flipSliderOverride = Flip::NO_FLIP;
    
private:

    int spriteBarDisabledIndex = 0;
    int spriteBarNormalIndex = 0;
    int spriteBarFocusedIndex = 0;
    int spriteBarPressedIndex = 0;
    int spriteBarSelectedIndex = 0;
    int spriteSliderDisabledIndex = 0;
    int spriteSliderNormalIndex = 0;
    int spriteSliderFocusedIndex = 0;
    int spriteSliderPressedIndex = 0;
    int spriteSliderSelectedIndex = 0;

    const Point range = Point::One;
    const float moveStep = 0.1f;
    
    Object slider;
    Animation sliderSpritesheet;

    float mouseOffset = 0;
    float dragStartX = 0;
    float prevSliderX = 0;

};