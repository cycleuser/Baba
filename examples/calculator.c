#include <baba.h>
#include <widgets/button.h>
#include <widgets/label.h>
#include <layout/layout.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char display_text[64] = "0";
static char expression[256] = "";
static double current_value = 0.0;
static double stored_value = 0.0;
static char current_op = 0;
static bool new_input = true;

static void update_display(BabaWidget* label) {
    baba_label_set_text(label, display_text);
}

static void digit_pressed(BabaWidget* button, void* user_data) {
    BabaWidget* display = (BabaWidget*)user_data;
    const char* text = baba_button_get_text(button);
    
    if (new_input) {
        strcpy(display_text, text);
        new_input = false;
    } else {
        if (strlen(display_text) < 15) {
            if (strcmp(display_text, "0") == 0) {
                strcpy(display_text, text);
            } else {
                strcat(display_text, text);
            }
        }
    }
    
    update_display(display);
}

static void decimal_pressed(BabaWidget* button, void* user_data) {
    BabaWidget* display = (BabaWidget*)user_data;
    
    if (new_input) {
        strcpy(display_text, "0.");
        new_input = false;
    } else if (strchr(display_text, '.') == NULL) {
        strcat(display_text, ".");
    }
    
    update_display(display);
}

static void clear_pressed(BabaWidget* button, void* user_data) {
    BabaWidget* display = (BabaWidget*)user_data;
    
    strcpy(display_text, "0");
    expression[0] = '\0';
    current_value = 0.0;
    stored_value = 0.0;
    current_op = 0;
    new_input = true;
    
    update_display(display);
}

static void operator_pressed(BabaWidget* button, void* user_data) {
    BabaWidget* display = (BabaWidget*)user_data;
    const char* text = baba_button_get_text(button);
    
    current_value = atof(display_text);
    current_op = text[0];
    stored_value = current_value;
    new_input = true;
    
    snprintf(expression, sizeof(expression), "%.10g %c", stored_value, current_op);
}

static void calculate(BabaWidget* display) {
    double result = 0.0;
    double second = atof(display_text);
    
    switch (current_op) {
        case '+':
            result = stored_value + second;
            break;
        case '-':
            result = stored_value - second;
            break;
        case '*':
            result = stored_value * second;
            break;
        case '/':
            if (second != 0.0) {
                result = stored_value / second;
            } else {
                strcpy(display_text, "Error");
                update_display(display);
                new_input = true;
                return;
            }
            break;
        default:
            result = second;
            break;
    }
    
    if (result == (long long)result) {
        snprintf(display_text, sizeof(display_text), "%lld", (long long)result);
    } else {
        snprintf(display_text, sizeof(display_text), "%.10g", result);
    }
    
    current_value = result;
    current_op = 0;
    new_input = true;
    
    update_display(display);
}

static void equals_pressed(BabaWidget* button, void* user_data) {
    BabaWidget* display = (BabaWidget*)user_data;
    calculate(display);
}

static void percent_pressed(BabaWidget* button, void* user_data) {
    BabaWidget* display = (BabaWidget*)user_data;
    
    double value = atof(display_text);
    value = value / 100.0;
    
    if (value == (long long)value) {
        snprintf(display_text, sizeof(display_text), "%lld", (long long)value);
    } else {
        snprintf(display_text, sizeof(display_text), "%.10g", value);
    }
    
    update_display(display);
}

static void negate_pressed(BabaWidget* button, void* user_data) {
    BabaWidget* display = (BabaWidget*)user_data;
    
    double value = atof(display_text);
    value = -value;
    
    if (value == (long long)value) {
        snprintf(display_text, sizeof(display_text), "%lld", (long long)value);
    } else {
        snprintf(display_text, sizeof(display_text), "%.10g", value);
    }
    
    update_display(display);
}

static BabaWidget* create_button(const char* text, BabaWidget* display, 
                                  void (*handler)(BabaWidget*, void*)) {
    BabaWidget* btn = baba_button_create(text);
    baba_button_set_on_click(btn, handler, display);
    return btn;
}

int main(int argc, char** argv) {
    BabaApp* app = baba_app_create();
    if (!app) {
        fprintf(stderr, "Failed to create app\n");
        return 1;
    }
    
    BabaWindow* window = baba_window_create(app, "把拔计算器 / Baba Calculator", 320, 480);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        baba_app_destroy(app);
        return 1;
    }
    
    BabaWidget* root = baba_window_get_root(window);
    
    BabaWidget* display = baba_label_create("0");
    baba_label_set_font_size(display, 36.0f);
    baba_label_set_color(display, (BabaColor){1.0f, 1.0f, 1.0f, 1.0f});
    baba_widget_set_bounds(display, (BabaRect){10, 10, 300, 80});
    baba_widget_add_child(root, display);
    
    BabaWidget* grid = baba_grid_layout_create(4, 5);
    baba_grid_layout_set_column_gap(grid, 5);
    baba_grid_layout_set_row_gap(grid, 5);
    baba_widget_set_bounds(grid, (BabaRect){10, 100, 300, 370});
    baba_widget_add_child(root, grid);
    
    baba_widget_add_child(grid, create_button("C", display, clear_pressed));
    baba_widget_add_child(grid, create_button("±", display, negate_pressed));
    baba_widget_add_child(grid, create_button("%", display, percent_pressed));
    baba_widget_add_child(grid, create_button("/", display, operator_pressed));
    
    baba_widget_add_child(grid, create_button("7", display, digit_pressed));
    baba_widget_add_child(grid, create_button("8", display, digit_pressed));
    baba_widget_add_child(grid, create_button("9", display, digit_pressed));
    baba_widget_add_child(grid, create_button("*", display, operator_pressed));
    
    baba_widget_add_child(grid, create_button("4", display, digit_pressed));
    baba_widget_add_child(grid, create_button("5", display, digit_pressed));
    baba_widget_add_child(grid, create_button("6", display, digit_pressed));
    baba_widget_add_child(grid, create_button("-", display, operator_pressed));
    
    baba_widget_add_child(grid, create_button("1", display, digit_pressed));
    baba_widget_add_child(grid, create_button("2", display, digit_pressed));
    baba_widget_add_child(grid, create_button("3", display, digit_pressed));
    baba_widget_add_child(grid, create_button("+", display, operator_pressed));
    
    baba_widget_add_child(grid, create_button("0", display, digit_pressed));
    baba_widget_add_child(grid, create_button(".", display, decimal_pressed));
    baba_widget_add_child(grid, create_button("=", display, equals_pressed));
    
    baba_window_show(window);
    
    int result = baba_app_run(app);
    
    baba_window_destroy(window);
    baba_app_destroy(app);
    
    return result;
}