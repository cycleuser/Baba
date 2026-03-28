#include "baba.h"
#include "platform/platform.h"
#include <stdlib.h>
#include <string.h>

#ifdef BABA_PLATFORM_MACOS
#include <CoreFoundation/CoreFoundation.h>
#endif

struct BabaApp {
    int running;
};

struct BabaWindow {
    BabaApp* app;
    BabaPlatformWindow* platform;
    BabaEventHandler event_handler;
    void* event_userdata;
    int width;
    int height;
};

static bool default_event_handler(BabaWindow* window, const BabaEvent* event, void* userdata) {
    if (event->type == BABA_EVENT_CLOSE) {
        baba_window_close(window);
        return true;
    }
    return false;
}

BabaApp* baba_app_create(void) {
    BabaApp* app = calloc(1, sizeof(BabaApp));
    if (!app) return NULL;
    app->running = 1;
    return app;
}

void baba_app_destroy(BabaApp* app) {
    if (app) free(app);
}

int baba_app_run(BabaApp* app) {
    if (!app) return -1;
    
    while (app->running) {
        baba_platform_poll_events();
        
#ifdef BABA_PLATFORM_MACOS
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.01, false);
#elif defined(BABA_PLATFORM_LINUX)
        usleep(10000);
#elif defined(BABA_PLATFORM_WINDOWS)
        Sleep(10);
#endif
    }
    
    return 0;
}

void baba_app_quit(BabaApp* app) {
    if (app) app->running = 0;
}

static bool on_platform_event(void* userdata, int keycode, bool pressed) {
    BabaWindow* window = (BabaWindow*)userdata;
    if (!window || !window->event_handler) return false;
    
    BabaEvent event = {
        .type = BABA_EVENT_KEY,
        .key = { .keycode = keycode, .pressed = pressed }
    };
    return window->event_handler(window, &event, window->event_userdata);
}

static bool on_platform_mouse(void* userdata, float x, float y, int button, bool pressed) {
    BabaWindow* window = (BabaWindow*)userdata;
    if (!window || !window->event_handler) return false;
    
    BabaEvent event = {
        .type = BABA_EVENT_MOUSE,
        .mouse = { .x = x, .y = y, .button = button, .pressed = pressed }
    };
    return window->event_handler(window, &event, window->event_userdata);
}

static bool on_platform_resize(void* userdata, int width, int height) {
    BabaWindow* window = (BabaWindow*)userdata;
    if (!window) return false;
    
    window->width = width;
    window->height = height;
    
    if (!window->event_handler) return false;
    
    BabaEvent event = {
        .type = BABA_EVENT_RESIZE,
        .resize = { .width = width, .height = height }
    };
    return window->event_handler(window, &event, window->event_userdata);
}

static bool on_platform_close(void* userdata) {
    BabaWindow* window = (BabaWindow*)userdata;
    if (!window || !window->event_handler) return true;
    
    BabaEvent event = { .type = BABA_EVENT_CLOSE };
    return window->event_handler(window, &event, window->event_userdata);
}

BabaWindow* baba_window_create(BabaApp* app, const char* title, int width, int height) {
    if (!app) return NULL;
    
    BabaWindow* window = calloc(1, sizeof(BabaWindow));
    if (!window) return NULL;
    
    window->app = app;
    window->width = width;
    window->height = height;
    window->event_handler = default_event_handler;
    
    BabaPlatformCallbacks callbacks = {
        .on_key = on_platform_event,
        .on_mouse = on_platform_mouse,
        .on_resize = on_platform_resize,
        .on_close = on_platform_close,
        .userdata = window
    };
    
    window->platform = baba_platform_window_create(title, width, height, &callbacks);
    if (!window->platform) {
        free(window);
        return NULL;
    }
    
    return window;
}

void baba_window_destroy(BabaWindow* window) {
    if (!window) return;
    if (window->platform) {
        baba_platform_window_destroy(window->platform);
    }
    free(window);
}

void baba_window_set_title(BabaWindow* window, const char* title) {
    if (!window || !window->platform) return;
    baba_platform_window_set_title(window->platform, title);
}

void baba_window_set_size(BabaWindow* window, int width, int height) {
    if (!window || !window->platform) return;
    baba_platform_window_set_size(window->platform, width, height);
}

