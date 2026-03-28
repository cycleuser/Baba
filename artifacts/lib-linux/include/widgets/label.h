#ifndef BABA_WIDGETS_LABEL_H
#define BABA_WIDGETS_LABEL_H

#include "widget.h"

typedef enum {
    BABA_TEXT_ALIGN_LEFT,
    BABA_TEXT_ALIGN_CENTER,
    BABA_TEXT_ALIGN_RIGHT,
} BabaTextAlign;

typedef enum {
    BABA_TEXT_WRAP_NONE,
    BABA_TEXT_WRAP_WORD,
    BABA_TEXT_WRAP_CHAR,
} BabaTextWrap;

BabaWidget* baba_label_create(const char* text);
void baba_label_set_text(BabaWidget* label, const char* text);
const char* baba_label_get_text(BabaWidget* label);
void baba_label_set_color(BabaWidget* label, BabaColor color);
void baba_label_set_font_size(BabaWidget* label, float size);
void baba_label_set_align(BabaWidget* label, BabaTextAlign align);
void baba_label_set_wrap(BabaWidget* label, BabaTextWrap wrap);

#endif