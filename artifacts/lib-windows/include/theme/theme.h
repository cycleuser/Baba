#ifndef BABA_THEME_THEME_H
#define BABA_THEME_THEME_H

#include "../baba.h"

typedef struct BabaTheme BabaTheme;

typedef struct {
    BabaColor primary;
    BabaColor secondary;
    BabaColor accent;
    BabaColor background;
    BabaColor surface;
    BabaColor error;
    BabaColor on_primary;
    BabaColor on_secondary;
    BabaColor on_accent;
    BabaColor on_background;
    BabaColor on_surface;
    BabaColor on_error;
} BabaColorScheme;

typedef struct {
    const char* regular;
    const char* bold;
    const char* italic;
    const char* bold_italic;
    float size;
    float line_height;
} BabaFontFamily;

typedef struct {
    float corner_radius;
    float border_width;
    float shadow_radius;
    float shadow_offset_x;
    float shadow_offset_y;
    BabaColor shadow_color;
    float transition_duration;
} BabaShapeStyle;

typedef struct {
    BabaColorScheme colors;
    BabaFontFamily font;
    BabaShapeStyle button;
    BabaShapeStyle input;
    BabaShapeStyle card;
    BabaShapeStyle dialog;
    float spacing_unit;
    float icon_size;
} BabaThemeData;

BabaTheme* baba_theme_create_default(void);
BabaTheme* baba_theme_create_light(void);
BabaTheme* baba_theme_create_dark(void);
BabaTheme* baba_theme_create_from_data(const BabaThemeData* data);
void baba_theme_destroy(BabaTheme* theme);

const BabaThemeData* baba_theme_get_data(BabaTheme* theme);
void baba_theme_set_data(BabaTheme* theme, const BabaThemeData* data);

BabaColor baba_theme_color(BabaTheme* theme, const char* name);
void baba_theme_set_color(BabaTheme* theme, const char* name, BabaColor color);

void baba_app_set_theme(BabaApp* app, BabaTheme* theme);
BabaTheme* baba_app_get_theme(BabaApp* app);

#endif