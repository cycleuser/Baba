#include "layout.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    BabaFlexLayout flex;
} FlexLayoutData;

typedef struct {
    BabaGridLayout grid;
} GridLayoutData;

void baba_layout_params(BabaWidget* widget, const BabaLayoutParams* params) {
    if (!widget || !params) return;
}

BabaLayoutParams* baba_layout_get_params(BabaWidget* widget) {
    return NULL;
}

static void flex_layout(BabaWidget* widget, BabaRect bounds) {
    FlexLayoutData* data = (FlexLayoutData*)widget->user_data;
    if (!data) return;
    
    size_t child_count = baba_widget_get_child_count(widget);
    if (child_count == 0) return;
    
    float gap = data->flex.gap;
    float total_gap = gap * (child_count - 1);
    float available = data->flex.direction == BABA_LAYOUT_DIRECTION_ROW 
        ? bounds.width - total_gap 
        : bounds.height - total_gap;
    float child_size = available / child_count;
    
    float offset = 0;
    for (size_t i = 0; i < child_count; i++) {
        BabaWidget* child = baba_widget_get_child(widget, i);
        if (!child) continue;
        
        BabaRect child_bounds = {
            .x = bounds.x + (data->flex.direction == BABA_LAYOUT_DIRECTION_ROW ? offset : 0),
            .y = bounds.y + (data->flex.direction == BABA_LAYOUT_DIRECTION_COLUMN ? offset : 0),
            .width = data->flex.direction == BABA_LAYOUT_DIRECTION_ROW ? child_size : bounds.width,
            .height = data->flex.direction == BABA_LAYOUT_DIRECTION_COLUMN ? child_size : bounds.height,
        };
        
        baba_widget_set_bounds(child, child_bounds);
        offset += child_size + gap;
    }
}

static void flex_destroy(BabaWidget* widget) {
    if (widget->user_data) free(widget->user_data);
}

static BabaWidgetVTable flex_vtable = {
    .init = NULL,
    .destroy = flex_destroy,
    .layout = flex_layout,
    .draw = NULL,
    .event = NULL,
    .measure = NULL,
};

BabaWidget* baba_flex_layout_create(BabaLayoutDirection direction) {
    BabaWidget* widget = baba_widget_create(&flex_vtable, sizeof(FlexLayoutData));
    if (!widget) return NULL;
    
    FlexLayoutData* data = calloc(1, sizeof(FlexLayoutData));
    if (!data) {
        baba_widget_destroy(widget);
        return NULL;
    }
    
    data->flex.direction = direction;
    data->flex.align_items = BABA_LAYOUT_ALIGN_START;
    data->flex.justify_content = BABA_LAYOUT_JUSTIFY_START;
    data->flex.wrap = false;
    data->flex.gap = 0;
    
    widget->user_data = data;
    return widget;
}

void baba_flex_layout_set_direction(BabaWidget* layout, BabaLayoutDirection direction) {
    if (!layout) return;
    FlexLayoutData* data = (FlexLayoutData*)layout->user_data;
    if (data) data->flex.direction = direction;
}

void baba_flex_layout_set_align(BabaWidget* layout, BabaLayoutAlign align) {
    if (!layout) return;
    FlexLayoutData* data = (FlexLayoutData*)layout->user_data;
    if (data) data->flex.align_items = align;
}

void baba_flex_layout_set_justify(BabaWidget* layout, BabaLayoutJustify justify) {
    if (!layout) return;
    FlexLayoutData* data = (FlexLayoutData*)layout->user_data;
    if (data) data->flex.justify_content = justify;
}

void baba_flex_layout_set_gap(BabaWidget* layout, float gap) {
    if (!layout) return;
    FlexLayoutData* data = (FlexLayoutData*)layout->user_data;
    if (data) data->flex.gap = gap;
}

void baba_flex_layout_set_wrap(BabaWidget* layout, bool wrap) {
    if (!layout) return;
    FlexLayoutData* data = (FlexLayoutData*)layout->user_data;
    if (data) data->flex.wrap = wrap;
}

