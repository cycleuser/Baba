#include <wayland-client.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../window.h"

struct BabaPlatformWindow {
    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_compositor* compositor;
    struct wl_surface* surface;
    struct wl_shell* shell;
    struct wl_shell_surface* shell_surface;
    struct wl_seat* seat;
    struct wl_pointer* pointer;
    struct wl_keyboard* keyboard;
    BabaWindowCallbacks callbacks;
    bool should_close;
    int width;
    int height;
    float mouse_x, mouse_y;
};

static void pointer_enter(void* data, struct wl_pointer* pointer,
                         uint32_t serial, struct wl_surface* surface,
                         wl_fixed_t sx, wl_fixed_t sy) {}
static void pointer_leave(void* data, struct wl_pointer* pointer,
                         uint32_t serial, struct wl_surface* surface) {}
static void pointer_motion(void* data, struct wl_pointer* pointer,
                          uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {
    BabaPlatformWindow* window = (BabaPlatformWindow*)data;
    window->mouse_x = wl_fixed_to_double(sx);
    window->mouse_y = wl_fixed_to_double(sy);
    if (window->callbacks.on_mouse_move) {
        window->callbacks.on_mouse_move(window->callbacks.userdata, 
            window->mouse_x, window->mouse_y);
    }
}
static void pointer_button(void* data, struct wl_pointer* pointer,
                          uint32_t serial, uint32_t time,
                          uint32_t button, uint32_t state) {
    BabaPlatformWindow* window = (BabaPlatformWindow*)data;
    if (window->callbacks.on_mouse_button) {
        int btn = (button == 0x110) ? 0 : (button == 0x112) ? 1 : 2;
        bool pressed = (state == WL_POINTER_BUTTON_STATE_PRESSED);
        window->callbacks.on_mouse_button(window->callbacks.userdata, btn, pressed);
    }
}
static void pointer_axis(void* data, struct wl_pointer* pointer,
                        uint32_t time, uint32_t axis, wl_fixed_t value) {}

static const struct wl_pointer_listener pointer_listener = {
    pointer_enter, pointer_leave, pointer_motion, pointer_button, pointer_axis,
};

static void keyboard_keymap(void* data, struct wl_keyboard* keyboard,
                           uint32_t format, int fd, uint32_t size) {
    close(fd);
}
static void keyboard_enter(void* data, struct wl_keyboard* keyboard,
                          uint32_t serial, struct wl_surface* surface,
                          struct wl_array* keys) {}
static void keyboard_leave(void* data, struct wl_keyboard* keyboard,
                          uint32_t serial, struct wl_surface* surface) {}
static void keyboard_key(void* data, struct wl_keyboard* keyboard,
                        uint32_t serial, uint32_t time,
                        uint32_t key, uint32_t state) {
    BabaPlatformWindow* window = (BabaPlatformWindow*)data;
    if (window->callbacks.on_key) {
        bool pressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
        window->callbacks.on_key(window->callbacks.userdata, (int)key, pressed);
    }
}
static void keyboard_modifiers(void* data, struct wl_keyboard* keyboard,
                              uint32_t serial, uint32_t mods_depressed,
                              uint32_t mods_latched, uint32_t mods_locked,
                              uint32_t group) {}

static const struct wl_keyboard_listener keyboard_listener = {
    keyboard_keymap, keyboard_enter, keyboard_leave, keyboard_key, keyboard_modifiers,
};

static void seat_capabilities(void* data, struct wl_seat* seat, uint32_t caps) {
    BabaPlatformWindow* window = (BabaPlatformWindow*)data;
    
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !window->pointer) {
        window->pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(window->pointer, &pointer_listener, window);
    }
    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !window->keyboard) {
        window->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(window->keyboard, &keyboard_listener, window);
    }
}

static const struct wl_seat_listener seat_listener = {
    seat_capabilities,
};

static void shell_surface_ping(void* data, struct wl_shell_surface* surface,
                              uint32_t serial) {
    wl_shell_surface_pong(surface, serial);
}
static void shell_surface_configure(void* data, struct wl_shell_surface* surface,
                                   uint32_t edges, int32_t width, int32_t height) {
    BabaPlatformWindow* window = (BabaPlatformWindow*)data;
    window->width = width;
    window->height = height;
    if (window->callbacks.on_resize) {
        window->callbacks.on_resize(window->callbacks.userdata, width, height);
    }
}
static void shell_surface_popup_done(void* data, struct wl_shell_surface* surface) {}

