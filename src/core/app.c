#include "../baba.h"
#include "../theme/theme.h"
#include "platform/window.h"
#include "render/vulkan_renderer.h"
#include "widgets/widget.h"
#include "core/vec.h"
#include <stdlib.h>
#include <string.h>

struct BabaApp {
    BabaVec windows;
    BabaTheme* theme;
    bool running;
    int exit_code;
};

struct BabaWindow {
    BabaApp* app;
    BabaPlatformWindow* platform_window;
    BabaVulkanRenderer* renderer;
    BabaPainter* painter;
    BabaWidget* root_widget;
    char* title;
    int width, height;
    bool visible;
    bool should_close;
};

static int on_window_resize(void* userdata, int width, int height) {
    BabaWindow* window = (BabaWindow*)userdata;
    window->width = width;
    window->height = height;
    if (window->renderer) {
        baba_vulkan_renderer_resize(window->renderer, width, height);
    }
    if (window->root_widget) {
        BabaRect bounds = {0, 0, (float)width, (float)height};
        baba_widget_set_bounds(window->root_widget, bounds);
    }
    return BABA_SUCCESS;
}

static int on_window_close(void* userdata) {
    BabaWindow* window = (BabaWindow*)userdata;
    window->should_close = true;
    return BABA_SUCCESS;
}

BabaApp* baba_app_create(void) {
    BabaApp* app = calloc(1, sizeof(BabaApp));
    if (!app) return NULL;
    
    if (baba_vec_init(&app->windows, sizeof(BabaWindow*), 4) != BABA_SUCCESS) {
        free(app);
        return NULL;
    }
    
    app->running = false;
    app->exit_code = 0;
    app->theme = baba_theme_create_default();
    
    return app;
}

void baba_app_destroy(BabaApp* app) {
    if (!app) return;
    
    BabaWindow* window;
    BABA_VEC_FOREACH(app->windows, window) {
        baba_window_destroy(window);
    }
    baba_vec_destroy(&app->windows);
    
    if (app->theme) {
        baba_theme_destroy(app->theme);
    }
    
    free(app);
}

int baba_app_run(BabaApp* app) {
    if (!app) return BABA_ERROR_INVALID_PARAM;
    
    app->running = true;
    
    while (app->running) {
        baba_platform_poll_events();
        
        BabaWindow* window;
        BABA_VEC_FOREACH(app->windows, window) {
            if (baba_platform_window_should_close(window->platform_window)) {
                window->should_close = true;
            }
            
            if (window->should_close) {
                baba_window_close(window);
                continue;
            }
            
            if (window->visible && window->renderer) {
                if (baba_vulkan_renderer_begin_frame(window->renderer) == BABA_SUCCESS) {
                    baba_painter_begin(window->painter);
                    
                    if (window->root_widget) {
                        baba_widget_draw_recursive(window->root_widget, window->painter);
                    }
                    
                    baba_painter_end(window->painter);
                    baba_vulkan_renderer_end_frame(window->renderer);
                }
            }
        }
        
        bool any_visible = false;
        BABA_VEC_FOREACH(app->windows, window) {
            if (!window->should_close && window->visible) {
                any_visible = true;
                break;
            }
        }
        
        if (!any_visible) {
            app->running = false;
        }
    }
    
    return app->exit_code;
}

void baba_app_quit(BabaApp* app) {
    if (app) {
        app->running = false;
    }
}

void baba_app_set_exit_code(BabaApp* app, int code) {
    if (app) {
        app->exit_code = code;
    }
}

BabaWindow* baba_window_create(BabaApp* app, const char* title, int width, int height) {
    if (!app || !title) return NULL;
    
    BabaWindow* window = calloc(1, sizeof(BabaWindow));
    if (!window) return NULL;
    
    window->app = app;
    window->title = strdup(title);
    window->width = width;
    window->height = height;
    window->visible = false;
    window->should_close = false;
    
    BabaWindowCallbacks callbacks = {
        .on_resize = on_window_resize,
        .on_close = on_window_close,
        .userdata = window
    };
    
    window->platform_window = baba_platform_window_create(title, width, height, &callbacks);
    if (!window->platform_window) {
        free(window->title);
        free(window);
        return NULL;
    }
    
    BabaVulkanConfig vulkan_config = {
        .vsync = true,
        .max_frames_in_flight = 2
    };
    
    window->renderer = baba_vulkan_renderer_create(window->platform_window, &vulkan_config);
    if (!window->renderer) {
        baba_platform_window_destroy(window->platform_window);
        free(window->title);
        free(window);
        return NULL;
    }
    
    window->painter = baba_painter_create(window->renderer);
    if (!window->painter) {
        baba_vulkan_renderer_destroy(window->renderer);
        baba_platform_window_destroy(window->platform_window);
        free(window->title);
        free(window);
        return NULL;
    }
    
    window->root_widget = baba_widget_create(NULL, 0);
    if (!window->root_widget) {
        baba_painter_destroy(window->painter);
        baba_vulkan_renderer_destroy(window->renderer);
        baba_platform_window_destroy(window->platform_window);
        free(window->title);
        free(window);
        return NULL;
    }
    
    baba_vec_push(&app->windows, &window);
    
    return window;
}

void baba_window_destroy(BabaWindow* window) {
    if (!window) return;
    
    if (window->root_widget) {
        baba_widget_destroy(window->root_widget);
    }
    if (window->painter) {
        baba_painter_destroy(window->painter);
    }
    if (window->renderer) {
        baba_vulkan_renderer_destroy(window->renderer);
    }
    if (window->platform_window) {
        baba_platform_window_destroy(window->platform_window);
    }
    
    free(window->title);
    free(window);
}

void baba_window_set_title(BabaWindow* window, const char* title) {
    if (!window || !title) return;
    free(window->title);
    window->title = strdup(title);
    baba_platform_window_set_title(window->platform_window, title);
}

void baba_window_set_size(BabaWindow* window, int width, int height) {
    if (!window) return;
    window->width = width;
    window->height = height;
    baba_platform_window_set_size(window->platform_window, width, height);
}

void baba_window_get_size(BabaWindow* window, int* width, int* height) {
    if (!window) return;
    if (width) *width = window->width;
    if (height) *height = window->height;
}

void baba_window_set_position(BabaWindow* window, int x, int y) {
    if (!window) return;
    baba_platform_window_set_position(window->platform_window, x, y);
}

void baba_window_get_position(BabaWindow* window, int* x, int* y) {
    if (!window) return;
    baba_platform_window_get_position(window->platform_window, x, y);
}

void baba_window_show(BabaWindow* window) {
    if (!window) return;
    window->visible = true;
    baba_platform_window_show(window->platform_window);
}

void baba_window_hide(BabaWindow* window) {
    if (!window) return;
    window->visible = false;
    baba_platform_window_hide(window->platform_window);
}

void baba_window_close(BabaWindow* window) {
    if (!window) return;
    window->should_close = true;
    window->visible = false;
}

BabaWidget* baba_window_get_root(BabaWindow* window) {
    return window ? window->root_widget : NULL;
}

const char* baba_get_error_string(BabaResult result) {
    switch (result) {
        case BABA_SUCCESS: return "Success";
        case BABA_ERROR_UNKNOWN: return "Unknown error";
        case BABA_ERROR_INVALID_PARAM: return "Invalid parameter";
        case BABA_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case BABA_ERROR_VULKAN: return "Vulkan error";
        case BABA_ERROR_PLATFORM: return "Platform error";
        case BABA_ERROR_FONT: return "Font error";
        default: return "Unknown error";
    }
}