static void grid_layout(BabaWidget* widget, BabaRect bounds) {
    GridLayoutData* data = (GridLayoutData*)widget->user_data;
    if (!data) return;
    
    int cols = data->grid.columns > 0 ? data->grid.columns : 1;
    int rows = data->grid.rows > 0 ? data->grid.rows : 1;
    
    float cell_width = bounds.width / cols;
    float cell_height = bounds.height / rows;
    
    size_t child_count = baba_widget_get_child_count(widget);
    for (size_t i = 0; i < child_count; i++) {
        BabaWidget* child = baba_widget_get_child(widget, i);
        if (!child) continue;
        
        int col = i % cols;
        int row = i / cols;
        
        BabaRect child_bounds = {
            .x = bounds.x + col * cell_width + data->grid.column_gap / 2,
            .y = bounds.y + row * cell_height + data->grid.row_gap / 2,
            .width = cell_width - data->grid.column_gap,
            .height = cell_height - data->grid.row_gap,
        };
        
        baba_widget_set_bounds(child, child_bounds);
    }
}

static void grid_destroy(BabaWidget* widget) {
    if (widget->user_data) {
        GridLayoutData* data = (GridLayoutData*)widget->user_data;
        free(data->grid.column_widths);
        free(data->grid.row_heights);
        free(data);
    }
}

static BabaWidgetVTable grid_vtable = {
    .init = NULL,
    .destroy = grid_destroy,
    .layout = grid_layout,
    .draw = NULL,
    .event = NULL,
    .measure = NULL,
};

BabaWidget* baba_grid_layout_create(int columns, int rows) {
    BabaWidget* widget = baba_widget_create(&grid_vtable, sizeof(GridLayoutData));
    if (!widget) return NULL;
    
    GridLayoutData* data = calloc(1, sizeof(GridLayoutData));
    if (!data) {
        baba_widget_destroy(widget);
        return NULL;
    }
    
    data->grid.columns = columns;
    data->grid.rows = rows;
    data->grid.column_gap = 0;
    data->grid.row_gap = 0;
    data->grid.auto_columns = false;
    data->grid.auto_rows = false;
    data->grid.column_widths = NULL;
    data->grid.row_heights = NULL;
    
    widget->user_data = data;
    return widget;
}

void baba_grid_layout_set_column_gap(BabaWidget* layout, float gap) {
    if (!layout) return;
    GridLayoutData* data = (GridLayoutData*)layout->user_data;
    if (data) data->grid.column_gap = gap;
}

void baba_grid_layout_set_row_gap(BabaWidget* layout, float gap) {
    if (!layout) return;
    GridLayoutData* data = (GridLayoutData*)layout->user_data;
    if (data) data->grid.row_gap = gap;
}

void baba_grid_layout_set_column_width(BabaWidget* layout, int column, float width) {
    if (!layout) return;
    GridLayoutData* data = (GridLayoutData*)layout->user_data;
    if (data && column >= 0 && column < data->grid.columns) {
        if (!data->grid.column_widths) {
            data->grid.column_widths = calloc(data->grid.columns, sizeof(float));
        }
        if (data->grid.column_widths) {
            data->grid.column_widths[column] = width;
        }
    }
}

void baba_grid_layout_set_row_height(BabaWidget* layout, int row, float height) {
    if (!layout) return;
    GridLayoutData* data = (GridLayoutData*)layout->user_data;
    if (data && row >= 0 && row < data->grid.rows) {
        if (!data->grid.row_heights) {
            data->grid.row_heights = calloc(data->grid.rows, sizeof(float));
        }
        if (data->grid.row_heights) {
            data->grid.row_heights[row] = height;
        }
    }
}

void baba_layout_perform(BabaWidget* root, BabaRect bounds) {
    if (!root) return;
    baba_widget_set_bounds(root, bounds);
}

BabaVec2 baba_layout_measure(BabaWidget* widget, BabaVec2 available) {
    if (!widget) return (BabaVec2){0, 0};
    
    float max_width = 0;
    float max_height = 0;
    
    size_t child_count = baba_widget_get_child_count(widget);
    for (size_t i = 0; i < child_count; i++) {
        BabaWidget* child = baba_widget_get_child(widget, i);
        if (!child) continue;
        
        BabaRect bounds = baba_widget_get_bounds(child);
        max_width = BABA_MAX(max_width, bounds.width);
        max_height = BABA_MAX(max_height, bounds.height);
    }
    
    return (BabaVec2){max_width, max_height};
}