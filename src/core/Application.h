#pragma once

#include "core/core_types.h"

// TODO: Move platform stuff to a seperate platform layer. Also
// remove platform stuff from application.cpp.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

struct ApplicationConfig
{
	u32 client_width;
	u32 client_height;
	u32 pos_x;
	u32 pos_y;
	char* name;
};

struct Application
{
	u32 client_width;
	u32 client_height;
	u32 pos_x;
	u32 pos_y;
	HWND window_handle;

	// Debug stats.
	u32 frame_count;
	LARGE_INTEGER frequency, time;
};

bool initialize(Application* app, ApplicationConfig& config);

void shutdown(Application* app);

bool create_window(Application* app);

bool run(Application* app);

void process_input();

void update_debug_stats(HWND window_handle, u32& frame_count, LARGE_INTEGER frequency, LARGE_INTEGER& time);