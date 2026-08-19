#pragma once

#include "Framework/Engine/Module.h"
#include "Framework/Input/InputEnums.h"
#include "Framework/Shapes/Point.h"
#include "Framework/Data/Array.h"

constexpr auto MAX_KEYBOARD_KEYS = 300;
constexpr auto MAX_MOUSE_BUTTONS = 5;

class Window;

class Input : public Module<Input>
{

	friend class App;
    friend class Render;

public:

    virtual ~Input();
	
    static bool GetKey(KeyCode code);

    static bool GetKeyDown(KeyCode code);

    static bool GetKeyUp(KeyCode code);

    static bool AnyKey();

    static bool AnyKeyUp();

    static bool GetMouseButton(MouseCode code);

    static bool GetMouseButtonDown(MouseCode code);

    static bool GetMouseButtonUp(MouseCode code);

    static bool AnyMouseButton();

    static bool AnyMouseButtonUp();

    // Get the Screen mouse position (independently on the camera)
    // Returns whether it is out of bounds (letterboxing)
    static bool GetScreenMousePosition(Point& position);

    // Get the Screen mouse position (with the camera)
    // Returns whether it is out of bounds (letterboxing)
    static bool GetWorldMousePosition(Point& position);

    static Point GetMouseMotion();

    static float GetMouseVScroll();
    
    static float GetMouseHScroll();

    // Returns a counter of consecutive number of clicks
    static int Clicks();

private:

	Input(Window* window);

	bool Awake() override;

	bool Start() override;

	bool Update(float dt) override;

	bool CleanUp() override;

	void UpdateKeyboardInput();

	void UpdateMouseInput();

    bool AdaptToSDLAxisScreen(Point& point) const;

private:

	Window* window = nullptr;

    Array<InputState> keyboard     = Array<InputState>(300, (InputState)0);
    Array<InputState> mouseButtons = Array<InputState>(  5, (InputState)0);
		
	Point mouseMotion = Point::Zero;
	Point mouse = Point::Zero;
    Point scrollMotion = Point::Zero;
    int clicks = 0;

    bool anyKeyboard[2] = {false, false};
    bool anyMouseButton[2] = {false, false};

};