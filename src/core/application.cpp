#include "core/application.h"

static LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(Window, Message, WParam, LParam);
}

bool initialize(Application* app, ApplicationConfig& config)
{
    QueryPerformanceFrequency(&(app->frequency));
    QueryPerformanceCounter(&(app->time));

	app->client_width  = config.client_width;
	app->client_height = config.client_height;
	app->pos_x         = config.pos_x;
	app->pos_y         = config.pos_y;

    if (!create_window(app))
    {
        // TODO: Log a message if we fail to create a window.
        return false;
    }
}

void shutdown(Application* app)
{
	// cleanup stuff like maybe destroy window(s).
}

bool create_window(Application* app)
{
    WNDCLASSEXW window_class = {};
    window_class.cbSize = sizeof(window_class);
    window_class.lpfnWndProc = &WindowProc;
    window_class.hInstance = GetModuleHandleW(nullptr);
    window_class.hIcon = LoadIconW(nullptr, (LPCWSTR)IDI_APPLICATION);
    window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    window_class.lpszClassName = L"d3d12_renderer";

    ATOM Atom = RegisterClassExW(&window_class);
    Assert(Atom);

    DWORD window_ex_style = WS_EX_APPWINDOW; //| WS_EX_NOREDIRECTIONBITMAP; // magic style to make DXGI_SWAP_EFFECT_FLIP_DISCARD not glitch on window resizing
    DWORD window_style = WS_OVERLAPPEDWINDOW;

    // Obtain the size of the border.
    RECT border_rect = { 0, 0, 0, 0 };
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    u32 window_pos_x = app->pos_x;
    u32 window_pos_y = app->pos_y;
    u32 window_width = app->client_width;
    u32 window_height = app->client_height;

    // In this case, the border rectangle is negative.
    window_pos_x += border_rect.left;
    window_pos_y += border_rect.top;

    // Grow by the size of the OS border.
    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    app->window_handle = CreateWindowExW(
        window_ex_style, window_class.lpszClassName, L"D3D12 Renderer", window_style,
        window_pos_x, window_pos_y, window_width, window_height,
        nullptr, nullptr, window_class.hInstance, nullptr);
    Assert(app->window_handle);

    ShowWindow(app->window_handle, SW_SHOWDEFAULT);

    return true;
}

bool run(Application* app)
{
    for (;;)
    {
        process_input();

#if RENDERER_DEBUG
        update_debug_stats(app->window_handle, app->frame_count, app->frequency, app->time);
#endif
    }
    return true;
}

void process_input()
{
    MSG message = {};

    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
        {
            ExitProcess(0);
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

void update_debug_stats(HWND window_handle, u32& frame_count, LARGE_INTEGER frequency, LARGE_INTEGER& time)
{
    RECT rect;
    GetClientRect(window_handle, &rect);

    DWORD rect_width  = rect.right - rect.left;
    DWORD rect_height = rect.bottom - rect.top;
    frame_count++;

    u64 update_title = time.QuadPart + frequency.QuadPart;

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (now.QuadPart > update_title)
    {
        update_title = now.QuadPart + frequency.QuadPart;

        double frames_per_second = (double)frame_count * frequency.QuadPart / (now.QuadPart - time.QuadPart);
        time = now;
        frame_count = 0;

        WCHAR title[1024];
        wsprintfW(title, L"D3D12 Renderer | Window Size: %dx%d | FPS: %d.%02d", rect_width, rect_height, 
            (s32)frames_per_second, (s32)(frames_per_second * 100) % 100);
        SetWindowTextW(window_handle, title);
    }
}