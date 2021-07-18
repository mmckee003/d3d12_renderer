#include "core/input.h"
#include "core/logger.h"
#include "core/platform/platform.h"

struct KeyboardInput
{
	bool keys[256];
};

struct MouseInput
{
	s32 x;
	s32 y;
	bool buttons[(u8)Button::BUTTON_MAX_BUTTONS];
};

struct Input
{
	KeyboardInput keyboard_current;
	KeyboardInput keyboard_previous;
	MouseInput mouse_current;
	MouseInput mouse_previous;
};

// TODO: Maybe these shouldn't be globals??
static bool initialized = false;
static Input input = {};

void initialize_input()
{
	initialized = true;
	LOG_INFO("Input subsystem initialized.");
}

void shutdown_input()
{
	// TODO: Add shutdown routines when needed.
	initialized = false;
}

void update_input(f64 delta_time)
{
	Assert(initialized);

	copy_memory(&input.keyboard_previous, &input.keyboard_current, sizeof(KeyboardInput));
	copy_memory(&input.mouse_previous, &input.mouse_current, sizeof(MouseInput));
}

// Keyboard input.
bool is_key_down(Key key)
{
	Assert(initialized);
	return input.keyboard_current.keys[(u32)key] == true;
}
bool is_key_up(Key key)
{
	Assert(initialized);
	return input.keyboard_current.keys[(u32)key] == false;
}

bool was_key_down(Key key)
{
	Assert(initialized);
	return input.keyboard_previous.keys[(u32)key] == true;
}

bool was_key_up(Key key)
{
	Assert(initialized);
	return input.keyboard_previous.keys[(u32)key] == false;
}

void process_key(Key key, bool pressed)
{
	// LOG_DEBUG("Processing key: %c", (u16)key);
	if (input.keyboard_current.keys[(u32)key] != pressed)
	{
		input.keyboard_current.keys[(u32)key] = pressed;
	}
}

// Mouse input.
bool is_button_down(Button button)
{
	Assert(initialized);
	return input.mouse_current.buttons[(u8)button] == true;
}

bool is_button_up(Button button)
{
	Assert(initialized);
	return input.mouse_current.buttons[(u8)button] == false;
}

bool was_button_down(Button button)
{
	Assert(initialized);
	return input.mouse_previous.buttons[(u8)button] == true;
}

bool was_button_up(Button button)
{
	Assert(initialized);
	return input.mouse_previous.buttons[(u8)button] == false;
}

void get_mouse_position(s32& x, s32& y)
{
	Assert(initialized);
	x = input.mouse_current.x;
	y = input.mouse_current.y;
}

void get_previous_mouse_position(s32& x, s32& y)
{
	Assert(initialized);
	x = input.mouse_previous.x;
	y = input.mouse_previous.y;
}

void process_button(Button button, bool pressed)
{
	// LOG_DEBUG("Processing Button: %d", (u8)button);
	if (input.mouse_current.buttons[(u8)button] != pressed)
	{
		input.mouse_current.buttons[(u8)button] = pressed;
	}
}

void process_mouse_move(s32 x, s32 y)
{
	if (input.mouse_current.x != x || input.mouse_current.y != y)
	{
		// LOG_DEBUG("Processing mouse movement: %d, %d", x, y);
		input.mouse_current.x = x;
		input.mouse_current.y = y;
	}
}

void process_mouse_wheel(s8 z_delta)
{
	// TODO: Add this to input tracking.
	// LOG_DEBUG("Processing mouse scroll: %d", z_delta);
}