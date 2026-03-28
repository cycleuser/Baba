#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "../platform.h"

struct BabaPlatformWindow {
    HWND hwnd;
    HINSTANCE hinstance;
    BabaPlatformCallbacks callbacks;
    bool should_close;
    int width;
    int height;
};

static const char* WINDOW_CLASS_NAME = "BabaWindowClass";
static bool g_class_registered = false;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    BabaPlatformWindow* window = (BabaPlatformWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (msg) {
        case WM_CLOSE:
            if (window) {
                window->should_close = true;
                if (window->callbacks.on_close) {
                    if (window->callbacks.on_close(window->callbacks.userdata)) {
                        DestroyWindow(hwnd);
                    }
                } else {
                    DestroyWindow(hwnd);
                }
            }
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        case WM_SIZE:
            if (window) {
                window->width = LOWORD(lParam);
                window->height = HIWORD(lParam);
                if (window->callbacks.on_resize) {
                    window->callbacks.on_resize(
                        window->callbacks.userdata,
                        window->width,
                        window->height
                    );
                }
            }
            break;
            
        case WM_MOUSEMOVE:
            if (window && window->callbacks.on_mouse) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                window->callbacks.on_mouse(window->callbacks.userdata, (float)x, (float)y, 0, false);
            }
            break;
            
        case WM_LBUTTONDOWN:
            if (window && window->callbacks.on_mouse) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                window->callbacks.on_mouse(window->callbacks.userdata, (float)x, (float)y, 1, true);
            }
            break;
            
        case WM_LBUTTONUP:
            if (window && window->callbacks.on_mouse) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                window->callbacks.on_mouse(window->callbacks.userdata, (float)x, (float)y, 1, false);
            }
            break;
            
        case WM_KEYDOWN:
            if (window && window->callbacks.on_key) {
                window->callbacks.on_key(window->callbacks.userdata, (int)wParam, true);
            }
            break;
            
        case WM_KEYUP:
            if (window && window->callbacks.on_key) {
                window->callbacks.on_key(window->callbacks.userdata, (int)wParam, false);
            }
            break;
    }
    
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

static void RegisterWindowClass(HINSTANCE hinstance) {
    if (g_class_registered) return;
    
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    RegisterClassExA(&wc);
    g_class_registered = true;
}

BabaPlatformWindow* baba_platform_window_create(
    const char* title,
    int width,
    int height,
    BabaPlatformCallbacks* callbacks
) {
    BabaPlatformWindow* window = calloc(1, sizeof(BabaPlatformWindow));
    if (!window) return NULL;
    
    window->hinstance = GetModuleHandle(NULL);
    window->width = width;
    window->height = height;
    window->should_close = false;
    
    if (callbacks) {
        window->callbacks = *callbacks;
    }
    
    RegisterWindowClass(window->hinstance);
    
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    int window_width = rect.right - rect.left;
    int window_height = rect.bottom - rect.top;
    
    window->hwnd = CreateWindowExA(
        0,
        WINDOW_CLASS_NAME,
        title ? title : "Baba",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        window_width, window_height,
        NULL, NULL,
        window->hinstance,
        NULL
    );
    
    if (!window->hwnd) {
        free(window);
        return NULL;
    }
    
    SetWindowLongPtr(window->hwnd, GWLP_USERDATA, (LONG_PTR)window);
    
    return window;
}

void baba_platform_window_destroy(BabaPlatformWindow* window) {
    if (!window) return;
    if (window->hwnd) {
        DestroyWindow(window->hwnd);
    }
    free(window);
}

void baba_platform_window_set_title(BabaPlatformWindow* window, const char* title) {
    if (!window || !window->hwnd || !title) return;
    SetWindowTextA(window->hwnd, title);
}

void baba_platform_window_get_size(BabaPlatformWindow* window, int* width, int* height) {
    if (!window) return;
    if (width) *width = window->width;
    if (height) *height = window->height;
}

void baba_platform_window_set_size(BabaPlatformWindow* window, int width, int height) {
    if (!window || !window->hwnd) return;
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    SetWindowPos(window->hwnd, NULL, 0, 0, 
                 rect.right - rect.left, rect.bottom - rect.top,
                 SWP_NOMOVE | SWP_NOZORDER);
    window->width = width;
    window->height = height;
}

void baba_platform_window_get_position(BabaPlatformWindow* window, int* x, int* y) {
    if (!window || !window->hwnd) return;
    RECT rect;
    GetWindowRect(window->hwnd, &rect);
    if (x) *x = rect.left;
    if (y) *y = rect.top;
}

void baba_platform_window_set_position(BabaPlatformWindow* window, int x, int y) {
    if (!window || !window->hwnd) return;
    SetWindowPos(window->hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void baba_platform_window_show(BabaPlatformWindow* window) {
    if (!window || !window->hwnd) return;
    ShowWindow(window->hwnd, SW_SHOW);
    UpdateWindow(window->hwnd);
}

void baba_platform_window_hide(BabaPlatformWindow* window) {
    if (!window || !window->hwnd) return;
    ShowWindow(window->hwnd, SW_HIDE);
}

void baba_platform_window_close(BabaPlatformWindow* window) {
    if (!window) return;
    window->should_close = true;
    if (window->hwnd) {
        PostMessage(window->hwnd, WM_CLOSE, 0, 0);
    }
}

void* baba_platform_window_get_native(BabaPlatformWindow* window) {
    return window ? (void*)window->hwnd : NULL;
}

void baba_platform_window_invalidate(BabaPlatformWindow* window) {
    if (!window || !window->hwnd) return;
    InvalidateRect(window->hwnd, NULL, TRUE);
}

static LARGE_INTEGER g_timer_freq = {0};
static LARGE_INTEGER g_timer_start = {0};

double baba_platform_get_time(void) {
    if (g_timer_freq.QuadPart == 0) {
        QueryPerformanceFrequency(&g_timer_freq);
        QueryPerformanceCounter(&g_timer_start);
    }
    
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)(now.QuadPart - g_timer_start.QuadPart) / (double)g_timer_freq.QuadPart;
}

void baba_platform_poll_events(void) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool baba_platform_window_should_close(BabaPlatformWindow* window) {
    return window ? window->should_close : true;
}