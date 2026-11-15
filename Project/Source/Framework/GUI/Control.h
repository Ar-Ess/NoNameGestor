#pragma once

#include "Framework/Engine/Object.h"
#include "Framework/Gui/ControlEnums.h"
#include "Framework/Render/Animation.h"
#include "Framework/Utils/Action.h"
#include "Framework/Input/InputEnums.h"

class Control : public LogicObject
{

    friend class GuiManager;

public:

    virtual ~Control();

    virtual bool Enable();

    virtual bool Disable();

    bool IsSelected() const;

    bool IsEnabled() const;

    bool IsState(ControlState state) const;

    ControlType Type() const;

    virtual Point Size() const override;
    virtual void Size(float x, float y) override;
    virtual void Size(const Point& size) override;

    void TextureScale(float v);
    void TextureScale(float x, float y);
    void TextureScale(const Point& scale);

    virtual void ColorMod(const Color& color);
    virtual void ResetColorMod();

protected:

    Control(ControlType type, const Rect& rectangle, const Animation& spritesheet, bool dragOutside, Control** selected);

    virtual bool Update(float dt) override;

    virtual bool Manipulate();

    bool MousePosition(Point& mouse) const;

    virtual bool UpdateState(ControlState state, bool force = false);

public: //Operators

    bool operator==(const Control* control) const;

    bool operator==(const Control& control) const;

public:

    // Calls when the control is selected throught GUI Navigation
    Action<> onSelect;
    // Calls when the control is diselected throught GUI Navigation
    Action<> onDiselect;
    // Calls when the control is disabled
    Action<> onDisable;
    // Calls when the control is enabled
    Action<> onEnable;

protected:

    const ControlType type = ControlType::EMPTY_CONTROL;
    const bool dragOutside = false;
    Control** selected = nullptr;
    
    Animation spritesheet;
    ControlState state = ControlState::NORMAL;

    Point baseSpritesheetScale = Point::One;
    Point invBaseSpritesheetScale = Point::One;

};