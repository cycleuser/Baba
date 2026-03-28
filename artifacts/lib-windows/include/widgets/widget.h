#ifndef BABA_WIDGETS_WIDGET_H
#define BABA_WIDGETS_WIDGET_H

#include "../baba.h"
#include "../core/types.h"
#include "../render/painter.h"
#include "../core/vec.h"

typedef struct BabaWidget BabaWidget;

typedef enum {
    BABA_WIDGET_FLAG_VISIBLE = 1 << 0,
    BABA_WIDGET_FLAG_ENABLED = 1 << 1,
    BABA_WIDGET_FLAG_HOVERED = 1 << 2,
    BABA_WIDGET_FLAG_FOCUSED = 1 << 3,
    BABA_WIDGET_FLAG_PRESSED = 1 << 4,
    BABA_WIDGET_FLAG_DIRTY = 1 << 5,
    BABA_WIDGET_FLAG_CLIP_CHILDREN = 1 << 6,
} BabaWidgetFlags;

typedef struct {
    BabaVec children;
    BabaWidget* parent;
    BabaRect bounds;
    BabaRect padding;
    BabaRect margin;
    uint32_t flags;
    void* user_data;
    const char* id;
    
    int (*init)(BabaWidget* widget);
    void (*destroy)(BabaWidget* widget);
    void (*layout)(BabaWidget* widget, BabaRect bounds);
    void (*draw)(BabaWidget* widget, BabaPainter* painter);
    bool (*event)(BabaWidget* widget, BabaEvent* event);
    BabaVec2 (*measure)(BabaWidget* widget, BabaVec2 available);
} BabaWidgetVTable;

struct BabaWidget {
    const BabaWidgetVTable* vtable;
    BabaVec children;
    BabaWidget* parent;
    void* user_data;
    const char* id;
    BabaRect bounds;
    BabaRect padding;
    BabaRect margin;
    uint32_t flags;
};

BabaWidget* baba_widget_create(const BabaWidgetVTable* vtable, size_t extra_size);
void baba_widget_destroy(BabaWidget* widget);

void baba_widget_add_child(BabaWidget* parent, BabaWidget* child);
void baba_widget_remove_child(BabaWidget* parent, BabaWidget* child);
BabaWidget* baba_widget_get_child(BabaWidget* widget, size_t index);
size_t baba_widget_get_child_count(BabaWidget* widget);

void baba_widget_set_bounds(BabaWidget* widget, BabaRect bounds);
void baba_widget_set_position(BabaWidget* widget, float x, float y);
void baba_widget_set_size(BabaWidget* widget, float width, float height);
BabaRect baba_widget_get_bounds(BabaWidget* widget);
BabaRect baba_widget_get_content_bounds(BabaWidget* widget);

void baba_widget_set_visible(BabaWidget* widget, bool visible);
bool baba_widget_is_visible(BabaWidget* widget);
void baba_widget_set_enabled(BabaWidget* widget, bool enabled);
bool baba_widget_is_enabled(BabaWidget* widget);

void baba_widget_set_padding(BabaWidget* widget, float left, float top, float right, float bottom);
void baba_widget_set_margin(BabaWidget* widget, float left, float top, float right, float bottom);

void baba_widget_invalidate(BabaWidget* widget);
void baba_widget_invalidate_layout(BabaWidget* widget);

BabaWidget* baba_widget_hit_test(BabaWidget* widget, float x, float y);
BabaWidget* baba_widget_find_by_id(BabaWidget* widget, const char* id);

void baba_widget_draw_recursive(BabaWidget* widget, BabaPainter* painter);
bool baba_widget_dispatch_event(BabaWidget* widget, BabaEvent* event);

#endif