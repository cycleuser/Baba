#ifndef BABA_RENDER_PAINTER_H
#define BABA_RENDER_PAINTER_H

#include "../baba.h"
#include "vulkan_renderer.h"

typedef struct BabaPainter BabaPainter;
typedef struct BabaFont BabaFont;
typedef struct BabaImage BabaImage;

typedef struct {
    BabaColor fill_color;
    BabaColor stroke_color;
    float stroke_width;
    float corner_radius;
    float opacity;
} BabaPaintStyle;

BabaPainter* baba_painter_create(BabaVulkanRenderer* renderer);
void baba_painter_destroy(BabaPainter* painter);

void baba_painter_begin(BabaPainter* painter);
void baba_painter_end(BabaPainter* painter);

void baba_painter_set_fill_color(BabaPainter* painter, BabaColor color);
void baba_painter_set_stroke_color(BabaPainter* painter, BabaColor color);
void baba_painter_set_stroke_width(BabaPainter* painter, float width);
void baba_painter_set_corner_radius(BabaPainter* painter, float radius);
void baba_painter_set_opacity(BabaPainter* painter, float opacity);

void baba_painter_rect(BabaPainter* painter, BabaRect rect);
void baba_painter_rounded_rect(BabaPainter* painter, BabaRect rect, float radius);
void baba_painter_circle(BabaPainter* painter, float x, float y, float radius);
void baba_painter_line(BabaPainter* painter, float x1, float y1, float x2, float y2);
void baba_painter_path_begin(BabaPainter* painter);
void baba_painter_path_move_to(BabaPainter* painter, float x, float y);
void baba_painter_path_line_to(BabaPainter* painter, float x, float y);
void baba_painter_path_curve_to(BabaPainter* painter, float cx1, float cy1, float cx2, float cy2, float x, float y);
void baba_painter_path_close(BabaPainter* painter);
void baba_painter_path_fill(BabaPainter* painter);
void baba_painter_path_stroke(BabaPainter* painter);

BabaFont* baba_font_load(const char* filepath, float size);
BabaFont* baba_font_load_memory(const void* data, size_t size, float font_size);
void baba_font_destroy(BabaFont* font);
void baba_font_get_metrics(BabaFont* font, float* ascent, float* descent, float* line_gap);

void baba_painter_font(BabaPainter* painter, BabaFont* font);
void baba_painter_font_size(BabaPainter* painter, float size);
void baba_painter_text(BabaPainter* painter, float x, float y, const char* text);
void baba_painter_text_ex(BabaPainter* painter, float x, float y, const char* text, size_t len);
BabaRect baba_painter_measure_text(BabaPainter* painter, const char* text);

BabaImage* baba_image_load(const char* filepath);
BabaImage* baba_image_load_memory(const void* data, size_t size);
void baba_image_destroy(BabaImage* image);
void baba_painter_image(BabaPainter* painter, BabaRect dest, BabaImage* image, BabaRect src);
void baba_painter_image_scaled(BabaPainter* painter, BabaRect dest, BabaImage* image);

void baba_painter_push_clip(BabaPainter* painter, BabaRect rect);
void baba_painter_pop_clip(BabaPainter* painter);

void baba_painter_push_transform(BabaPainter* painter, float matrix[6]);
void baba_painter_pop_transform(BabaPainter* painter);

#endif