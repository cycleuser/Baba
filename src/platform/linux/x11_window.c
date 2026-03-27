#define VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../platform/window.h"

struct BabaPlatformWindow {
    Display* display;
    Window window;
    Atom wm_delete_window;
    BabaWindowCallbacks callbacks;
    bool should_close;
    int width;
    int height;
};

static int X11ErrorHandler(Display* display, XErrorEvent* event) {
    return 0;
}

BabaPlatformWindow* baba_platform_window_create(
    const char* title,
    int width,
    int height,
    BabaWindowCallbacks* callbacks
) {
    BabaPlatformWindow* window = calloc(1, sizeof(BabaPlatformWindow));
    if (!window) return NULL;
    
    window->display = XOpenDisplay(NULL);
    if (!window->display) {
        free(window);
        return NULL;
    }
    
    XSetErrorHandler(X11ErrorHandler);
    
    int screen = DefaultScreen(window->display);
    Window root = RootWindow(window->display, screen);
    
    int visual_depth = DefaultDepth(window->display, screen);
    Visual* visual = DefaultVisual(window->display, screen);
    
    XSetWindowAttributes attrs = {
        .background_pixel = BlackPixel(window->display, screen),
        .border_pixel = BlackPixel(window->display, screen),
        .event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | 
                      ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                      StructureNotifyMask,
        .colormap = XCreateColormap(window->display, root, visual, AllocNone),
    };
    
    unsigned long attr_mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;
    
    window->window = XCreateWindow(
        window->display, root,
        0, 0, width, height,
        0, visual_depth, InputOutput, visual,
        attr_mask, &attrs
    );
    
    if (!window->window) {
        XCloseDisplay(window->display);
        free(window);
        return NULL;
    }
    
    XStoreName(window->display, window->window, title);
    
    window->wm_delete_window = XInternAtom(window->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(window->display, window->window, &window->wm_delete_window, 1);
    
    XSizeHints* size_hints = XAllocSizeHints();
    size_hints->flags = PMinSize | PMaxSize;
    size_hints->min_width = 100;
    size_hints->min_height = 100;
    size_hints->max_width = 32768;
    size_hints->max_height = 32768;
    XSetWMNormalHints(window->display, window->window, size_hints);
    XFree(size_hints);
    
    window->width = width;
    window->height = height;
    window->should_close = false;
    
    if (callbacks) {
        window->callbacks = *callbacks;
    }
    
    return window;
}

void baba_platform_window_destroy(BabaPlatformWindow* window) {
    if (!window) return;
    
    if (window->window) {
        XDestroyWindow(window->display, window->window);
    }
    if (window->display) {
        XCloseDisplay(window->display);
    }
    free(window);
}

void baba_platform_window_set_title(BabaPlatformWindow* window, const char* title) {
    if (!window || !title) return;
    XStoreName(window->display, window->window, title);
}

void baba_platform_window_get_size(BabaPlatformWindow* window, int* width, int* height) {
    if (!window) return;
    if (width) *width = window->width;
    if (height) *height = window->height;
}

void baba_platform_window_get_position(BabaPlatformWindow* window, int* x, int* y) {
    if (!window) return;
    XWindowAttributes attrs;
    XGetWindowAttributes(window->display, window->window, &attrs);
    if (x) *x = attrs.x;
    if (y) *y = attrs.y;
}

void baba_platform_window_set_size(BabaPlatformWindow* window, int width, int height) {
    if (!window) return;
    XResizeWindow(window->display, window->window, width, height);
    window->width = width;
    window->height = height;
}

void baba_platform_window_set_position(BabaPlatformWindow* window, int x, int y) {
    if (!window) return;
    XMoveWindow(window->display, window->window, x, y);
}

void baba_platform_window_show(BabaPlatformWindow* window) {
    if (!window) return;
    XMapWindow(window->display, window->window);
    XFlush(window->display);
}

void baba_platform_window_hide(BabaPlatformWindow* window) {
    if (!window) return;
    XUnmapWindow(window->display, window->window);
    XFlush(window->display);
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
    
    VkXlibSurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .dpy = window->display,
        .window = window->window,
    };
    
    VkSurfaceKHR surface;
    VkResult result = vkCreateXlibSurfaceKHR(instance, &create_info, NULL, &surface);
    
    if (result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    
    return surface;
}

const char** baba_platform_get_required_vulkan_extensions(uint32_t* count) {
    static const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
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

void baba_platform_process_events(BabaPlatformWindow* window) {
    if (!window || !window->display) return;
    
    XEvent event;
    while (XPending(window->display)) {
        XNextEvent(window->display, &event);
        
        switch (event.type) {
            case ClientMessage:
                if ((Atom)event.xclient.data.l[0] == window->wm_delete_window) {
                    window->should_close = true;
                    if (window->callbacks.on_close) {
                        window->callbacks.on_close(window->callbacks.userdata);
                    }
                }
                break;
                
            case ConfigureNotify:
                if (event.xconfigure.width != window->width || 
                    event.xconfigure.height != window->height) {
                    window->width = event.xconfigure.width;
                    window->height = event.xconfigure.height;
                    if (window->callbacks.on_resize) {
                        window->callbacks.on_resize(window->callbacks.userdata, 
                            window->width, window->height);
                    }
                }
                break;
                
            case MotionNotify:
                if (window->callbacks.on_mouse_move) {
                    window->callbacks.on_mouse_move(window->callbacks.userdata,
                        (float)event.xmotion.x, (float)event.xmotion.y);
                }
                break;
                
            case ButtonPress:
            case ButtonRelease:
                if (window->callbacks.on_mouse_button) {
                    int button = 0;
                    switch (event.xbutton.button) {
                        case Button1: button = 0; break;
                        case Button3: button = 1; break;
                        case Button2: button = 2; break;
                    }
                    bool pressed = (event.type == ButtonPress);
                    window->callbacks.on_mouse_button(window->callbacks.userdata, button, pressed);
                }
                break;
                
            case KeyPress:
            case KeyRelease:
                if (window->callbacks.on_key) {
                    KeySym keysym = XLookupKeysym(&event.xkey, 0);
                    bool pressed = (event.type == KeyPress);
                    window->callbacks.on_key(window->callbacks.userdata, (int)keysym, pressed);
                }
                break;
        }
    }
}