#define BABA_IMPLEMENTATION
#include "../baba.h"

static BabaTextField* editor = NULL;
static BabaLabel* status = NULL;
static char current_file[512] = "";

static void on_new(BabaButton* btn, void* data) {
    baba_textfield_set_text(editor, "");
    current_file[0] = 0;
    baba_label_set_text(status, "New file");
}

static void on_open(BabaButton* btn, void* data) {
    char* path = baba_file_open_dialog(NULL, "Open Text File", "txt,c,cpp,h,md,json,xml");
    if (path) {
        FILE* f = fopen(path, "r");
        if (f) {
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* content = malloc(size + 1);
            fread(content, 1, size, f);
            content[size] = 0;
            fclose(f);
            baba_textfield_set_text(editor, content);
            free(content);
            strcpy(current_file, path);
            char msg[256];
            snprintf(msg, sizeof(msg), "Opened: %s (%ld bytes)", path, size);
            baba_label_set_text(status, msg);
        } else {
            baba_label_set_text(status, "Failed to open file");
        }
        free(path);
    }
}

static void on_save(BabaButton* btn, void* data) {
    const char* text = baba_textfield_get_text(editor);
    if (!text || strlen(text) == 0) {
        baba_label_set_text(status, "Nothing to save");
        return;
    }
    
    if (strlen(current_file) == 0) {
        char* path = baba_file_save_dialog(NULL, "Save Text File", "txt");
        if (path) {
            strcpy(current_file, path);
            free(path);
        } else {
            return;
        }
    }
    
    FILE* f = fopen(current_file, "w");
    if (f) {
        fprintf(f, "%s", text);
        fclose(f);
        char msg[256];
        snprintf(msg, sizeof(msg), "Saved: %s", current_file);
        baba_label_set_text(status, msg);
    } else {
        baba_label_set_text(status, "Failed to save file");
    }
}

static void on_save_as(BabaButton* btn, void* data) {
    const char* text = baba_textfield_get_text(editor);
    if (!text || strlen(text) == 0) {
        baba_label_set_text(status, "Nothing to save");
        return;
    }
    
    char* path = baba_file_save_dialog(NULL, "Save Text File As", "txt");
    if (path) {
        FILE* f = fopen(path, "w");
        if (f) {
            fprintf(f, "%s", text);
            fclose(f);
            strcpy(current_file, path);
            char msg[256];
            snprintf(msg, sizeof(msg), "Saved: %s", path);
            baba_label_set_text(status, msg);
        } else {
            baba_label_set_text(status, "Failed to save file");
        }
        free(path);
    }
}

int main(void) {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "Text Editor", 700, 500);
    
    BabaButton* btn_new = baba_button_create(win, "New", 10, 10, 60, 30);
    baba_button_set_callback(btn_new, on_new, NULL);
    
    BabaButton* btn_open = baba_button_create(win, "Open", 80, 10, 60, 30);
    baba_button_set_callback(btn_open, on_open, NULL);
    
    BabaButton* btn_save = baba_button_create(win, "Save", 150, 10, 60, 30);
    baba_button_set_callback(btn_save, on_save, NULL);
    
    BabaButton* btn_save_as = baba_button_create(win, "Save As", 220, 10, 70, 30);
    baba_button_set_callback(btn_save_as, on_save_as, NULL);
    
    editor = baba_textfield_create(win, "", 10, 50, 680, 380, true);
    status = baba_label_create(win, "New file - start typing or open a file", 10, 450, 680, 25);
    
    baba_window_show(win);
    baba_app_run(app);
    
    baba_textfield_destroy(editor);
    baba_label_destroy(status);
    baba_button_destroy(btn_new);
    baba_button_destroy(btn_open);
    baba_button_destroy(btn_save);
    baba_button_destroy(btn_save_as);
    baba_window_destroy(win);
    baba_app_destroy(app);
    
    return 0;
}