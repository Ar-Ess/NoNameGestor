#ifndef __INPUT_H__
#define __INPUT_H__

#include "Module.h"
#include "Window.h"
#include "Point.h"
#include "SDL/include/SDL.h"

//#define NUM_KEYS 352
#define NUM_MOUSE_BUTTONS 5
#define MAX_PADS 1
//#define LAST_KEYS_PRESSED_BUFFER 50

struct SDL_Rect;

enum class EventWindow
{
	WE_QUIT = 0,
	WE_HIDE = 1,
	WE_SHOW = 2,
	WE_COUNT
};

enum class KeyState
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

class Input : public Module
{
public:

	Input(Window* window);

	virtual ~Input();

	bool Start();

	bool PreUpdate(float dt);

	void UpdateKeyboardInput();

	void UpdateMouseInput();

	uint8_t* UpdateGamepadsInput();

	bool CleanUp();

	KeyState GetKey(int id) const
	{
		return keyboard[id];
	}

	KeyState GetMouseButtonDown(int id) const
	{
		return mouseButtons[id - 1];
	}

	bool GetWindowEvent(EventWindow ev);

	void GetMousePosition(float &x, float &y);

	Point GetMousePosition() const;

	void GetMouseMotion(float& x, float& y);

	Point GetMouseMotion() const;

private:

	Window* window = nullptr;

	bool windowEvents[(int)EventWindow::WE_COUNT];

	KeyState* keyboard = nullptr;	
	KeyState* mouseButtons = nullptr;

	KeyState* controllerButtons = nullptr;
		
	Point mouseMotion = {};
	Point mouse = {};
};

#endif // __INPUT_H__