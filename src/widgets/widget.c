#include "widget.h"
#include <stdlib.h>
#include <string.h>

BabaWidget* baba_widget_create(const BabaWidgetVTable* vtable, size_t extra_size) {
    BabaWidget* widget = calloc(1, sizeof(BabaWidget) + extra_size);
    if (!widget) return NULL;
    
    widget->vtable = vtable;
    widget->flags = BABA_WIDGET_FLAG_VISIBLE | BABA_WIDGET_FLAG_ENABLED;
    baba_vec_init(&widget->children, sizeof(BabaWidget*), 4);
    
    if (vtable && vtable->init) {
        vtable->init(widget);
    }
    
    return widget;
}

void baba_widget_destroy(BabaWidget* widget) {
    if (!widget) return;
    
    BabaWidget* child;
    BABA_VEC_FOREACH(widget->children, child) {
        baba_widget_destroy(child);
    }
    baba_vec_destroy(&widget->children);
    
    if (widget->vtable && widget->vtable->destroy) {
        widget->vtable->destroy(widget);
    }
    
    free(widget);
}

void baba_widget_add_child(BabaWidget* parent, BabaWidget* child) {
    if (!parent || !child) return;
    child->parent = parent;
    baba_vec_push(&parent->children, &child);
}

void baba_widget_remove_child(BabaWidget* parent, BabaWidget* child) {
    if (!parent || !child) return;
    child->parent = NULL;
}

BabaWidget* baba_widget_get_child(BabaWidget* widget, size_t index) {
    if (!widget) return NULL;
    return *(BabaWidget**)baba_vec_get(&widget->children, index);
}

size_t baba_widget_get_child_count(BabaWidget* widget) {
    return widget ? widget->children.size : 0;
}

void baba_widget_set_bounds(BabaWidget* widget, BabaRect bounds) {
    if (!widget) return;
    widget->bounds = bounds;
    if (widget->vtable && widget->vtable->layout) {
        widget->vtable->layout(widget, bounds);
    }
}

void baba_widget_set_position(BabaWidget* widget, float x, float y) {
    if (!widget) return;
    widget->bounds.x = x;
    widget->bounds.y = y;
}

void baba_widget_set_size(BabaWidget* widget, float width, float height) {
    if (!widget) return;
    widget->bounds.width = width;
    widget->bounds.height = height;
}

BabaRect baba_widget_get_bounds(BabaWidget* widget) {
    return widget ? widget->bounds : (BabaRect){0, 0, 0, 0};
}

BabaRect baba_widget_get_content_bounds(BabaWidget* widget) {
    if (!widget) return (BabaRect){0, 0, 0, 0};
    BabaRect content = widget->bounds;
    content.x += widget->padding.x;
    content.y += widget->padding.y;
    content.width -= widget->padding.x + widget->padding.width;
    content.height -= widget->padding.y + widget->padding.height;
    return content;
}

void baba_widget_set_visible(BabaWidget* widget, bool visible) {
    if (!widget) return;
    if (visible) widget->flags |= BABA_WIDGET_FLAG_VISIBLE;
    else widget->flags &= ~BABA_WIDGET_FLAG_VISIBLE;
}

bool baba_widget_is_visible(BabaWidget* widget) {
    return widget ? (widget->flags & BABA_WIDGET_FLAG_VISIBLE) != 0 : false;
}

void baba_widget_set_enabled(BabaWidget* widget, bool enabled) {
    if (!widget) return;
    if (enabled) widget->flags |= BABA_WIDGET_FLAG_ENABLED;
    else widget->flags &= ~BABA_WIDGET_FLAG_ENABLED;
}

bool baba_widget_is_enabled(BabaWidget* widget) {
    return widget ? (widget->flags & BABA_WIDGET_FLAG_ENABLED) != 0 : false;
}

void baba_widget_set_padding(BabaWidget* widget, float left, float top, float right, float bottom) {
    if (!widget) return;
    widget->padding = (BabaRect){left, top, right, bottom};
}

void baba_widget_set_margin(BabaWidget* widget, float left, float top, float right, float bottom) {
    if (!widget) return;
    widget->margin = (BabaRect){left, top, right, bottom};
}

void baba_widget_invalidate(BabaWidget* widget) {
    if (!widget) return;
    widget->flags |= BABA_WIDGET_FLAG_DIRTY;
}

void baba_widget_invalidate_layout(BabaWidget* widget) {
    if (!widget) return;
}

BabaWidget* baba_widget_hit_test(BabaWidget* widget, float x, float y) {
    if (!widget || !baba_widget_is_visible(widget)) return NULL;
    
    BabaRect bounds = widget->bounds;
    if (x < bounds.x || x > bounds.x + bounds.width ||
        y < bounds.y || y > bounds.y + bounds.height) {
        return NULL;
    }
    
    BabaWidget* child;
    BABA_VEC_FOREACH(widget->children, child) {
        BabaWidget* hit = baba_widget_hit_test(child, x, y);
        if (hit) return hit;
    }
    
    return widget;
}

BabaWidget* baba_widget_find_by_id(BabaWidget* widget, const char* id) {
    if (!widget || !id) return NULL;
    if (widget->id && strcmp(widget->id, id) == 0) return widget;
    
    BabaWidget* child;
    BABA_VEC_FOREACH(widget->children, child) {
        BabaWidget* found = baba_widget_find_by_id(child, id);
        if (found) return found;
    }
    
    return NULL;
}

void baba_widget_draw_recursive(BabaWidget* widget, BabaPainter* painter) {
    if (!widget || !baba_widget_is_visible(widget)) return;
    
    if (widget->vtable && widget->vtable->draw) {
        widget->vtable->draw(widget, painter);
    }
    
    BabaWidget* child;
    BABA_VEC_FOREACH(widget->children, child) {
        baba_widget_draw_recursive(child, painter);
    }
}

bool baba_widget_dispatch_event(BabaWidget* widget, BabaEvent* event) {
    if (!widget || !baba_widget_is_visible(widget)) return false;
    
    if (widget->vtable && widget->vtable->event) {
        if (widget->vtable->event(widget, event)) return true;
    }
    
    if (widget->parent) {
        return baba_widget_dispatch_event(widget->parent, event);
    }
    
    return false;
}