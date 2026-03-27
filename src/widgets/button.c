#include "button.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* text;
    BabaColor background_color;
    BabaColor text_color;
    BabaColor hover_color;
    BabaColor press_color;
    float corner_radius;
    void (*on_click)(BabaWidget*, void*);
    void* on_click_data;
} ButtonData;

static void button_destroy(BabaWidget* widget) {
    ButtonData* data = (ButtonData*)widget->user_data;
    if (data) {
        free(data->text);
        free(data);
    }
}

static void button_draw(BabaWidget* widget, BabaPainter* painter) {
    ButtonData* data = (ButtonData*)widget->user_data;
    if (!data) return;
    
    BabaRect bounds = baba_widget_get_bounds(widget);
    
    BabaColor bg = data->background_color;
    if (widget->flags & BABA_WIDGET_FLAG_PRESSED) {
        bg = data->press_color;
    } else if (widget->flags & BABA_WIDGET_FLAG_HOVERED) {
        bg = data->hover_color;
    }
    
    baba_painter_set_fill_color(painter, bg);
    baba_painter_rounded_rect(painter, bounds, data->corner_radius);
    
    baba_painter_set_fill_color(painter, data->text_color);
    baba_painter_font_size(painter, 14.0f);
    baba_painter_text(painter, bounds.x + 10, bounds.y + bounds.height / 2, data->text);
}

static bool button_event(BabaWidget* widget, BabaEvent* event) {
    ButtonData* data = (ButtonData*)widget->user_data;
    if (!data) return false;
    
    switch (event->type) {
        case BABA_EVENT_MOUSE_BUTTON:
            if (event->mouse_button.button == BABA_MOUSE_BUTTON_LEFT) {
                if (event->mouse_button.pressed) {
                    widget->flags |= BABA_WIDGET_FLAG_PRESSED;
                } else {
                    widget->flags &= ~BABA_WIDGET_FLAG_PRESSED;
                    if (data->on_click) {
                        data->on_click(widget, data->on_click_data);
                    }
                }
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

static BabaWidgetVTable button_vtable = {
    .init = NULL,
    .destroy = button_destroy,
    .layout = NULL,
    .draw = button_draw,
    .event = button_event,
    .measure = NULL,
};

BabaWidget* baba_button_create(const char* text) {
    BabaWidget* widget = baba_widget_create(&button_vtable, sizeof(ButtonData));
    if (!widget) return NULL;
    
    ButtonData* data = calloc(1, sizeof(ButtonData));
    if (!data) {
        baba_widget_destroy(widget);
        return NULL;
    }
    
    data->text = text ? strdup(text) : strdup("");
    data->background_color = (BabaColor){0.2f, 0.4f, 0.8f, 1.0f};
    data->text_color = (BabaColor){1.0f, 1.0f, 1.0f, 1.0f};
    data->hover_color = (BabaColor){0.3f, 0.5f, 0.9f, 1.0f};
    data->press_color = (BabaColor){0.1f, 0.3f, 0.7f, 1.0f};
    data->corner_radius = 4.0f;
    
    widget->user_data = data;
    return widget;
}

void baba_button_set_text(BabaWidget* button, const char* text) {
    if (!button) return;
    ButtonData* data = (ButtonData*)button->user_data;
    if (data) {
        free(data->text);
        data->text = text ? strdup(text) : strdup("");
    }
}

const char* baba_button_get_text(BabaWidget* button) {
    if (!button) return NULL;
    ButtonData* data = (ButtonData*)button->user_data;
    return data ? data->text : NULL;
}

void baba_button_set_on_click(BabaWidget* button, void (*callback)(BabaWidget*, void*), void* user_data) {
    if (!button) return;
    ButtonData* data = (ButtonData*)button->user_data;
    if (data) {
        data->on_click = callback;
        data->on_click_data = user_data;
    }
}

void baba_button_set_style(BabaWidget* button, const BabaButtonStyle* style) {
    if (!button || !style) return;
    ButtonData* data = (ButtonData*)button->user_data;
    if (data) {
        if (style->text) {
            free(data->text);
            data->text = strdup(style->text);
        }
        data->background_color = style->background_color;
        data->text_color = style->text_color;
        data->hover_color = style->hover_color;
        data->press_color = style->press_color;
        data->corner_radius = style->corner_radius;
    }
}