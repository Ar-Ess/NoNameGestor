#pragma once

#include "Framework/Engine/Module.h"
#include "Framework/Input/InputManager.h"
#include "Framework/Data/Vector.h"
#include "Framework/GUI/ControlCreation.h"
#include "Framework/Gui/Button.h"
#include "Framework/Gui/Slider.h"
#include "Framework/Gui/Checkbox.h"

class AssetsManager;
class Font;

class GuiManager : public Module<GuiManager>
{

	friend class App;

public:

	// Create a new control based on a rectangle. "dragOutside" allows to influence on the control even if you're outside its bounds.
	// Once this function is called, follow it with a "->" to determine the type of control to create.
	static ControlCreation* CreateControl(const Rect& rectangle, bool dragOutside = true);

	// Completely erase a control from the gui
	static bool DestroyControl(Control* erase);

	//Set the key for GUI navigation. "KeyCode::UNKNOWN" to disable navigation.
	static void NavigationKey(KeyCode key = KeyCode::UNKNOWN);

	// Subscribe a control to the keyboard navigation system.
	// The navigation order is relative to this function call order
	// Use "NavigationKey(KeyCode)" to enable navigation.
	static void SubscribeToNavigation(Control* add);

	// Remove a control to the keyboard navigation system.
	// Use "NavigationKey(KeyCode::UNKNOWN)" to disable navigation.
	static void RemoveFromNavigation(Control* add);

	// Clear all controls from the navigation system
	static void ClearNavigation();

	// De/Activate Debug for all the controls
	static void Debug(bool active);

	// Retrieve the default system font
	static Font* DefaultFont();

private:

	GuiManager();

	bool Awake() override;

	bool Start() override;
	
	bool PreUpdate(float dt) override;

	bool CleanUp() override;

	void NavigationLogic();

private:

	Font* defaultFont = nullptr;

	Vector<Control*, true> controls;
	ControlCreation* creation = nullptr;
	
	Vector<Control*, false> navigation;
	KeyCode navKey = KeyCode::TAB;
	int navSelectedIndex = -1;
	Control* navSelected = nullptr;

};