#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <stdlib.h>
#include <string.h>
#include "../window.h"

struct BabaPlatformWindow {
    HWND hwnd;
    HINSTANCE hinstance;
    BabaWindowCallbacks callbacks;
    bool should_close;
    int width;
    int height;
};

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    BabaPlatformWindow* window = (BabaPlatformWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (msg) {
        case WM_CLOSE:
            window->should_close = true;
            if (window->callbacks.on_close) {
                window->callbacks.on_close(window->callbacks.userdata);
            }
            return 0;
            
        case WM_SIZE:
            window->width = LOWORD(lParam);
            window->height = HIWORD(lParam);
            if (window->callbacks.on_resize) {
                window->callbacks.on_resize(window->callbacks.userdata, window->width, window->height);
            }
            return 0;
            
        case WM_MOUSEMOVE:
            if (window->callbacks.on_mouse_move) {
                float x = (float)LOWORD(lParam);
                float y = (float)HIWORD(lParam);
                window->callbacks.on_mouse_move(window->callbacks.userdata, x, y);
            }
            return 0;
            
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            if (window->callbacks.on_mouse_button) {
                bool pressed = (msg == WM_LBUTTONDOWN);
                window->callbacks.on_mouse_button(window->callbacks.userdata, 0, pressed);
            }
            return 0;
            
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
            if (window->callbacks.on_mouse_button) {
                bool pressed = (msg == WM_RBUTTONDOWN);
                window->callbacks.on_mouse_button(window->callbacks.userdata, 1, pressed);
            }
            return 0;
            
        case WM_KEYDOWN:
        case WM_KEYUP:
            if (window->callbacks.on_key) {
                bool pressed = (msg == WM_KEYDOWN);
                window->callbacks.on_key(window->callbacks.userdata, (int)wParam, pressed);
            }
            return 0;
            
        case WM_CHAR:
            if (window->callbacks.on_char && wParam >= 32) {
                window->callbacks.on_char(window->callbacks.userdata, (uint32_t)wParam);
            }
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static const char* WINDOW_CLASS_NAME = "BabaWindowClass";
static bool g_class_registered = false;

static void RegisterWindowClass(HINSTANCE hinstance) {
    if (g_class_registered) return;
    
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    RegisterClassEx(&wc);
    g_class_registered = true;
}

BabaPlatformWindow* baba_platform_window_create(
    const char* title,
    int width,
    int height,
    BabaWindowCallbacks* callbacks
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
    
    int window_width = width;
    int window_height = height;
    
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    window_width = rect.right - rect.left;
    window_height = rect.bottom - rect.top;
    
    int title_len = MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
    wchar_t* wtitle = malloc(title_len * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, title_len);
    
    window->hwnd = CreateWindowExA(
        0,
        WINDOW_CLASS_NAME,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        window_width, window_height,
        NULL, NULL,
        window->hinstance,
        NULL
    );
    
    free(wtitle);
    
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
    if (!window || !title) return;
    SetWindowTextA(window->hwnd, title);
}

void baba_platform_window_get_size(BabaPlatformWindow* window, int* width, int* height) {
    if (!window) return;
    if (width) *width = window->width;
    if (height) *height = window->height;
}

void baba_platform_window_get_position(BabaPlatformWindow* window, int* x, int* y) {
    if (!window) return;
    RECT rect;
    GetWindowRect(window->hwnd, &rect);
    if (x) *x = rect.left;
    if (y) *y = rect.top;
}

void baba_platform_window_set_size(BabaPlatformWindow* window, int width, int height) {
    if (!window) return;
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    SetWindowPos(window->hwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
}

void baba_platform_window_set_position(BabaPlatformWindow* window, int x, int y) {
    if (!window) return;
    SetWindowPos(window->hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void baba_platform_window_show(BabaPlatformWindow* window) {
    if (!window) return;
    ShowWindow(window->hwnd, SW_SHOW);
    UpdateWindow(window->hwnd);
}

void baba_platform_window_hide(BabaPlatformWindow* window) {
    if (!window) return;
    ShowWindow(window->hwnd, SW_HIDE);
}

void baba_platform_window_close(BabaPlatformWindow* window) {
    if (!window) return;
    window->should_close = true;
}

bool baba_platform_window_should_close(BabaPlatformWindow* window) {
    return window ? window->should_close : true;
}

VkSurfaceKHR baba_platform_window_create_surface(
    BabaPlatformWindow* window,
    VkInstance instance
) {
    if (!window || !instance) return VK_NULL_HANDLE;
    
    VkWin32SurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .hinstance = window->hinstance,
        .hwnd = window->hwnd,
    };
    
    VkSurfaceKHR surface;
    VkResult result = vkCreateWin32SurfaceKHR(instance, &create_info, NULL, &surface);
    
    if (result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    
    return surface;
}

const char** baba_platform_get_required_vulkan_extensions(uint32_t* count) {
    static const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    };
    if (count) *count = 2;
    return extensions;
}

double baba_platform_get_time(void) {
    static LARGE_INTEGER frequency = {0};
    static bool initialized = false;
    
    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        initialized = true;
    }
    
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
}

void baba_platform_poll_events(void) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}