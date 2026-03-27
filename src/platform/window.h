#ifndef BABA_PLATFORM_WINDOW_H
#define BABA_PLATFORM_WINDOW_H

#include "../core/types.h"
#include <vulkan/vulkan.h>

typedef struct BabaPlatformWindow BabaPlatformWindow;

typedef struct {
    int (*on_resize)(void* userdata, int width, int height);
    int (*on_close)(void* userdata);
    int (*on_mouse_move)(void* userdata, float x, float y);
    int (*on_mouse_button)(void* userdata, int button, bool pressed);
    int (*on_key)(void* userdata, int keycode, bool pressed);
    int (*on_char)(void* userdata, uint32_t codepoint);
    void* userdata;
} BabaWindowCallbacks;

BabaPlatformWindow* baba_platform_window_create(
    const char* title,
    int width,
    int height,
    BabaWindowCallbacks* callbacks
);

void baba_platform_window_destroy(BabaPlatformWindow* window);

void baba_platform_window_set_title(BabaPlatformWindow* window, const char* title);
void baba_platform_window_get_size(BabaPlatformWindow* window, int* width, int* height);
void baba_platform_window_get_position(BabaPlatformWindow* window, int* x, int* y);
void baba_platform_window_set_size(BabaPlatformWindow* window, int width, int height);
void baba_platform_window_set_position(BabaPlatformWindow* window, int x, int y);
void baba_platform_window_show(BabaPlatformWindow* window);
void baba_platform_window_hide(BabaPlatformWindow* window);
void baba_platform_window_close(BabaPlatformWindow* window);

VkSurfaceKHR baba_platform_window_create_surface(
    BabaPlatformWindow* window,
    VkInstance instance
);

const char** baba_platform_get_required_vulkan_extensions(uint32_t* count);
double baba_platform_get_time(void);
void baba_platform_poll_events(void);
bool baba_platform_window_should_close(BabaPlatformWindow* window);

#endif