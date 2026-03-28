#define BABA_IMPLEMENTATION
#include "../baba.h"

static void on_btn(BabaButton* btn, void* data) {
    printf("Button clicked!\n");
}

int main(void) {
    printf("Starting demo...\n");
    
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "Baba Demo", 300, 200);
    
    BabaButton* btn = baba_button_create(win, "Hello", 100, 80, 100, 40);
    baba_button_set_callback(btn, on_btn, NULL);
    
    baba_window_show(win);
    printf("Window shown, running app...\n");
    
    baba_app_run(app);
    
    baba_button_destroy(btn);
    baba_window_destroy(win);
    baba_app_destroy(app);
    return 0;
}