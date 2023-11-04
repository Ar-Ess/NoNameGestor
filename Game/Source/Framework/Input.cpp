#include "Input.h"
#include "SDL/include/SDL.h"
#include "Window.h"

Input::Input(Window* window) : Module()
{
	this->window = window;
}

Input::~Input()
{
}

bool Input::Awake()
{
	keyboard.fill(InputState::INP_IDLE);
	mouseButtons.fill(InputState::INP_IDLE);

	return true;
}

bool Input::Start()
{
	LOG("Init SDL input event system");
	bool ret = true;

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Input::Update(float dt)
{
	bool ret = true;

	SDL_PumpEvents();
	static SDL_Event event;

	UpdateKeyboardInput();
	UpdateMouseInput();

	while(SDL_PollEvent(&event) != 0)
	{
		switch(event.type)
		{
		case SDL_QUIT: windowEvents[(int)EventWindow::WE_QUIT] = true; ret = false; break;

		case SDL_WINDOWEVENT:
		{
			switch (event.window.event)
			{
				//case SDL_WINDOWEVENT_LEAVE:
			case SDL_WINDOWEVENT_HIDDEN:
			case SDL_WINDOWEVENT_MINIMIZED:
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				windowEvents[(int)EventWindow::WE_HIDE] = true;
				break;

				//case SDL_WINDOWEVENT_ENTER:
			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			case SDL_WINDOWEVENT_MAXIMIZED:
				break;
			case SDL_WINDOWEVENT_RESTORED:
				windowEvents[(int)EventWindow::WE_SHOW] = true;
				break;
			}
			break;
		}

		case SDL_MOUSEBUTTONDOWN: 
			mouseButtons[event.button.button - 1] = InputState::INP_DOWN;
			anyMouseButton[0] = true;
			break;

		case SDL_MOUSEBUTTONUP: 
			mouseButtons[event.button.button - 1] = InputState::INP_UP; 
			anyMouseButton[1] = true;
			break;

		case SDL_MOUSEMOTION:
		{
			int scale = window->Scale();
			mouseMotion.x = float(event.motion.xrel) / float(scale);
			mouseMotion.y = float(event.motion.yrel) / float(scale);
			mouse.x = float(event.motion.x) / float(scale);
			mouse.y = float(event.motion.y) / float(scale);
			break;
		}

		case SDL_MOUSEWHEEL:
			scrollMotion.x = event.wheel.preciseX;
			scrollMotion.y = event.wheel.preciseY;
			break;

		}
	}

	return ret;
}

void Input::UpdateKeyboardInput()
{
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	anyKeyboard[0] = false;
	anyKeyboard[1] = false;

	for (unsigned int i = 0; i < MAX_KEYBOARD_KEYS; ++i)
	{
		if (keys[i] == 1) // BUTTON PRESSED
		{
			if (keyboard[i] == InputState::INP_IDLE)
				keyboard[i] = InputState::INP_DOWN;
			else
				keyboard[i] = InputState::INP_REPEAT;

			anyKeyboard[0] = true;
		}
		else // BUTTON RELEASED
		{
			if (keyboard[i] == InputState::INP_REPEAT || keyboard[i] == InputState::INP_DOWN)
			{
				keyboard[i] = InputState::INP_UP;
				anyKeyboard[1] = true;
			}
			else
				keyboard[i] = InputState::INP_IDLE;
		}
	}
}

void Input::UpdateMouseInput()
{
	anyMouseButton[0] = false;
	anyMouseButton[1] = false;

	for (unsigned int i = 0; i < MAX_MOUSE_BUTTONS; ++i)
	{
		if (mouseButtons[i] == InputState::INP_DOWN)
		{
			mouseButtons[i] = InputState::INP_REPEAT;
			anyMouseButton[0] = true;
		}

		if (mouseButtons[i] == InputState::INP_UP)
			mouseButtons[i] = InputState::INP_IDLE;
	}
}

bool Input::CleanUp()
{
	LOG("Quitting SDL event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);

	return true;
}

bool Input::GetKey(KeyCode code) const
{
	return keyboard[(int)code] == InputState::INP_REPEAT || keyboard[(int)code] == InputState::INP_DOWN;
}

bool Input::GetKeyDown(KeyCode code) const
{
	return keyboard[(int)code] == InputState::INP_DOWN;
}

bool Input::GetKeyUp(KeyCode code) const
{
	return keyboard[(int)code] == InputState::INP_UP;
}

bool Input::AnyKey() const
{
	return anyKeyboard[0];
}

bool Input::AnyKeyUp() const
{
	return anyKeyboard[1];
}

bool Input::GetMouseButton(MouseCode code) const
{
	return mouseButtons[(int)code] == InputState::INP_REPEAT || mouseButtons[(int)code] == InputState::INP_DOWN;
}

bool Input::GetMouseButtonDown(MouseCode code) const
{
	return mouseButtons[(int)code] == InputState::INP_DOWN;
}

bool Input::GetMouseButtonUp(MouseCode code) const
{
	return mouseButtons[(int)code] == InputState::INP_UP;
}

bool Input::AnyMouseButton() const
{
	return anyMouseButton[0];
}

bool Input::AnyMouseButtonUp() const
{
	return anyMouseButton[1];
}

bool Input::GetWindowEvent(EventWindow ev)
{
	return windowEvents[(int)ev];
}

Point Input::GetMousePosition() const
{
	return mouse;
}

Point Input::GetMouseMotion() const
{
	return mouseMotion;
}

float Input::GetMouseVScroll() const
{
	return scrollMotion.y;
}

float Input::GetMouseHScroll() const
{
	return scrollMotion.x;
}