#include "label.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* text;
    BabaColor color;
    float font_size;
    BabaTextAlign align;
    BabaTextWrap wrap;
} LabelData;

static void label_destroy(BabaWidget* widget) {
    LabelData* data = (LabelData*)widget->user_data;
    if (data) {
        free(data->text);
        free(data);
    }
}

static void label_draw(BabaWidget* widget, BabaPainter* painter) {
    LabelData* data = (LabelData*)widget->user_data;
    if (!data) return;
    
    BabaRect bounds = baba_widget_get_bounds(widget);
    
    baba_painter_set_fill_color(painter, data->color);
    baba_painter_font_size(painter, data->font_size);
    baba_painter_text(painter, bounds.x, bounds.y + data->font_size, data->text);
}

static BabaWidgetVTable label_vtable = {
    .init = NULL,
    .destroy = label_destroy,
    .layout = NULL,
    .draw = label_draw,
    .event = NULL,
    .measure = NULL,
};

BabaWidget* baba_label_create(const char* text) {
    BabaWidget* widget = baba_widget_create(&label_vtable, sizeof(LabelData));
    if (!widget) return NULL;
    
    LabelData* data = calloc(1, sizeof(LabelData));
    if (!data) {
        baba_widget_destroy(widget);
        return NULL;
    }
    
    data->text = text ? strdup(text) : strdup("");
    data->color = (BabaColor){0.0f, 0.0f, 0.0f, 1.0f};
    data->font_size = 14.0f;
    data->align = BABA_TEXT_ALIGN_LEFT;
    data->wrap = BABA_TEXT_WRAP_NONE;
    
    widget->user_data = data;
    return widget;
}

void baba_label_set_text(BabaWidget* label, const char* text) {
    if (!label) return;
    LabelData* data = (LabelData*)label->user_data;
    if (data) {
        free(data->text);
        data->text = text ? strdup(text) : strdup("");
    }
}

const char* baba_label_get_text(BabaWidget* label) {
    if (!label) return NULL;
    LabelData* data = (LabelData*)label->user_data;
    return data ? data->text : NULL;
}

void baba_label_set_color(BabaWidget* label, BabaColor color) {
    if (!label) return;
    LabelData* data = (LabelData*)label->user_data;
    if (data) data->color = color;
}

void baba_label_set_font_size(BabaWidget* label, float size) {
    if (!label) return;
    LabelData* data = (LabelData*)label->user_data;
    if (data) data->font_size = size;
}

void baba_label_set_align(BabaWidget* label, BabaTextAlign align) {
    if (!label) return;
    LabelData* data = (LabelData*)label->user_data;
    if (data) data->align = align;
}

void baba_label_set_wrap(BabaWidget* label, BabaTextWrap wrap) {
    if (!label) return;
    LabelData* data = (LabelData*)label->user_data;
    if (data) data->wrap = wrap;
}