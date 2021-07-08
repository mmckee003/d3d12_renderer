#include "core/core_types.h"

#include <windows.h>
#include <cstdint>

#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)

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

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
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

    u32 x      = 100;
    u32 y      = 100;
    u32 width  = 1280;
    u32 height = 720;

    u32 client_x      = x;
    u32 client_y      = y;
    u32 client_width  = width;
    u32 client_height = height;

    u32 window_x      = client_x;
    u32 window_y      = client_y;
    u32 window_width  = client_width;
    u32 window_height = client_height;

    DWORD window_ex_style = WS_EX_APPWINDOW; //| WS_EX_NOREDIRECTIONBITMAP; // magic style to make DXGI_SWAP_EFFECT_FLIP_DISCARD not glitch on window resizing
    DWORD window_style = WS_OVERLAPPEDWINDOW;

    // Obtain the size of the border.
    RECT border_rect = {0, 0, 0, 0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    // In this case, the border rectangle is negative.
    window_x += border_rect.left;
    window_y += border_rect.top;

    // Grow by the size of the OS border.
    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    HWND window = CreateWindowExW(
        window_ex_style, window_class.lpszClassName, L"D3D12 Renderer", window_style,
        window_x, window_y, window_width, window_height,
        nullptr, nullptr, window_class.hInstance, nullptr);
    Assert(window);

    ShowWindow(window, SW_SHOWDEFAULT);

    LARGE_INTEGER frequency, time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&time);

    uint32_t frames = 0;
    int64_t update_title = time.QuadPart + frequency.QuadPart; 

    for (;;)
    {
        // Handle Win32 events and input.
        MSG message = {};

        while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
                ExitProcess(0);
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        RECT rect;
        GetClientRect(window, &rect);

        DWORD rect_width  = rect.right - rect.left;
        DWORD rect_height = rect.bottom - rect.top;
        frames++;

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        if (now.QuadPart > update_title)
        {
            update_title = now.QuadPart + frequency.QuadPart;

            double frames_per_second = (double)frames * frequency.QuadPart / (now.QuadPart - time.QuadPart);
            time = now;
            frames = 0;

            WCHAR title[1024];
            wsprintfW(title, L"D3D12 Renderer | Window Size: %dx%d | FPS: %d.%02d", rect_width, rect_height, (int)frames_per_second, (int)(frames_per_second*100) % 100);
            SetWindowTextW(window, title); 
        }
    }
    return 0;
}