#include "../baba.h"
#include <stdlib.h>

struct BabaWindow {
    void* platform_window;
    void* renderer;
    void* root_widget;
};

BabaWindow* baba_window_create(BabaApp* app, const char* title, int width, int height) {
    return NULL;
}

void baba_window_destroy(BabaWindow* window) {}

void baba_window_set_title(BabaWindow* window, const char* title) {}

void baba_window_set_size(BabaWindow* window, int width, int height) {}

void baba_window_get_size(BabaWindow* window, int* width, int* height) {
    if (width) *width = 0;
    if (height) *height = 0;
}

void baba_window_set_position(BabaWindow* window, int x, int y) {}

void baba_window_get_position(BabaWindow* window, int* x, int* y) {
    if (x) *x = 0;
    if (y) *y = 0;
}

void baba_window_show(BabaWindow* window) {}

void baba_window_hide(BabaWindow* window) {}

void baba_window_close(BabaWindow* window) {}

BabaWidget* baba_window_get_root(BabaWindow* window) {
    return NULL;
}