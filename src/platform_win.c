#include "platform.h"
#include "config.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <string.h>

static HWND g_window;
static BITMAPINFO g_bitmap_info;
static unsigned long g_rgb_buffer[SCREEN_PIXELS];
static LARGE_INTEGER g_perf_frequency;
static int g_has_perf_counter;
static int g_mouse_centered;

static void center_mouse_in_client(void)
{
    POINT center_point;
    RECT client_rect;

    if (g_window == 0) {
        return;
    }

    GetClientRect(g_window, &client_rect);
    center_point.x = (client_rect.right - client_rect.left) / 2;
    center_point.y = (client_rect.bottom - client_rect.top) / 2;
    ClientToScreen(g_window, &center_point);
    SetCursorPos(center_point.x, center_point.y);
}

static int key_is_down(ConfigKey key)
{
    int virtual_key;

    virtual_key = 0;
    switch (key) {
    case CONFIG_KEY_W: virtual_key = 'W'; break;
    case CONFIG_KEY_A: virtual_key = 'A'; break;
    case CONFIG_KEY_S: virtual_key = 'S'; break;
    case CONFIG_KEY_D: virtual_key = 'D'; break;
    case CONFIG_KEY_M: virtual_key = 'M'; break;
    case CONFIG_KEY_Q: virtual_key = 'Q'; break;
    case CONFIG_KEY_E: virtual_key = 'E'; break;
    case CONFIG_KEY_F: virtual_key = 'F'; break;
    case CONFIG_KEY_SPACE: virtual_key = VK_SPACE; break;
    case CONFIG_KEY_TAB: virtual_key = VK_TAB; break;
    case CONFIG_KEY_ESCAPE: virtual_key = VK_ESCAPE; break;
    case CONFIG_KEY_UP: virtual_key = VK_UP; break;
    case CONFIG_KEY_DOWN: virtual_key = VK_DOWN; break;
    case CONFIG_KEY_LEFT: virtual_key = VK_LEFT; break;
    case CONFIG_KEY_RIGHT: virtual_key = VK_RIGHT; break;
    default: break;
    }

    return virtual_key != 0 && (GetAsyncKeyState(virtual_key) & 0x8000) ? 1 : 0;
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT:
    {
        PAINTSTRUCT paint;

        BeginPaint(hwnd, &paint);
        EndPaint(hwnd, &paint);
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_ACTIVATE:
        g_mouse_centered = 0;
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, message, wparam, lparam);
    }
}

