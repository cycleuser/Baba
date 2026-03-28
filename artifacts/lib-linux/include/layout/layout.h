#ifndef BABA_LAYOUT_LAYOUT_H
#define BABA_LAYOUT_LAYOUT_H

#include "../baba.h"
#include "../widgets/widget.h"

typedef enum {
    BABA_LAYOUT_DIRECTION_ROW,
    BABA_LAYOUT_DIRECTION_COLUMN,
} BabaLayoutDirection;

typedef enum {
    BABA_LAYOUT_ALIGN_START,
    BABA_LAYOUT_ALIGN_CENTER,
    BABA_LAYOUT_ALIGN_END,
    BABA_LAYOUT_ALIGN_STRETCH,
} BabaLayoutAlign;

typedef enum {
    BABA_LAYOUT_JUSTIFY_START,
    BABA_LAYOUT_JUSTIFY_CENTER,
    BABA_LAYOUT_JUSTIFY_END,
    BABA_LAYOUT_JUSTIFY_SPACE_BETWEEN,
    BABA_LAYOUT_JUSTIFY_SPACE_AROUND,
    BABA_LAYOUT_JUSTIFY_SPACE_EVENLY,
} BabaLayoutJustify;

typedef struct {
    BabaLayoutDirection direction;
    BabaLayoutAlign align_items;
    BabaLayoutJustify justify_content;
    bool wrap;
    float gap;
    float row_gap;
    float column_gap;
} BabaFlexLayout;

typedef struct {
    int columns;
    int rows;
    float column_gap;
    float row_gap;
    bool auto_columns;
    bool auto_rows;
    float* column_widths;
    float* row_heights;
} BabaGridLayout;

typedef struct {
    float left, top, right, bottom;
} BabaEdgeInsets;

typedef struct {
    float grow;
    float shrink;
    BabaVec2 min_size;
    BabaVec2 max_size;
    BabaVec2 preferred_size;
    BabaEdgeInsets margin;
    BabaEdgeInsets padding;
} BabaLayoutParams;

void baba_layout_params(BabaWidget* widget, const BabaLayoutParams* params);
BabaLayoutParams* baba_layout_get_params(BabaWidget* widget);

BabaWidget* baba_flex_layout_create(BabaLayoutDirection direction);
void baba_flex_layout_set_direction(BabaWidget* layout, BabaLayoutDirection direction);
void baba_flex_layout_set_align(BabaWidget* layout, BabaLayoutAlign align);
void baba_flex_layout_set_justify(BabaWidget* layout, BabaLayoutJustify justify);
void baba_flex_layout_set_gap(BabaWidget* layout, float gap);
void baba_flex_layout_set_wrap(BabaWidget* layout, bool wrap);

BabaWidget* baba_grid_layout_create(int columns, int rows);
void baba_grid_layout_set_column_gap(BabaWidget* layout, float gap);
void baba_grid_layout_set_row_gap(BabaWidget* layout, float gap);
void baba_grid_layout_set_column_width(BabaWidget* layout, int column, float width);
void baba_grid_layout_set_row_height(BabaWidget* layout, int row, float height);

void baba_layout_perform(BabaWidget* root, BabaRect bounds);
BabaVec2 baba_layout_measure(BabaWidget* widget, BabaVec2 available);

#endif