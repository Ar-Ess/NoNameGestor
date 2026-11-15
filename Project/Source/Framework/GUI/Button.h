#pragma once

#include "Framework/Gui/Control.h"

class Button : public Control
{

    friend class ControlCreation;

public:

    virtual ~Button();

    bool Draw(float dt) override;

    void SetSpriteIndexes(int normal, int focussed, int pressed, int disabled, int selected);

private:

    Button(const Rect& rectangle, const Animation& spritesheet, bool dragOutside, Control** selected);

    bool Update(float dt) override;

    bool Manipulate() override;

    bool UpdateState(ControlState state, bool force = false) override;

public:
    
    // Set the key to interact with the button when selected
    KeyCode submitKey = KeyCode::RETURN;
    // Calls when the user releases the click button
    Action<> onRelease;
    // Calls when the user removes the pointer from the button while clicking. This won't call if dragOutside enabled
    Action<> onRemove;
    // Calls when the user puts the mouse on the button
    Action<> onFocus;
    // Calls when the user puts the mouse on the button
    Action<> onUnfocus;
    // Calls when the user clicks on the button.
    Action<> onClick;
    // Calls when the user submits the click on the button when checkbox selected.
    Action<> onSubmit;

private:

    int spriteDisabledIndex = 0;
    int spriteNormalIndex = 0;
    int spriteFocusedIndex = 0;
    int spritePressedIndex = 0;
    int spriteSelectedIndex = 0;
};
