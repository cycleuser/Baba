#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "../platform.h"

struct BabaPlatformWindow {
    Display* display;
    Window window;
    Atom wm_delete_window;
    BabaPlatformCallbacks callbacks;
    bool should_close;
    int width;
    int height;
    float mouse_x;
    float mouse_y;
};

static int X11ErrorHandler(Display* display, XErrorEvent* event) {
    return 0;
}

BabaPlatformWindow* baba_platform_window_create(
    const char* title,
    int width,
    int height,
    BabaPlatformCallbacks* callbacks
) {
    BabaPlatformWindow* window = calloc(1, sizeof(BabaPlatformWindow));
    if (!window) return NULL;
    
    if (callbacks) {
        window->callbacks = *callbacks;
    }
    window->width = width;
    window->height = height;
    
    window->display = XOpenDisplay(NULL);
    if (!window->display) {
        free(window);
        return NULL;
    }
    
    XSetErrorHandler(X11ErrorHandler);
    
    int screen = DefaultScreen(window->display);
    Window root = RootWindow(window->display, screen);
    
    window->window = XCreateSimpleWindow(
        window->display,
        root,
        0, 0,
        width, height,
        0,
        BlackPixel(window->display, screen),
        BlackPixel(window->display, screen)
    );
    
    if (!window->window) {
        XCloseDisplay(window->display);
        free(window);
        return NULL;
    }
    
    XStoreName(window->display, window->window, title ? title : "Baba");
    
    window->wm_delete_window = XInternAtom(window->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(window->display, window->window, &window->wm_delete_window, 1);
    
    XSelectInput(window->display, window->window,
        ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
        StructureNotifyMask
    );
    
    XMapWindow(window->display, window->window);
    XFlush(window->display);
    
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
    if (!window || !window->display || !window->window || !title) return;
    XStoreName(window->display, window->window, title);
    XFlush(window->display);
}

void baba_platform_window_get_size(BabaPlatformWindow* window, int* width, int* height) {
    if (!window) return;
    if (width) *width = window->width;
    if (height) *height = window->height;
}

void baba_platform_window_set_size(BabaPlatformWindow* window, int width, int height) {
    if (!window || !window->display || !window->window) return;
    XResizeWindow(window->display, window->window, width, height);
    window->width = width;
    window->height = height;
    XFlush(window->display);
}

void baba_platform_window_get_position(BabaPlatformWindow* window, int* x, int* y) {
    if (!window || !window->display || !window->window) return;
    Window root;
    int win_x, win_y;
    unsigned int w, h, border, depth;
    XGetGeometry(window->display, window->window, &root, &win_x, &win_y, &w, &h, &border, &depth);
    if (x) *x = win_x;
    if (y) *y = win_y;
}

void baba_platform_window_set_position(BabaPlatformWindow* window, int x, int y) {
    if (!window || !window->display || !window->window) return;
    XMoveWindow(window->display, window->window, x, y);
    XFlush(window->display);
}

void baba_platform_window_show(BabaPlatformWindow* window) {
    if (!window || !window->display || !window->window) return;
    XMapWindow(window->display, window->window);
    XFlush(window->display);
}

void baba_platform_window_hide(BabaPlatformWindow* window) {
    if (!window || !window->display || !window->window) return;
    XUnmapWindow(window->display, window->window);
    XFlush(window->display);
}

void baba_platform_window_close(BabaPlatformWindow* window) {
    if (!window) return;
    window->should_close = true;
}

void* baba_platform_window_get_native(BabaPlatformWindow* window) {
    return window ? (void*)window->window : NULL;
}

void baba_platform_window_invalidate(BabaPlatformWindow* window) {
    if (!window || !window->display || !window->window) return;
    XClearArea(window->display, window->window, 0, 0, 0, 0, True);
    XFlush(window->display);
}

double baba_platform_get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

void baba_platform_poll_events(void) {
}

bool baba_platform_window_should_close(BabaPlatformWindow* window) {
    if (!window || !window->display) return true;
    
    XEvent event;
    while (XPending(window->display) > 0) {
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
                if (window->callbacks.on_resize) {
                    window->width = event.xconfigure.width;
                    window->height = event.xconfigure.height;
                    window->callbacks.on_resize(
                        window->callbacks.userdata,
                        window->width,
                        window->height
                    );
                }
                break;
                
            case KeyPress:
            case KeyRelease:
                if (window->callbacks.on_key) {
                    KeySym keysym = XLookupKeysym(&event.xkey, 0);
                    window->callbacks.on_key(
                        window->callbacks.userdata,
                        (int)keysym,
                        event.type == KeyPress
                    );
                }
                break;
                
            case ButtonPress:
            case ButtonRelease:
                if (window->callbacks.on_mouse) {
                    window->callbacks.on_mouse(
                        window->callbacks.userdata,
                        window->mouse_x,
                        window->mouse_y,
                        event.xbutton.button,
                        event.type == ButtonPress
                    );
                }
                break;
                
            case MotionNotify:
                window->mouse_x = (float)event.xmotion.x;
                window->mouse_y = (float)event.xmotion.y;
                if (window->callbacks.on_mouse) {
                    window->callbacks.on_mouse(
                        window->callbacks.userdata,
                        window->mouse_x,
                        window->mouse_y,
                        0, false
                    );
                }
                break;
        }
    }
    
    return window->should_close;
}