#include <baba.h>
#include <widgets/button.h>
#include <widgets/label.h>
#include <stdio.h>

void on_button_click(BabaWidget* button, void* user_data) {
    printf("Button clicked!\n");
}

int main(int argc, char** argv) {
    BabaApp* app = baba_app_create();
    if (!app) {
        fprintf(stderr, "Failed to create app\n");
        return 1;
    }
    
    BabaWindow* window = baba_window_create(app, "Baba Demo", 800, 600);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        baba_app_destroy(app);
        return 1;
    }
    
    BabaWidget* root = baba_window_get_root(window);
    BabaWidget* button = baba_button_create("Click Me");
    baba_button_set_on_click(button, on_button_click, NULL);
    baba_widget_add_child(root, button);
    
    BabaWidget* label = baba_label_create("Hello, Baba GUI!");
    baba_label_set_font_size(label, 24.0f);
    baba_widget_add_child(root, label);
    
    baba_window_show(window);
    
    int result = baba_app_run(app);
    
    baba_window_destroy(window);
    baba_app_destroy(app);
    
    return result;
}