#ifndef BABA_WIDGETS_BUTTON_H
#define BABA_WIDGETS_BUTTON_H

#include "widget.h"

typedef struct {
    const char* text;
    BabaColor background_color;
    BabaColor text_color;
    BabaColor hover_color;
    BabaColor press_color;
    float corner_radius;
    void (*on_click)(BabaWidget* button, void* user_data);
    void* user_data;
} BabaButtonStyle;

BabaWidget* baba_button_create(const char* text);
void baba_button_set_text(BabaWidget* button, const char* text);
const char* baba_button_get_text(BabaWidget* button);
void baba_button_set_on_click(BabaWidget* button, void (*callback)(BabaWidget*, void*), void* user_data);
void baba_button_set_style(BabaWidget* button, const BabaButtonStyle* style);

#endif