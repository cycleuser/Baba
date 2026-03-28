#define BABA_IMPLEMENTATION
#include "../baba.h"

static BabaTableView* table = NULL;
static BabaLabel* status = NULL;
static char current_file[512] = "";
static char** csv_data = NULL;
static int csv_rows = 0;
static int csv_cols = 0;

static void parse_csv(const char* line, char** values, int max_cols) {
    int col = 0;
    const char* start = line;
    bool in_quotes = false;
    
    for (const char* p = line; *p && col < max_cols; p++) {
        if (*p == '"') in_quotes = !in_quotes;
        else if (*p == ',' && !in_quotes) {
            int len = p - start;
            values[col] = malloc(len + 1);
            strncpy(values[col], start, len);
            values[col][len] = 0;
            if (values[col][0] == '"') {
                memmove(values[col], values[col]+1, len-2);
                values[col][len-2] = 0;
            }
            col++;
            start = p + 1;
        }
    }
    if (col < max_cols) {
        int len = strlen(start);
        if (start[len-1] == '\n') len--;
        if (start[len-1] == '\r') len--;
        values[col] = malloc(len + 1);
        strncpy(values[col], start, len);
        values[col][len] = 0;
        if (values[col][0] == '"') {
            memmove(values[col], values[col]+1, len-2);
            values[col][len-2] = 0;
        }
    }
}

static void free_csv(void) {
    if (csv_data) {
        for (int i = 0; i < csv_rows * csv_cols; i++) free(csv_data[i]);
        free(csv_data);
        csv_data = NULL;
    }
    csv_rows = 0;
    csv_cols = 0;
}

static void load_csv(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) { baba_label_set_text(status, "Failed to open file"); return; }
    
    free_csv();
    baba_table_clear(table);
    strcpy(current_file, path);
    
    char line[4096];
    bool first = true;
    int max_cols = 20;
    
    while (fgets(line, sizeof(line), f)) {
        int col_count = 0;
        for (char* p = line; *p; p++) if (*p == ',') col_count++;
        col_count++;
        if (col_count > max_cols) max_cols = col_count;
        
        char* values[50] = {0};
        parse_csv(line, values, max_cols);
        
        if (first) {
            csv_cols = max_cols;
            baba_table_set_headers(table, (const char**)values, max_cols);
            first = false;
        } else {
            baba_table_add_row(table, (const char**)values, max_cols);
        }
        
        for (int i = 0; i < max_cols; i++) {
            if (values[i]) {
                csv_data = realloc(csv_data, sizeof(char*) * (csv_rows * csv_cols + csv_cols + 1));
                csv_data[csv_rows * csv_cols + i] = values[i];
            }
        }
        csv_rows++;
    }
    fclose(f);
    
    char msg[256];
    snprintf(msg, sizeof(msg), "Loaded: %s (%d rows)", path, csv_rows);
    baba_label_set_text(status, msg);
}

static void save_csv(const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) { baba_label_set_text(status, "Failed to save file"); return; }
    
    int rows = baba_table_get_row_count(table);
    char buf[256];
    
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < csv_cols; c++) {
            baba_table_get_cell(table, r, c, buf, sizeof(buf));
            if (c > 0) fprintf(f, ",");
            if (strchr(buf, ',') || strchr(buf, '"')) fprintf(f, "\"%s\"", buf);
            else fprintf(f, "%s", buf);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    
    char msg[256];
    snprintf(msg, sizeof(msg), "Saved: %s", path);
    baba_label_set_text(status, msg);
}

static void on_open(BabaButton* btn, void* data) {
    char* path = baba_file_open_dialog(NULL, "Open CSV", "csv,txt");
    if (path) { load_csv(path); free(path); }
}

static void on_save(BabaButton* btn, void* data) {
    if (strlen(current_file) == 0) {
        char* path = baba_file_save_dialog(NULL, "Save CSV", "csv");
        if (path) { save_csv(path); strcpy(current_file, path); free(path); }
    } else {
        save_csv(current_file);
    }
}

static void on_save_as(BabaButton* btn, void* data) {
    char* path = baba_file_save_dialog(NULL, "Save CSV As", "csv");
    if (path) { save_csv(path); strcpy(current_file, path); free(path); }
}

static void on_clear(BabaButton* btn, void* data) {
    free_csv();
    baba_table_clear(table);
    current_file[0] = 0;
    baba_label_set_text(status, "Cleared");
}

int main(void) {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "CSV Viewer", 800, 500);
    
    BabaButton* btn_open = baba_button_create(win, "Open CSV", 10, 10, 100, 30);
    baba_button_set_callback(btn_open, on_open, NULL);
    
    BabaButton* btn_save = baba_button_create(win, "Save", 120, 10, 80, 30);
    baba_button_set_callback(btn_save, on_save, NULL);
    
    BabaButton* btn_save_as = baba_button_create(win, "Save As", 210, 10, 80, 30);
    baba_button_set_callback(btn_save_as, on_save_as, NULL);
    
    BabaButton* btn_clear = baba_button_create(win, "Clear", 300, 10, 80, 30);
    baba_button_set_callback(btn_clear, on_clear, NULL);
    
    table = baba_table_create(win, 10, 50, 780, 380);
    status = baba_label_create(win, "Open a CSV file to begin", 10, 450, 780, 25);
    
    baba_window_show(win);
    baba_app_run(app);
    
    baba_table_destroy(table);
    baba_label_destroy(status);
    baba_button_destroy(btn_open);
    baba_button_destroy(btn_save);
    baba_button_destroy(btn_save_as);
    baba_button_destroy(btn_clear);
    baba_window_destroy(win);
    baba_app_destroy(app);
    
    free_csv();
    return 0;
}