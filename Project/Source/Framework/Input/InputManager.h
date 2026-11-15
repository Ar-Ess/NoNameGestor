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

    static KeyCode GetFirstKey(InputState state);

    static const char* GetKeyName(KeyCode key);

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

    const char* KeyCodeName[512] =
    {
        "UNKNOWN", // 0
        "", "", "", // 1-3
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", // 4-15
        "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", // 16-29
        "NUM_1", "NUM_2", "NUM_3", "NUM_4", "NUM_5", "NUM_6", "NUM_7", "NUM_8", "NUM_9", "NUM_0", // 30-39
        "RETURN", "ESCAPE", "BACKSPACE", "TAB", "SPACE", "MINUS", "EQUALS", "LEFTBRACKET",
        "RIGHTBRACKET", "BACKSLASH", "NONUSHASH", "SEMICOLON", "APOSTROPHE", "GRAVE",
        "COMMA", "PERIOD", "SLASH", "CAPSLOCK", // 40-57
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
        "PRINTSCREEN", "SCROLLLOCK", "PAUSE", "INSERT", "HOME", "PAGEUP", "DELETE",
        "END", "PAGEDOWN", "RIGHT", "LEFT", "DOWN", "UP", "NUMLOCKCLEAR",
        "KP_DIVIDE", "KP_MULTIPLY", "KP_MINUS", "KP_PLUS", "KP_ENTER",
        "KP_1", "KP_2", "KP_3", "KP_4", "KP_5", "KP_6", "KP_7", "KP_8", "KP_9",
        "KP_0", "KP_PERIOD", "NONUSBACKSLASH", "APPLICATION", "POWER", "KP_EQUALS",
        "F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22",
        "F23", "F24", "EXECUTE", "HELP", "MENU", "SELECT", "STOP", "AGAIN", "UNDO",
        "CUT", "COPY", "PASTE", "FIND", "MUTE", "VOLUMEUP", "VOLUMEDOWN",
        "", "", "", // 130-132
        "KP_COMMA", "KP_EQUALSAS400", // 133-134
        "INTERNATIONAL1", "INTERNATIONAL2", "INTERNATIONAL3", "INTERNATIONAL4",
        "INTERNATIONAL5", "INTERNATIONAL6", "INTERNATIONAL7", "INTERNATIONAL8",
        "INTERNATIONAL9", "LANG1", "LANG2", "LANG3", "LANG4", "LANG5", "LANG6",
        "LANG7", "LANG8", "LANG9", "ALTERASE", "SYSREQ", "CANCEL", "CLEAR", "PRIOR",
        "RETURN2", "SEPARATOR", "OUT_KEY", "OPER", "CLEARAGAIN", "CRSEL", "EXSEL",
        "", "", "", "", "", "", "", "", "", "", "", // 165-175
        "KP_00", "KP_000", "THOUSANDSSEPARATOR", "DECIMALSEPARATOR", "CURRENCYUNIT",
        "CURRENCYSUBUNIT", "KP_LEFTPAREN", "KP_RIGHTPAREN", "KP_LEFTBRACE",
        "KP_RIGHTBRACE", "KP_TAB", "KP_BACKSPACE", "KP_A", "KP_B", "KP_C", "KP_D",
        "KP_E", "KP_F", "KP_XOR", "KP_POWER", "KP_PERCENT", "KP_LESS", "KP_GREATER",
        "KP_AMPERSAND", "KP_DBLAMPERSAND", "KP_VERTICALBAR", "KP_DBLVERTICALBAR",
        "KP_COLON", "KP_HASH", "KP_SPACE", "KP_AT", "KP_EXCLAM", "KP_MEMSTORE",
        "KP_MEMRECALL", "KP_MEMCLEAR", "KP_MEMADD", "KP_MEMSUBTRACT",
        "KP_MEMMULTIPLY", "KP_MEMDIVIDE", "KP_PLUSMINUS", "KP_CLEAR", "KP_CLEARENTRY",
        "KP_BINARY", "KP_OCTAL", "KP_DECIMAL", "KP_HEXADECIMAL",
        "", "", // 222-223
        "LCTRL", "LSHIFT", "LALT", "LGUI", "RCTRL", "RSHIFT", "RALT", "RGUI",
    };

};