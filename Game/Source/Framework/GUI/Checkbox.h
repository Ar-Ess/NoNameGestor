#pragma once

#include "Framework/Gui/Control.h"

class Checkbox : public Control
{

	friend class ControlCreation;

public:

	virtual ~Checkbox();

	bool Draw(float dt) override;
    
    bool Check() const;
    void Check(bool check, bool notify = true);

    void SetSpriteIndexes(int normalOn, int normalOff, int focussedOn, int focussedOff, int pressedOn, int pressedOff, int disabledOn, int disabledOff, int selectedOn, int selectedOff);

private:
	
	Checkbox(const Rect& rectangle, const Animation& animation, bool check, bool dragOutside, Control** selected);
	
	bool Update(float dt) override;

	bool Manipulate() override;

    bool UpdateState(ControlState state, bool force = false) override;

public:

    // Set the key to interact with the button when selected
    KeyCode submitKey = KeyCode::RETURN;
    // Calls when the user releases the click button
    Action<bool> onRelease;
    // Calls when the user removes the pointer from the button while clicking. This won't call if dragOutside enabled
    Action<> onRemove;
    // Calls when the user puts the mouse on the button
    Action<> onFocus;
    // Calls when the user puts the mouse on the button
    Action<> onUnfocus;
    // Calls when the user clicks on the button.
    Action<> onClick;
    // Calls when the user submits the click on the button when checkbox selected.
    Action<bool> onSubmit;

private:

    int spriteDisabledOnIndex = 0;
    int spriteNormalOnIndex = 0;
    int spriteFocusedOnIndex = 0;
    int spritePressedOnIndex = 0;
    int spriteSelectedOnIndex = 0;
    int spriteDisabledOffIndex = 0;
    int spriteNormalOffIndex = 0;
    int spriteFocusedOffIndex = 0;
    int spritePressedOffIndex = 0;
    int spriteSelectedOffIndex = 0;

    bool check = false;

};


