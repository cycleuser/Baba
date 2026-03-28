#define BABA_IMPLEMENTATION
#include "../baba.h"

typedef struct {
    BabaTextField* display;
    char value[64];
    char stored[64];
    char op;
    bool new_input;
    bool has_stored;
} Calc;

static Calc g_calc;

static void update(void) {
    baba_textfield_set_text(g_calc.display, g_calc.value);
}

static void on_digit(BabaButton* btn, void* data) {
    char d = (char)(intptr_t)data;
    if (g_calc.new_input) {
        g_calc.value[0] = d;
        g_calc.value[1] = 0;
        g_calc.new_input = false;
    } else {
        int len = strlen(g_calc.value);
        if (len < 15 && strcmp(g_calc.value, "0") != 0) {
            g_calc.value[len] = d;
            g_calc.value[len+1] = 0;
        } else if (len < 15) {
            g_calc.value[0] = d;
            g_calc.value[1] = 0;
        }
    }
    update();
}

static void on_op(BabaButton* btn, void* data) {
    char op = (char)(intptr_t)data;
    if (g_calc.has_stored && !g_calc.new_input) {
        double a = atof(g_calc.stored), b = atof(g_calc.value), r = 0;
        switch (g_calc.op) {
            case '+': r = a+b; break;
            case '-': r = a-b; break;
            case '*': r = a*b; break;
            case '/': r = b? a/b : 0; break;
        }
        snprintf(g_calc.value, 64, "%.10g", r);
        update();
    }
    strcpy(g_calc.stored, g_calc.value);
    g_calc.op = op;
    g_calc.has_stored = true;
    g_calc.new_input = true;
}

static void on_eq(BabaButton* btn, void* data) {
    if (!g_calc.has_stored) return;
    double a = atof(g_calc.stored), b = atof(g_calc.value), r = 0;
    switch (g_calc.op) {
        case '+': r = a+b; break;
        case '-': r = a-b; break;
        case '*': r = a*b; break;
        case '/': r = b? a/b : 0; break;
        default: r = b; break;
    }
    snprintf(g_calc.value, 64, "%.10g", r);
    update();
    g_calc.has_stored = false;
    g_calc.new_input = true;
}

static void on_clear(BabaButton* btn, void* data) {
    strcpy(g_calc.value, "0");
    g_calc.stored[0] = 0;
    g_calc.op = 0;
    g_calc.has_stored = false;
    g_calc.new_input = true;
    update();
}

static void on_neg(BabaButton* btn, void* data) {
    double v = -atof(g_calc.value);
    snprintf(g_calc.value, 64, "%.10g", v);
    update();
}

static void on_pct(BabaButton* btn, void* data) {
    double v = atof(g_calc.value) / 100.0;
    snprintf(g_calc.value, 64, "%.10g", v);
    update();
}

static void on_dot(BabaButton* btn, void* data) {
    if (g_calc.new_input) {
        strcpy(g_calc.value, "0.");
        g_calc.new_input = false;
    } else if (!strchr(g_calc.value, '.')) {
        strcat(g_calc.value, ".");
    }
    update();
}

int main(void) {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "Calculator", 280, 400);
    
    memset(&g_calc, 0, sizeof(g_calc));
    strcpy(g_calc.value, "0");
    g_calc.new_input = true;
    
    float m = 5, bw = 60, bh = 50, sy = 60;
    g_calc.display = baba_textfield_create(win, "0", m, m, 270, 50, false);
    
    BabaButton* btns[19] = {0};
    const char* labels[4][4] = {
        {"C","#","%","/"}, {"7","8","9","*"}, {"4","5","6","-"}, {"1","2","3","+"}
    };
    int ids[4][4] = {
        {-1,-2,-3,'/'}, {'7','8','9','*'}, {'4','5','6','-'}, {'1','2','3','+'}
    };
    
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            float x = m + c*(bw+m), y = sy + r*(bh+m);
            btns[r*4+c] = baba_button_create(win, labels[r][c], x, y, bw, bh);
            int id = ids[r][c];
            if (id >= '0' && id <= '9') baba_button_set_callback(btns[r*4+c], on_digit, (void*)(intptr_t)id);
            else if (id == -1) baba_button_set_callback(btns[r*4+c], on_clear, NULL);
            else if (id == -2) baba_button_set_callback(btns[r*4+c], on_neg, NULL);
            else if (id == -3) baba_button_set_callback(btns[r*4+c], on_pct, NULL);
            else baba_button_set_callback(btns[r*4+c], on_op, (void*)(intptr_t)id);
        }
    }
    
    btns[16] = baba_button_create(win, "0", m, sy+4*(bh+m), bw*2+m, bh);
    baba_button_set_callback(btns[16], on_digit, (void*)(intptr_t)'0');
    btns[17] = baba_button_create(win, ".", m+2*(bw+m), sy+4*(bh+m), bw, bh);
    baba_button_set_callback(btns[17], on_dot, NULL);
    btns[18] = baba_button_create(win, "=", m+3*(bw+m), sy+4*(bh+m), bw, bh);
    baba_button_set_callback(btns[18], on_eq, NULL);
    
    baba_window_show(win);
    baba_app_run(app);
    
    for (int i = 0; i < 19; i++) baba_button_destroy(btns[i]);
    baba_textfield_destroy(g_calc.display);
    baba_window_destroy(win);
    baba_app_destroy(app);
    return 0;
}