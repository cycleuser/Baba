#include "textbox.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* text;
    char* placeholder;
    size_t cursor_pos;
    size_t selection_start;
    size_t selection_end;
    bool password_mode;
    bool readonly;
    size_t max_length;
    BabaColor background_color;
    BabaColor text_color;
    BabaColor border_color;
    float font_size;
} TextBoxData;

static void textbox_destroy(BabaWidget* widget) {
    TextBoxData* data = (TextBoxData*)widget->user_data;
    if (data) {
        free(data->text);
        free(data->placeholder);
        free(data);
    }
}

static void textbox_draw(BabaWidget* widget, BabaPainter* painter) {
    TextBoxData* data = (TextBoxData*)widget->user_data;
    if (!data) return;
    
    BabaRect bounds = baba_widget_get_bounds(widget);
    
    baba_painter_set_fill_color(painter, data->background_color);
    baba_painter_rect(painter, bounds);
    
    const char* display_text = data->text && data->text[0] ? data->text : data->placeholder;
    BabaColor text_color = data->text && data->text[0] ? data->text_color : (BabaColor){0.5f, 0.5f, 0.5f, 1.0f};
    
    if (data->password_mode && data->text) {
        size_t len = strlen(data->text);
        char* masked = malloc(len + 1);
        memset(masked, '*', len);
        masked[len] = '\0';
        baba_painter_set_fill_color(painter, text_color);
        baba_painter_font_size(painter, data->font_size);
        baba_painter_text(painter, bounds.x + 8, bounds.y + bounds.height / 2, masked);
        free(masked);
    } else {
        baba_painter_set_fill_color(painter, text_color);
        baba_painter_font_size(painter, data->font_size);
        baba_painter_text(painter, bounds.x + 8, bounds.y + bounds.height / 2, display_text);
    }
}

static bool textbox_event(BabaWidget* widget, BabaEvent* event) {
    TextBoxData* data = (TextBoxData*)widget->user_data;
    if (!data || data->readonly) return false;
    
    switch (event->type) {
        case BABA_EVENT_CHAR:
            if (event->character.codepoint >= 32 && event->character.codepoint < 127) {
                size_t len = data->text ? strlen(data->text) : 0;
                if (data->max_length == 0 || len < data->max_length) {
                    char* new_text = malloc(len + 2);
                    if (data->text) {
                        strcpy(new_text, data->text);
                        free(data->text);
                    } else {
                        new_text[0] = '\0';
                    }
                    new_text[len] = (char)event->character.codepoint;
                    new_text[len + 1] = '\0';
                    data->text = new_text;
                }
                return true;
            }
            break;
        case BABA_EVENT_KEY:
            if (event->key.state == BABA_KEY_STATE_PRESSED) {
                if (event->key.keycode == 8) {
                    size_t len = data->text ? strlen(data->text) : 0;
                    if (len > 0) {
                        data->text[len - 1] = '\0';
                    }
                    return true;
                }
            }
            break;
        default:
            break;
    }
    return false;
}

static BabaWidgetVTable textbox_vtable = {
    .init = NULL,
    .destroy = textbox_destroy,
    .layout = NULL,
    .draw = textbox_draw,
    .event = textbox_event,
    .measure = NULL,
};

BabaWidget* baba_textbox_create(const char* placeholder) {
    BabaWidget* widget = baba_widget_create(&textbox_vtable, sizeof(TextBoxData));
    if (!widget) return NULL;
    
    TextBoxData* data = calloc(1, sizeof(TextBoxData));
    if (!data) {
        baba_widget_destroy(widget);
        return NULL;
    }
    
    data->text = strdup("");
    data->placeholder = placeholder ? strdup(placeholder) : strdup("");
    data->cursor_pos = 0;
    data->max_length = 0;
    data->background_color = (BabaColor){1.0f, 1.0f, 1.0f, 1.0f};
    data->text_color = (BabaColor){0.0f, 0.0f, 0.0f, 1.0f};
    data->border_color = (BabaColor){0.7f, 0.7f, 0.7f, 1.0f};
    data->font_size = 14.0f;
    
    widget->user_data = data;
    return widget;
}

void baba_textbox_set_text(BabaWidget* textbox, const char* text) {
    if (!textbox) return;
    TextBoxData* data = (TextBoxData*)textbox->user_data;
    if (data) {
        free(data->text);
        data->text = text ? strdup(text) : strdup("");
    }
}

const char* baba_textbox_get_text(BabaWidget* textbox) {
    if (!textbox) return NULL;
    TextBoxData* data = (TextBoxData*)textbox->user_data;
    return data ? data->text : NULL;
}

void baba_textbox_set_placeholder(BabaWidget* textbox, const char* placeholder) {
    if (!textbox) return;
    TextBoxData* data = (TextBoxData*)textbox->user_data;
    if (data) {
        free(data->placeholder);
        data->placeholder = placeholder ? strdup(placeholder) : strdup("");
    }
}

void baba_textbox_set_password_mode(BabaWidget* textbox, bool password) {
    if (!textbox) return;
    TextBoxData* data = (TextBoxData*)textbox->user_data;
    if (data) data->password_mode = password;
}

void baba_textbox_set_readonly(BabaWidget* textbox, bool readonly) {
    if (!textbox) return;
    TextBoxData* data = (TextBoxData*)textbox->user_data;
    if (data) data->readonly = readonly;
}

void baba_textbox_set_max_length(BabaWidget* textbox, size_t max_length) {
    if (!textbox) return;
    TextBoxData* data = (TextBoxData*)textbox->user_data;
    if (data) data->max_length = max_length;
}

void baba_textbox_set_style(BabaWidget* textbox, const BabaTextBoxStyle* style) {
    if (!textbox || !style) return;
    TextBoxData* data = (TextBoxData*)textbox->user_data;
    if (data) {
        data->background_color = style->background_color;
        data->text_color = style->text_color;
        data->border_color = style->border_color;
    }
}

void baba_textbox_select_all(BabaWidget* textbox) {
    if (!textbox) return;
    TextBoxData* data = (TextBoxData*)textbox->user_data;
    if (data && data->text) {
        data->selection_start = 0;
        data->selection_end = strlen(data->text);
    }
}

void baba_textbox_clear_selection(BabaWidget* textbox) {
    if (!textbox) return;
    TextBoxData* data = (TextBoxData*)textbox->user_data;
    if (data) {
        data->selection_start = 0;
        data->selection_end = 0;
    }
}