void baba_window_get_size(BabaWindow* window, int* width, int* height) {
    if (!window) return;
    if (width) *width = window->width;
    if (height) *height = window->height;
}

void baba_window_set_position(BabaWindow* window, int x, int y) {
    if (!window || !window->platform) return;
    baba_platform_window_set_position(window->platform, x, y);
}

void baba_window_get_position(BabaWindow* window, int* x, int* y) {
    if (!window || !window->platform) return;
    baba_platform_window_get_position(window->platform, x, y);
}

void baba_window_show(BabaWindow* window) {
    if (!window || !window->platform) return;
    baba_platform_window_show(window->platform);
}

void baba_window_hide(BabaWindow* window) {
    if (!window || !window->platform) return;
    baba_platform_window_hide(window->platform);
}

void baba_window_close(BabaWindow* window) {
    if (!window) return;
    baba_platform_window_close(window->platform);
    if (window->app) {
        window->app->running = 0;
    }
}

void baba_window_set_event_handler(BabaWindow* window, BabaEventHandler handler, void* userdata) {
    if (!window) return;
    window->event_handler = handler;
    window->event_userdata = userdata;
}

BabaCanvas* baba_window_get_canvas(BabaWindow* window) {
    return NULL;
}

void baba_canvas_clear(BabaCanvas* canvas, BabaColor color) {
}

void baba_canvas_draw_rect(BabaCanvas* canvas, BabaRect rect, BabaColor color) {
}

void baba_canvas_draw_text(BabaCanvas* canvas, float x, float y, const char* text, BabaColor color) {
}

void baba_canvas_present(BabaCanvas* canvas) {
}

const char* baba_get_version(void) {
    return "0.2.0";
}

struct BabaButton {
    BabaWindow* window;
    BabaPlatformButton* platform;
    BabaButtonClickCallback callback;
    void* userdata;
};

static void on_button_click(void* userdata) {
    BabaButton* button = (BabaButton*)userdata;
    if (button && button->callback) {
        button->callback(button, button->userdata);
    }
}

BabaButton* baba_button_create(BabaWindow* window, const char* text, float x, float y, float width, float height) {
    if (!window || !window->platform) return NULL;
    
    BabaButton* button = calloc(1, sizeof(BabaButton));
    if (!button) return NULL;
    
    button->window = window;
    button->platform = baba_platform_button_create(
        window->platform, text, x, y, width, height,
        on_button_click, button
    );
    
    if (!button->platform) {
        free(button);
        return NULL;
    }
    
    return button;
}

void baba_button_destroy(BabaButton* button) {
    if (!button) return;
    if (button->platform) {
        baba_platform_button_destroy(button->platform);
    }
    free(button);
}

void baba_button_set_text(BabaButton* button, const char* text) {
    if (!button || !button->platform) return;
    baba_platform_button_set_text(button->platform, text);
}

void baba_button_set_callback(BabaButton* button, BabaButtonClickCallback callback, void* userdata) {
    if (!button) return;
    button->callback = callback;
    button->userdata = userdata;
}

struct BabaTextField {
    BabaWindow* window;
    BabaPlatformTextField* platform;
};

BabaTextField* baba_textfield_create(BabaWindow* window, const char* text, float x, float y, float width, float height) {
    if (!window || !window->platform) return NULL;
    
    BabaTextField* field = calloc(1, sizeof(BabaTextField));
    if (!field) return NULL;
    
    field->window = window;
    field->platform = baba_platform_textfield_create(
        window->platform, text, x, y, width, height
    );
    
    if (!field->platform) {
        free(field);
        return NULL;
    }
    
    return field;
}

void baba_textfield_destroy(BabaTextField* field) {
    if (!field) return;
    if (field->platform) {
        baba_platform_textfield_destroy(field->platform);
    }
    free(field);
}

void baba_textfield_set_text(BabaTextField* field, const char* text) {
    if (!field || !field->platform) return;
    baba_platform_textfield_set_text(field->platform, text);
}

const char* baba_textfield_get_text(BabaTextField* field) {
    if (!field || !field->platform) return NULL;
    return baba_platform_textfield_get_text(field->platform);
}