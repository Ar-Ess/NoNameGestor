#pragma once

enum class WinEvent
{
	// True when we quit the window
	QUIT = 0,
	// True when we minimize the window
	MINIMIZED = 1,
	// True when we maximize the window
	SHOW = 2,
	// True when we resize the window
	RESIZED = 3,
	// True when we go from windowed mode to fullscreen
	SWAP_TO_FULLSCREEN = 4,
	// True when we go from fullscreen mode to windowed
	SWAP_TO_WINDOWED = 5,
	// True when we go swap from any window mode to another
	SWAP_WINDOW_MODE = 6,
	// True when we the window focus is lost with the keyboard
	KEYBORD_FOCUS_LOST = 7,
	// True when the window is hidden
	HIDE = 8,
	// True when the window size and position is restored
	RESTORED = 9,
	// True when we maximize the window
	MAXIMIZED = 10,
	// True when we regain focus on the window with the keyboard
	KEYBOARD_FOCUS_GAINED = 11,
	// True when we regain focus on the window with the mouse
	MOUSE_FOCUS_GAINED = 12,
	// True when we lost focus on the window with the mouse
	MOUSE_FOCUS_LOST = 13,
};
