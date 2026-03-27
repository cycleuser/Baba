#include "theme.h"
#include <stdlib.h>
#include <string.h>

struct BabaTheme {
    BabaThemeData data;
};

static BabaThemeData default_light_theme = {
    .colors = {
        .primary = {0.2f, 0.4f, 0.8f, 1.0f},
        .secondary = {0.6f, 0.6f, 0.6f, 1.0f},
        .accent = {0.9f, 0.5f, 0.2f, 1.0f},
        .background = {0.95f, 0.95f, 0.95f, 1.0f},
        .surface = {1.0f, 1.0f, 1.0f, 1.0f},
        .error = {0.9f, 0.2f, 0.2f, 1.0f},
        .on_primary = {1.0f, 1.0f, 1.0f, 1.0f},
        .on_secondary = {0.0f, 0.0f, 0.0f, 1.0f},
        .on_accent = {1.0f, 1.0f, 1.0f, 1.0f},
        .on_background = {0.1f, 0.1f, 0.1f, 1.0f},
        .on_surface = {0.1f, 0.1f, 0.1f, 1.0f},
        .on_error = {1.0f, 1.0f, 1.0f, 1.0f},
    },
    .font = {
        .regular = NULL,
        .bold = NULL,
        .italic = NULL,
        .bold_italic = NULL,
        .size = 14.0f,
        .line_height = 1.5f,
    },
    .button = {
        .corner_radius = 4.0f,
        .border_width = 0.0f,
        .shadow_radius = 0.0f,
        .shadow_offset_x = 0.0f,
        .shadow_offset_y = 0.0f,
        .shadow_color = {0.0f, 0.0f, 0.0f, 0.0f},
        .transition_duration = 0.15f,
    },
    .input = {
        .corner_radius = 4.0f,
        .border_width = 1.0f,
        .shadow_radius = 0.0f,
        .shadow_offset_x = 0.0f,
        .shadow_offset_y = 0.0f,
        .shadow_color = {0.0f, 0.0f, 0.0f, 0.0f},
        .transition_duration = 0.15f,
    },
    .card = {
        .corner_radius = 8.0f,
        .border_width = 0.0f,
        .shadow_radius = 4.0f,
        .shadow_offset_x = 0.0f,
        .shadow_offset_y = 2.0f,
        .shadow_color = {0.0f, 0.0f, 0.0f, 0.1f},
        .transition_duration = 0.0f,
    },
    .dialog = {
        .corner_radius = 12.0f,
        .border_width = 0.0f,
        .shadow_radius = 8.0f,
        .shadow_offset_x = 0.0f,
        .shadow_offset_y = 4.0f,
        .shadow_color = {0.0f, 0.0f, 0.0f, 0.15f},
        .transition_duration = 0.0f,
    },
    .spacing_unit = 8.0f,
    .icon_size = 24.0f,
};

static BabaThemeData default_dark_theme = {
    .colors = {
        .primary = {0.4f, 0.6f, 0.9f, 1.0f},
        .secondary = {0.4f, 0.4f, 0.4f, 1.0f},
        .accent = {0.9f, 0.6f, 0.3f, 1.0f},
        .background = {0.12f, 0.12f, 0.12f, 1.0f},
        .surface = {0.18f, 0.18f, 0.18f, 1.0f},
        .error = {0.9f, 0.3f, 0.3f, 1.0f},
        .on_primary = {1.0f, 1.0f, 1.0f, 1.0f},
        .on_secondary = {1.0f, 1.0f, 1.0f, 1.0f},
        .on_accent = {1.0f, 1.0f, 1.0f, 1.0f},
        .on_background = {0.9f, 0.9f, 0.9f, 1.0f},
        .on_surface = {0.9f, 0.9f, 0.9f, 1.0f},
        .on_error = {1.0f, 1.0f, 1.0f, 1.0f},
    },
    .font = {
        .regular = NULL,
        .bold = NULL,
        .italic = NULL,
        .bold_italic = NULL,
        .size = 14.0f,
        .line_height = 1.5f,
    },
    .button = {
        .corner_radius = 4.0f,
        .border_width = 0.0f,
        .shadow_radius = 0.0f,
        .shadow_offset_x = 0.0f,
        .shadow_offset_y = 0.0f,
        .shadow_color = {0.0f, 0.0f, 0.0f, 0.0f},
        .transition_duration = 0.15f,
    },
    .input = {
        .corner_radius = 4.0f,
        .border_width = 1.0f,
        .shadow_radius = 0.0f,
        .shadow_offset_x = 0.0f,
        .shadow_offset_y = 0.0f,
        .shadow_color = {0.0f, 0.0f, 0.0f, 0.0f},
        .transition_duration = 0.15f,
    },
    .card = {
        .corner_radius = 8.0f,
        .border_width = 0.0f,
        .shadow_radius = 4.0f,
        .shadow_offset_x = 0.0f,
        .shadow_offset_y = 2.0f,
        .shadow_color = {0.0f, 0.0f, 0.0f, 0.2f},
        .transition_duration = 0.0f,
    },
    .dialog = {
        .corner_radius = 12.0f,
        .border_width = 0.0f,
        .shadow_radius = 8.0f,
        .shadow_offset_x = 0.0f,
        .shadow_offset_y = 4.0f,
        .shadow_color = {0.0f, 0.0f, 0.0f, 0.25f},
        .transition_duration = 0.0f,
    },
    .spacing_unit = 8.0f,
    .icon_size = 24.0f,
};

