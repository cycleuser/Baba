#ifndef BABA_WIDGETS_TEXTBOX_H
#define BABA_WIDGETS_TEXTBOX_H

#include "widget.h"

typedef struct {
    BabaColor background_color;
    BabaColor text_color;
    BabaColor selection_color;
    BabaColor cursor_color;
    BabaColor border_color;
    BabaColor focus_border_color;
    float border_width;
    float corner_radius;
    float padding;
} BabaTextBoxStyle;

BabaWidget* baba_textbox_create(const char* placeholder);
void baba_textbox_set_text(BabaWidget* textbox, const char* text);
const char* baba_textbox_get_text(BabaWidget* textbox);
void baba_textbox_set_placeholder(BabaWidget* textbox, const char* placeholder);
void baba_textbox_set_password_mode(BabaWidget* textbox, bool password);
void baba_textbox_set_readonly(BabaWidget* textbox, bool readonly);
void baba_textbox_set_max_length(BabaWidget* textbox, size_t max_length);
void baba_textbox_set_style(BabaWidget* textbox, const BabaTextBoxStyle* style);
void baba_textbox_select_all(BabaWidget* textbox);
void baba_textbox_clear_selection(BabaWidget* textbox);

#endif