static const struct wl_shell_surface_listener shell_surface_listener = {
    shell_surface_ping, shell_surface_configure, shell_surface_popup_done,
};

static void registry_global(void* data, struct wl_registry* registry,
                           uint32_t id, const char* interface, uint32_t version) {
    BabaPlatformWindow* window = (BabaPlatformWindow*)data;
    
    if (strcmp(interface, "wl_compositor") == 0) {
        window->compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
    } else if (strcmp(interface, "wl_shell") == 0) {
        window->shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
    } else if (strcmp(interface, "wl_seat") == 0) {
        window->seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
        wl_seat_add_listener(window->seat, &seat_listener, window);
    }
}

static void registry_global_remove(void* data, struct wl_registry* registry, uint32_t id) {}

static const struct wl_registry_listener registry_listener = {
    registry_global, registry_global_remove,
};

BabaPlatformWindow* baba_platform_window_create(
    const char* title,
    int width,
    int height,
    BabaWindowCallbacks* callbacks
) {
    BabaPlatformWindow* window = calloc(1, sizeof(BabaPlatformWindow));
    if (!window) return NULL;
    
    window->display = wl_display_connect(NULL);
    if (!window->display) {
        free(window);
        return NULL;
    }
    
    window->width = width;
    window->height = height;
    window->should_close = false;
    
    if (callbacks) {
        window->callbacks = *callbacks;
    }
    
    window->registry = wl_display_get_registry(window->display);
    wl_registry_add_listener(window->registry, &registry_listener, window);
    wl_display_roundtrip(window->display);
    
    if (!window->compositor) {
        wl_display_disconnect(window->display);
        free(window);
        return NULL;
    }
    
    window->surface = wl_compositor_create_surface(window->compositor);
    
    if (window->shell) {
        window->shell_surface = wl_shell_get_shell_surface(window->shell, window->surface);
        wl_shell_surface_add_listener(window->shell_surface, &shell_surface_listener, window);
        wl_shell_surface_set_toplevel(window->shell_surface);
        wl_shell_surface_set_title(window->shell_surface, title);
    }
    
    return window;
}

void baba_platform_window_destroy(BabaPlatformWindow* window) {
    if (!window) return;
    
    if (window->keyboard) wl_keyboard_destroy(window->keyboard);
    if (window->pointer) wl_pointer_destroy(window->pointer);
    if (window->seat) wl_seat_destroy(window->seat);
    if (window->shell_surface) wl_shell_surface_destroy(window->shell_surface);
    if (window->shell) wl_shell_destroy(window->shell);
    if (window->surface) wl_surface_destroy(window->surface);
    if (window->compositor) wl_compositor_destroy(window->compositor);
    if (window->registry) wl_registry_destroy(window->registry);
    if (window->display) wl_display_disconnect(window->display);
    
    free(window);
}

void baba_platform_window_set_title(BabaPlatformWindow* window, const char* title) {
    if (!window || !title || !window->shell_surface) return;
    wl_shell_surface_set_title(window->shell_surface, title);
}

void baba_platform_window_get_size(BabaPlatformWindow* window, int* width, int* height) {
    if (!window) return;
    if (width) *width = window->width;
    if (height) *height = window->height;
}

void baba_platform_window_get_position(BabaPlatformWindow* window, int* x, int* y) {
    if (x) *x = 0;
    if (y) *y = 0;
}

void baba_platform_window_set_size(BabaPlatformWindow* window, int width, int height) {
    if (!window) return;
    window->width = width;
    window->height = height;
}

void baba_platform_window_set_position(BabaPlatformWindow* window, int x, int y) {
}

void baba_platform_window_show(BabaPlatformWindow* window) {
    if (!window) return;
    wl_surface_commit(window->surface);
    wl_display_flush(window->display);
}

void baba_platform_window_hide(BabaPlatformWindow* window) {
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
    
    VkWaylandSurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .display = window->display,
        .surface = window->surface,
    };
    
    VkSurfaceKHR surface;
    VkResult result = vkCreateWaylandSurfaceKHR(instance, &create_info, NULL, &surface);
    
    if (result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    
    return surface;
}

const char** baba_platform_get_required_vulkan_extensions(uint32_t* count) {
    static const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
    };
    if (count) *count = 2;
    return extensions;
}

double baba_platform_get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

void baba_platform_poll_events(void) {
}