BabaTheme* baba_theme_create_default(void) {
    return baba_theme_create_light();
}

BabaTheme* baba_theme_create_light(void) {
    BabaTheme* theme = malloc(sizeof(BabaTheme));
    if (theme) {
        memcpy(&theme->data, &default_light_theme, sizeof(BabaThemeData));
    }
    return theme;
}

BabaTheme* baba_theme_create_dark(void) {
    BabaTheme* theme = malloc(sizeof(BabaTheme));
    if (theme) {
        memcpy(&theme->data, &default_dark_theme, sizeof(BabaThemeData));
    }
    return theme;
}

BabaTheme* baba_theme_create_from_data(const BabaThemeData* data) {
    if (!data) return NULL;
    BabaTheme* theme = malloc(sizeof(BabaTheme));
    if (theme) {
        memcpy(&theme->data, data, sizeof(BabaThemeData));
    }
    return theme;
}

void baba_theme_destroy(BabaTheme* theme) {
    free(theme);
}

const BabaThemeData* baba_theme_get_data(BabaTheme* theme) {
    return theme ? &theme->data : NULL;
}

void baba_theme_set_data(BabaTheme* theme, const BabaThemeData* data) {
    if (theme && data) {
        memcpy(&theme->data, data, sizeof(BabaThemeData));
    }
}

BabaColor baba_theme_color(BabaTheme* theme, const char* name) {
    if (!theme || !name) return (BabaColor){0, 0, 0, 1};
    
    if (strcmp(name, "primary") == 0) return theme->data.colors.primary;
    if (strcmp(name, "secondary") == 0) return theme->data.colors.secondary;
    if (strcmp(name, "accent") == 0) return theme->data.colors.accent;
    if (strcmp(name, "background") == 0) return theme->data.colors.background;
    if (strcmp(name, "surface") == 0) return theme->data.colors.surface;
    if (strcmp(name, "error") == 0) return theme->data.colors.error;
    if (strcmp(name, "on_primary") == 0) return theme->data.colors.on_primary;
    if (strcmp(name, "on_secondary") == 0) return theme->data.colors.on_secondary;
    if (strcmp(name, "on_accent") == 0) return theme->data.colors.on_accent;
    if (strcmp(name, "on_background") == 0) return theme->data.colors.on_background;
    if (strcmp(name, "on_surface") == 0) return theme->data.colors.on_surface;
    if (strcmp(name, "on_error") == 0) return theme->data.colors.on_error;
    
    return (BabaColor){0, 0, 0, 1};
}

void baba_theme_set_color(BabaTheme* theme, const char* name, BabaColor color) {
    if (!theme || !name) return;
    
    if (strcmp(name, "primary") == 0) theme->data.colors.primary = color;
    else if (strcmp(name, "secondary") == 0) theme->data.colors.secondary = color;
    else if (strcmp(name, "accent") == 0) theme->data.colors.accent = color;
    else if (strcmp(name, "background") == 0) theme->data.colors.background = color;
    else if (strcmp(name, "surface") == 0) theme->data.colors.surface = color;
    else if (strcmp(name, "error") == 0) theme->data.colors.error = color;
    else if (strcmp(name, "on_primary") == 0) theme->data.colors.on_primary = color;
    else if (strcmp(name, "on_secondary") == 0) theme->data.colors.on_secondary = color;
    else if (strcmp(name, "on_accent") == 0) theme->data.colors.on_accent = color;
    else if (strcmp(name, "on_background") == 0) theme->data.colors.on_background = color;
    else if (strcmp(name, "on_surface") == 0) theme->data.colors.on_surface = color;
    else if (strcmp(name, "on_error") == 0) theme->data.colors.on_error = color;
}