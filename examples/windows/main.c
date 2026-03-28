/*
 * Baba GUI - Windows Example
 * 
 * A simple Windows application using Baba GUI framework.
 * 
 * Build with:
 *   gcc main.c -I"INCLUDE_PATH" -L"LIB_PATH" -lbaba_windows -lvulkan -luser32 -lgdi32 -o app.exe
 * 
 * Where:
 *   INCLUDE_PATH = python -c "import baba; print(baba.get_include_dir())"
 *   LIB_PATH = python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))"
 */

#include <baba.h>
#include <widgets/button.h>
#include <widgets/label.h>
#include <stdio.h>

static BabaWidget* g_label = NULL;
static int g_click_count = 0;

void on_button_click(BabaWidget* button, void* user_data) {
    g_click_count++;
    char text[64];
    snprintf(text, sizeof(text), "Clicked %d times!", g_click_count);
    baba_label_set_text(g_label, text);
    printf("Button clicked! Count: %d\n", g_click_count);
}

int main() {
    printf("Baba GUI - Windows Example\n");
    printf("==========================\n\n");
    
    BabaApp* app = baba_app_create();
    if (!app) {
        printf("Error: Failed to create application\n");
        return 1;
    }
    
    BabaWindow* win = baba_window_create(app, "Baba GUI Demo", 400, 300);
    if (!win) {
        printf("Error: Failed to create window\n");
        baba_app_destroy(app);
        return 1;
    }
    
    BabaWidget* root = baba_window_get_root(win);
    
    g_label = baba_label_create("Click the button below");
    baba_label_set_font_size(g_label, 18.0f);
    baba_widget_add_child(root, g_label);
    
    BabaWidget* button = baba_button_create("Click Me!");
    baba_button_set_on_click(button, on_button_click, NULL);
    baba_widget_add_child(root, button);
    
    printf("Window created. Running event loop...\n");
    
    baba_window_show(win);
    int result = baba_app_run(app);
    
    baba_window_destroy(win);
    baba_app_destroy(app);
    
    printf("Application exited with code %d\n", result);
    return result;
}