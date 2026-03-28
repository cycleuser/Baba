#define BABA_IMPLEMENTATION
#include "../baba.h"

static BabaImageView* image = NULL;
static BabaLabel* info = NULL;
static char current_file[512] = "";

static void on_open(BabaButton* btn, void* data) {
    char* path = baba_file_open_dialog(NULL, "Open Image", "png,jpg,jpeg,gif,bmp,tiff");
    if (path) {
        if (baba_image_load_file(image, path)) {
            strcpy(current_file, path);
            int w, h;
            baba_image_get_size(image, &w, &h);
            char msg[256];
            snprintf(msg, sizeof(msg), "%s - %dx%d", path, w, h);
            baba_label_set_text(info, msg);
        } else {
            baba_label_set_text(info, "Failed to load image");
        }
        free(path);
    }
}

static void on_save(BabaButton* btn, void* data) {
    if (strlen(current_file) == 0) {
        baba_label_set_text(info, "No image loaded");
        return;
    }
    char* path = baba_file_save_dialog(NULL, "Save Image", "png");
    if (path) {
        if (baba_image_save_file(image, path)) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Saved: %s", path);
            baba_label_set_text(info, msg);
        } else {
            baba_label_set_text(info, "Failed to save image");
        }
        free(path);
    }
}

static void on_info(BabaButton* btn, void* data) {
    if (strlen(current_file) == 0) {
        baba_label_set_text(info, "No image loaded");
        return;
    }
    int w, h;
    baba_image_get_size(image, &w, &h);
    char msg[256];
    snprintf(msg, sizeof(msg), "File: %s | Size: %dx%d pixels", current_file, w, h);
    baba_label_set_text(info, msg);
}

static void on_clear(BabaButton* btn, void* data) {
    current_file[0] = 0;
    baba_label_set_text(info, "Open an image file to view");
}

int main(void) {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "Image Viewer", 700, 550);
    
    BabaButton* btn_open = baba_button_create(win, "Open Image", 10, 10, 100, 30);
    baba_button_set_callback(btn_open, on_open, NULL);
    
    BabaButton* btn_save = baba_button_create(win, "Save PNG", 120, 10, 100, 30);
    baba_button_set_callback(btn_save, on_save, NULL);
    
    BabaButton* btn_info = baba_button_create(win, "Info", 230, 10, 80, 30);
    baba_button_set_callback(btn_info, on_info, NULL);
    
    BabaButton* btn_clear = baba_button_create(win, "Clear", 320, 10, 80, 30);
    baba_button_set_callback(btn_clear, on_clear, NULL);
    
    image = baba_image_create(win, 10, 50, 680, 420);
    info = baba_label_create(win, "Open an image file to view", 10, 490, 680, 25);
    
    baba_window_show(win);
    baba_app_run(app);
    
    baba_image_destroy(image);
    baba_label_destroy(info);
    baba_button_destroy(btn_open);
    baba_button_destroy(btn_save);
    baba_button_destroy(btn_info);
    baba_button_destroy(btn_clear);
    baba_window_destroy(win);
    baba_app_destroy(app);
    
    return 0;
}