#ifndef BABA_PLATFORM_H
#define BABA_PLATFORM_H

#include "../baba.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct BabaPlatformWindow BabaPlatformWindow;
typedef struct BabaPlatformButton BabaPlatformButton;
typedef struct BabaPlatformTextField BabaPlatformTextField;

typedef void (*BabaButtonCallback)(void* userdata);

typedef struct {
    bool (*on_key)(void* userdata, int keycode, bool pressed);
    bool (*on_mouse)(void* userdata, float x, float y, int button, bool pressed);
    bool (*on_resize)(void* userdata, int width, int height);
    bool (*on_close)(void* userdata);
    void* userdata;
} BabaPlatformCallbacks;

BabaPlatformWindow* baba_platform_window_create(
    const char* title,
    int width,
    int height,
    BabaPlatformCallbacks* callbacks
);

void baba_platform_window_destroy(BabaPlatformWindow* window);

void baba_platform_window_set_title(BabaPlatformWindow* window, const char* title);
void baba_platform_window_get_size(BabaPlatformWindow* window, int* width, int* height);
void baba_platform_window_set_size(BabaPlatformWindow* window, int width, int height);
void baba_platform_window_get_position(BabaPlatformWindow* window, int* x, int* y);
void baba_platform_window_set_position(BabaPlatformWindow* window, int x, int y);
void baba_platform_window_show(BabaPlatformWindow* window);
void baba_platform_window_hide(BabaPlatformWindow* window);
void baba_platform_window_close(BabaPlatformWindow* window);

void* baba_platform_window_get_native(BabaPlatformWindow* window);
void baba_platform_window_invalidate(BabaPlatformWindow* window);

double baba_platform_get_time(void);
void baba_platform_poll_events(void);
bool baba_platform_window_should_close(BabaPlatformWindow* window);

BabaPlatformButton* baba_platform_button_create(
    BabaPlatformWindow* window,
    const char* text,
    float x, float y, float width, float height,
    BabaButtonCallback callback,
    void* userdata
);
void baba_platform_button_destroy(BabaPlatformButton* button);
void baba_platform_button_set_text(BabaPlatformButton* button, const char* text);

BabaPlatformTextField* baba_platform_textfield_create(
    BabaPlatformWindow* window,
    const char* text,
    float x, float y, float width, float height
);
void baba_platform_textfield_destroy(BabaPlatformTextField* field);
void baba_platform_textfield_set_text(BabaPlatformTextField* field, const char* text);
const char* baba_platform_textfield_get_text(BabaPlatformTextField* field);

#endif