int platform_init(const char *title, const PaletteColor *palette)
{
    WNDCLASSA wc;
    RECT rect;
    HINSTANCE instance;

    (void)palette;
    instance = GetModuleHandle(0);
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = window_proc;
    wc.hInstance = instance;
    wc.lpszClassName = "Mode13hWindowClass";
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = 0;

    if (!RegisterClassA(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        fprintf(stderr, "RegisterClassA failed: %lu\n", (unsigned long)GetLastError());
        return 0;
    }

    rect.left = 0;
    rect.top = 0;
    rect.right = 1280;
    rect.bottom = 800;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    g_window = CreateWindowA(
        wc.lpszClassName,
        title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        0,
        0,
        instance,
        0);

    if (g_window == 0) {
        fprintf(stderr, "CreateWindowA failed: %lu\n", (unsigned long)GetLastError());
        return 0;
    }

    memset(&g_bitmap_info, 0, sizeof(g_bitmap_info));
    g_bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    g_bitmap_info.bmiHeader.biWidth = SCREEN_WIDTH;
    g_bitmap_info.bmiHeader.biHeight = -SCREEN_HEIGHT;
    g_bitmap_info.bmiHeader.biPlanes = 1;
    g_bitmap_info.bmiHeader.biBitCount = 32;
    g_bitmap_info.bmiHeader.biCompression = BI_RGB;
    g_has_perf_counter = QueryPerformanceFrequency(&g_perf_frequency) ? 1 : 0;
    return 1;
}

void platform_shutdown(void)
{
    if (g_window != 0) {
        DestroyWindow(g_window);
        g_window = 0;
    }
}

void platform_poll_input(PlatformInput *input)
{
    const GameConfig *config;
    int turn_left_down;
    int turn_right_down;
    int strafe_left_down;
    int strafe_right_down;
    MSG msg;

    memset(input, 0, sizeof(*input));
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            input->quit = 1;
            return;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    config = config_get();
    turn_left_down = key_is_down(config->turn_left);
    turn_right_down = key_is_down(config->turn_right);
    strafe_left_down = key_is_down(config->strafe_left);
    strafe_right_down = key_is_down(config->strafe_right);
    input->quit = key_is_down(config->quit);
    input->forward = key_is_down(config->forward);
    input->backward = key_is_down(config->backward);
    if (config->mouse_look) {
        input->turn_left = 0;
        input->turn_right = 0;
        input->strafe_left = strafe_left_down || turn_left_down;
        input->strafe_right = strafe_right_down || turn_right_down;
    } else {
        input->turn_left = turn_left_down;
        input->turn_right = turn_right_down;
        input->strafe_left = strafe_left_down;
        input->strafe_right = strafe_right_down;
    }
    input->use = key_is_down(config->use);
    input->fire = key_is_down(config->fire);
    input->next_weapon = key_is_down(config->next_weapon);
    input->toggle_minimap = key_is_down(config->toggle_minimap);
    input->mouse_delta_x = 0;

    if (config->mouse_fire && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
        input->fire = 1;
    }

    if (config->mouse_look && GetForegroundWindow() == g_window) {
        POINT cursor_point;
        POINT center_point;
        RECT client_rect;

        GetClientRect(g_window, &client_rect);
        center_point.x = (client_rect.right - client_rect.left) / 2;
        center_point.y = (client_rect.bottom - client_rect.top) / 2;
        ClientToScreen(g_window, &center_point);

        GetCursorPos(&cursor_point);
        if (g_mouse_centered) {
            input->mouse_delta_x = cursor_point.x - center_point.x;
        }
        center_mouse_in_client();
        g_mouse_centered = 1;
    } else {
        g_mouse_centered = 0;
    }
}

void platform_present(const u8 *framebuffer, const PaletteColor *palette)
{
    HDC dc;
    HDC memory_dc;
    HBITMAP back_buffer;
    HBITMAP old_bitmap;
    RECT rect;
    int client_width;
    int client_height;
    int scale_x;
    int scale_y;
    int scale;
    int dest_width;
    int dest_height;
    int dest_x;
    int dest_y;
    int i;

    if (g_window == 0) {
        return;
    }

    for (i = 0; i < SCREEN_PIXELS; ++i) {
        const PaletteColor *color;

        color = &palette[framebuffer[i]];
        g_rgb_buffer[i] = ((unsigned long)color->b << 16) |
                          ((unsigned long)color->g << 8) |
                          ((unsigned long)color->r);
    }

    GetClientRect(g_window, &rect);
    client_width = rect.right - rect.left;
    client_height = rect.bottom - rect.top;
    scale_x = client_width / SCREEN_WIDTH;
    scale_y = client_height / SCREEN_HEIGHT;
    scale = scale_x < scale_y ? scale_x : scale_y;
    if (scale < 1) {
        scale = 1;
    }

    dest_width = SCREEN_WIDTH * scale;
    dest_height = SCREEN_HEIGHT * scale;
    dest_x = (client_width - dest_width) / 2;
    dest_y = (client_height - dest_height) / 2;

    dc = GetDC(g_window);
    memory_dc = CreateCompatibleDC(dc);
    if (memory_dc == 0) {
        ReleaseDC(g_window, dc);
        return;
    }

    back_buffer = CreateCompatibleBitmap(dc, client_width, client_height);
    if (back_buffer == 0) {
        DeleteDC(memory_dc);
        ReleaseDC(g_window, dc);
        return;
    }

    old_bitmap = (HBITMAP)SelectObject(memory_dc, back_buffer);
    FillRect(memory_dc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
    SetStretchBltMode(memory_dc, COLORONCOLOR);
    StretchDIBits(
        memory_dc,
        dest_x,
        dest_y,
        dest_width,
        dest_height,
        0,
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        g_rgb_buffer,
        &g_bitmap_info,
        DIB_RGB_COLORS,
        SRCCOPY);

    BitBlt(dc, 0, 0, client_width, client_height, memory_dc, 0, 0, SRCCOPY);
    SelectObject(memory_dc, old_bitmap);
    DeleteObject(back_buffer);
    DeleteDC(memory_dc);
    ReleaseDC(g_window, dc);
}

u32 platform_ticks_ms(void)
{
    if (g_has_perf_counter) {
        LARGE_INTEGER counter;

        QueryPerformanceCounter(&counter);
        return (u32)((counter.QuadPart * 1000ULL) / g_perf_frequency.QuadPart);
    }

    return (u32)GetTickCount();
}

void platform_sleep_ms(u32 ms)
{
    if (ms <= 1) {
        Sleep(0);
    } else {
        Sleep(ms);
    }
}
