#include "painter.h"
#include <stdlib.h>
#include <string.h>

struct BabaPainter {
    BabaVulkanRenderer* renderer;
    BabaColor fill_color;
    BabaColor stroke_color;
    float stroke_width;
    float corner_radius;
    float opacity;
    BabaFont* font;
    float font_size;
};

struct BabaFont {
    void* data;
    float size;
};

struct BabaImage {
    void* data;
    int width;
    int height;
    int channels;
};

BabaPainter* baba_painter_create(BabaVulkanRenderer* renderer) {
    BabaPainter* painter = calloc(1, sizeof(BabaPainter));
    if (!painter) return NULL;
    
    painter->renderer = renderer;
    painter->fill_color = (BabaColor){0.5f, 0.5f, 0.5f, 1.0f};
    painter->stroke_color = (BabaColor){0.0f, 0.0f, 0.0f, 1.0f};
    painter->stroke_width = 1.0f;
    painter->corner_radius = 0.0f;
    painter->opacity = 1.0f;
    painter->font_size = 14.0f;
    
    return painter;
}

void baba_painter_destroy(BabaPainter* painter) {
    free(painter);
}

void baba_painter_begin(BabaPainter* painter) {}

void baba_painter_end(BabaPainter* painter) {}

void baba_painter_set_fill_color(BabaPainter* painter, BabaColor color) {
    if (painter) painter->fill_color = color;
}

void baba_painter_set_stroke_color(BabaPainter* painter, BabaColor color) {
    if (painter) painter->stroke_color = color;
}

void baba_painter_set_stroke_width(BabaPainter* painter, float width) {
    if (painter) painter->stroke_width = width;
}

void baba_painter_set_corner_radius(BabaPainter* painter, float radius) {
    if (painter) painter->corner_radius = radius;
}

void baba_painter_set_opacity(BabaPainter* painter, float opacity) {
    if (painter) painter->opacity = opacity;
}

void baba_painter_rect(BabaPainter* painter, BabaRect rect) {}

void baba_painter_rounded_rect(BabaPainter* painter, BabaRect rect, float radius) {}

void baba_painter_circle(BabaPainter* painter, float x, float y, float radius) {}

void baba_painter_line(BabaPainter* painter, float x1, float y1, float x2, float y2) {}

void baba_painter_path_begin(BabaPainter* painter) {}

void baba_painter_path_move_to(BabaPainter* painter, float x, float y) {}

void baba_painter_path_line_to(BabaPainter* painter, float x, float y) {}

void baba_painter_path_curve_to(BabaPainter* painter, float cx1, float cy1, float cx2, float cy2, float x, float y) {}

void baba_painter_path_close(BabaPainter* painter) {}

void baba_painter_path_fill(BabaPainter* painter) {}

void baba_painter_path_stroke(BabaPainter* painter) {}

BabaFont* baba_font_load(const char* filepath, float size) {
    BabaFont* font = calloc(1, sizeof(BabaFont));
    if (font) font->size = size;
    return font;
}

BabaFont* baba_font_load_memory(const void* data, size_t size, float font_size) {
    BabaFont* font = calloc(1, sizeof(BabaFont));
    if (font) font->size = font_size;
    return font;
}

void baba_font_destroy(BabaFont* font) {
    free(font);
}

void baba_font_get_metrics(BabaFont* font, float* ascent, float* descent, float* line_gap) {
    if (ascent) *ascent = font ? font->size * 0.8f : 0;
    if (descent) *descent = font ? font->size * 0.2f : 0;
    if (line_gap) *line_gap = 0;
}

void baba_painter_font(BabaPainter* painter, BabaFont* font) {
    if (painter) painter->font = font;
}

void baba_painter_font_size(BabaPainter* painter, float size) {
    if (painter) painter->font_size = size;
}

void baba_painter_text(BabaPainter* painter, float x, float y, const char* text) {}

void baba_painter_text_ex(BabaPainter* painter, float x, float y, const char* text, size_t len) {}

BabaRect baba_painter_measure_text(BabaPainter* painter, const char* text) {
    BabaRect rect = {0, 0, 0, 0};
    if (painter && text) {
        rect.width = strlen(text) * painter->font_size * 0.6f;
        rect.height = painter->font_size;
    }
    return rect;
}

BabaImage* baba_image_load(const char* filepath) {
    return calloc(1, sizeof(BabaImage));
}

BabaImage* baba_image_load_memory(const void* data, size_t size) {
    return calloc(1, sizeof(BabaImage));
}

void baba_image_destroy(BabaImage* image) {
    free(image);
}

void baba_painter_image(BabaPainter* painter, BabaRect dest, BabaImage* image, BabaRect src) {}

void baba_painter_image_scaled(BabaPainter* painter, BabaRect dest, BabaImage* image) {}

void baba_painter_push_clip(BabaPainter* painter, BabaRect rect) {}

void baba_painter_pop_clip(BabaPainter* painter) {}

void baba_painter_push_transform(BabaPainter* painter, float matrix[6]) {}

void baba_painter_pop_transform(BabaPainter* painter) {}