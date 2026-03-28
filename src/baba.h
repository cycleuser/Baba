#ifndef BABA_H
#define BABA_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BABA_VERSION_MAJOR 0
#define BABA_VERSION_MINOR 2
#define BABA_VERSION_PATCH 0

typedef struct BabaApp BabaApp;
typedef struct BabaWindow BabaWindow;
typedef struct BabaCanvas BabaCanvas;
typedef struct BabaButton BabaButton;
typedef struct BabaTextField BabaTextField;

typedef void (*BabaButtonClickCallback)(BabaButton* button, void* userdata);

typedef struct {
    float x, y;
} BabaVec2;

typedef struct {
    float x, y, width, height;
} BabaRect;

typedef struct {
    float r, g, b, a;
} BabaColor;

typedef struct {
    int keycode;
    bool pressed;
} BabaKeyEvent;

typedef struct {
    float x, y;
    int button;
    bool pressed;
} BabaMouseEvent;

typedef struct {
    int width, height;
} BabaResizeEvent;

typedef enum {
    BABA_EVENT_KEY,
    BABA_EVENT_MOUSE,
    BABA_EVENT_RESIZE,
    BABA_EVENT_CLOSE,
} BabaEventType;

typedef struct {
    BabaEventType type;
    union {
        BabaKeyEvent key;
        BabaMouseEvent mouse;
        BabaResizeEvent resize;
    };
} BabaEvent;

typedef bool (*BabaEventHandler)(BabaWindow* window, const BabaEvent* event, void* userdata);

BabaApp* baba_app_create(void);
void baba_app_destroy(BabaApp* app);
int baba_app_run(BabaApp* app);
void baba_app_quit(BabaApp* app);

BabaWindow* baba_window_create(BabaApp* app, const char* title, int width, int height);
void baba_window_destroy(BabaWindow* window);
void baba_window_set_title(BabaWindow* window, const char* title);
void baba_window_set_size(BabaWindow* window, int width, int height);
void baba_window_get_size(BabaWindow* window, int* width, int* height);
void baba_window_set_position(BabaWindow* window, int x, int y);
void baba_window_get_position(BabaWindow* window, int* x, int* y);
void baba_window_show(BabaWindow* window);
void baba_window_hide(BabaWindow* window);
void baba_window_close(BabaWindow* window);
void baba_window_set_event_handler(BabaWindow* window, BabaEventHandler handler, void* userdata);

BabaCanvas* baba_window_get_canvas(BabaWindow* window);

BabaButton* baba_button_create(BabaWindow* window, const char* text, float x, float y, float width, float height);
void baba_button_destroy(BabaButton* button);
void baba_button_set_text(BabaButton* button, const char* text);
void baba_button_set_callback(BabaButton* button, BabaButtonClickCallback callback, void* userdata);

BabaTextField* baba_textfield_create(BabaWindow* window, const char* text, float x, float y, float width, float height);
void baba_textfield_destroy(BabaTextField* field);
void baba_textfield_set_text(BabaTextField* field, const char* text);
const char* baba_textfield_get_text(BabaTextField* field);

void baba_canvas_clear(BabaCanvas* canvas, BabaColor color);
void baba_canvas_draw_rect(BabaCanvas* canvas, BabaRect rect, BabaColor color);
void baba_canvas_draw_text(BabaCanvas* canvas, float x, float y, const char* text, BabaColor color);
void baba_canvas_present(BabaCanvas* canvas);

const char* baba_get_version(void);

#ifdef __cplusplus
}
#endif

#endif