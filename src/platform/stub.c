/*
 * Baba GUI - Platform Stub for Cross-Compilation
 * 
 * This file provides stub implementations for cross-compilation.
 * The actual platform implementation must be linked at runtime on the target system.
 */

#include "window.h"
#include <stdlib.h>

BabaPlatformWindow* baba_platform_window_create(
    const char* title,
    int width,
    int height,
    BabaWindowCallbacks* callbacks
) {
    (void)title; (void)width; (void)height; (void)callbacks;
    return NULL;
}

void baba_platform_window_destroy(BabaPlatformWindow* window) {
    (void)window;
}

void baba_platform_window_set_title(BabaPlatformWindow* window, const char* title) {
    (void)window; (void)title;
}

void baba_platform_window_get_size(BabaPlatformWindow* window, int* width, int* height) {
    (void)window;
    if (width) *width = 0;
    if (height) *height = 0;
}

void baba_platform_window_get_position(BabaPlatformWindow* window, int* x, int* y) {
    (void)window;
    if (x) *x = 0;
    if (y) *y = 0;
}

void baba_platform_window_set_size(BabaPlatformWindow* window, int width, int height) {
    (void)window; (void)width; (void)height;
}

void baba_platform_window_set_position(BabaPlatformWindow* window, int x, int y) {
    (void)window; (void)x; (void)y;
}

void baba_platform_window_show(BabaPlatformWindow* window) {
    (void)window;
}

void baba_platform_window_hide(BabaPlatformWindow* window) {
    (void)window;
}

void baba_platform_window_close(BabaPlatformWindow* window) {
    (void)window;
}

VkSurfaceKHR baba_platform_window_create_surface(
    BabaPlatformWindow* window,
    VkInstance instance
) {
    (void)window; (void)instance;
    return VK_NULL_HANDLE;
}

const char** baba_platform_get_required_vulkan_extensions(uint32_t* count) {
    if (count) *count = 0;
    return NULL;
}

double baba_platform_get_time(void) {
    return 0.0;
}

void baba_platform_poll_events(void) {
}

bool baba_platform_window_should_close(BabaPlatformWindow* window) {
    (void)window;
    return false;
}