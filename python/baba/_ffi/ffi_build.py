from cffi import FFI

ffi = FFI()

ffi.cdef("""
typedef struct BabaApp BabaApp;
typedef struct BabaWindow BabaWindow;
typedef struct BabaWidget BabaWidget;
typedef struct BabaContext BabaContext;

typedef enum {
    BABA_SUCCESS = 0,
    BABA_ERROR_UNKNOWN = -1,
    BABA_ERROR_INVALID_PARAM = -2,
    BABA_ERROR_OUT_OF_MEMORY = -3,
    BABA_ERROR_VULKAN = -4,
    BABA_ERROR_PLATFORM = -5,
    BABA_ERROR_FONT = -6,
} BabaResult;

typedef struct {
    float x, y;
} BabaVec2;

typedef struct {
    float x, y, width, height;
} BabaRect;

typedef struct {
    float r, g, b, a;
} BabaColor;

BabaApp* baba_app_create(void);
void baba_app_destroy(BabaApp* app);
int baba_app_run(BabaApp* app);
void baba_app_quit(BabaApp* app);

BabaWindow* baba_window_create(BabaApp* app, const char* title, int width, int height);
void baba_window_destroy(BabaWindow* window);
void baba_window_set_title(BabaWindow* window, const char* title);
void baba_window_set_size(BabaWindow* window, int width, int height);
void baba_window_get_size(BabaWindow* window, int* width, int* height);
void baba_window_show(BabaWindow* window);
void baba_window_hide(BabaWindow* window);
void baba_window_close(BabaWindow* window);
BabaWidget* baba_window_get_root(BabaWindow* window);

BabaWidget* baba_widget_create(void* vtable, size_t extra_size);
void baba_widget_destroy(BabaWidget* widget);
void baba_widget_add_child(BabaWidget* parent, BabaWidget* child);
void baba_widget_remove_child(BabaWidget* parent, BabaWidget* child);
BabaWidget* baba_widget_get_child(BabaWidget* widget, size_t index);
size_t baba_widget_get_child_count(BabaWidget* widget);
void baba_widget_set_bounds(BabaWidget* widget, BabaRect bounds);
BabaRect baba_widget_get_bounds(BabaWidget* widget);
void baba_widget_set_visible(BabaWidget* widget, bool visible);
bool baba_widget_is_visible(BabaWidget* widget);
void baba_widget_invalidate(BabaWidget* widget);

BabaWidget* baba_button_create(const char* text);
void baba_button_set_text(BabaWidget* button, const char* text);
const char* baba_button_get_text(BabaWidget* button);
void baba_button_set_on_click(BabaWidget* button, void* callback, void* user_data);

BabaWidget* baba_label_create(const char* text);
void baba_label_set_text(BabaWidget* label, const char* text);
const char* baba_label_get_text(BabaWidget* label);
void baba_label_set_color(BabaWidget* label, BabaColor color);
void baba_label_set_font_size(BabaWidget* label, float size);

BabaWidget* baba_textbox_create(const char* placeholder);
void baba_textbox_set_text(BabaWidget* textbox, const char* text);
const char* baba_textbox_get_text(BabaWidget* textbox);
void baba_textbox_set_password_mode(BabaWidget* textbox, bool password);

const char* baba_get_error_string(int result);

typedef void (*BabaButtonClickCallback)(BabaWidget* button, void* user_data);
""")

__all__ = ["ffi", "load_baba"]


def load_baba():
    import os
    import sys
    import ctypes.util

    lib_name = "baba"

    if sys.platform == "darwin":
        lib_name = "libbaba.dylib"
    elif sys.platform == "win32":
        lib_name = "baba.dll"
    else:
        lib_name = "libbaba.so"

    script_dir = os.path.dirname(os.path.abspath(__file__))
    lib_paths = [
        os.path.join(script_dir, "..", "..", "build", lib_name),
        os.path.join(script_dir, "lib", lib_name),
        os.path.join(script_dir, lib_name),
    ]

    if hasattr(sys, "prefix"):
        lib_paths.insert(0, os.path.join(sys.prefix, "lib", lib_name))

    for path in lib_paths:
        if os.path.exists(path):
            return ffi.dlopen(path)

    system_lib = ctypes.util.find_library("baba")
    if system_lib:
        return ffi.dlopen(system_lib)

    raise RuntimeError(
        f"Could not find Baba library. Searched paths:\n" + "\n".join(f"  - {p}" for p in lib_paths)
    )
