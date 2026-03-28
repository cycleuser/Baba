#ifndef BABA_H
#define BABA_H

#include "core/types.h"

#define BABA_VERSION_MAJOR 0
#define BABA_VERSION_MINOR 1
#define BABA_VERSION_PATCH 0

typedef struct BabaApp BabaApp;
typedef struct BabaWindow BabaWindow;
typedef struct BabaWidget BabaWidget;
typedef struct BabaContext BabaContext;

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
    int (*on_create)(BabaWidget* widget, void* user_data);
    int (*on_destroy)(BabaWidget* widget, void* user_data);
    int (*on_draw)(BabaWidget* widget, BabaContext* ctx, void* user_data);
    int (*on_event)(BabaWidget* widget, void* event, void* user_data);
    void* user_data;
} BabaWidgetCallbacks;

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
BabaWidget* baba_window_get_root(BabaWindow* window);

const char* baba_get_error_string(BabaResult result);

#endif