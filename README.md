# Baba

一个简单的单头文件跨平台 GUI 库，类似于 stb 系列。

## 特点

- **单头文件**: 只需 `#include "baba.h"` 
- **无外部依赖**: 使用平台原生 API
- **跨平台**: macOS (Cocoa), Windows (Win32), Linux (X11)
- **简单编译**: 一行命令即可编译

## 使用方法

```c
#define BABA_IMPLEMENTATION
#include "baba.h"

int main(void) {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "Hello", 400, 300);
    
    BabaButton* btn = baba_button_create(win, "Click Me", 50, 50, 100, 30);
    baba_button_set_callback(btn, my_callback, NULL);
    
    baba_window_show(win);
    baba_app_run(app);
    return 0;
}
```

## 编译

### macOS
```sh
clang -x objective-c -framework Cocoa your_file.c -o your_app
```

### Windows (MinGW)
```sh
gcc your_file.c -luser32 -lgdi32 -lcomdlg32 -lshell32 -o your_app.exe
```

### Linux
```sh
gcc your_file.c -lX11 -o your_app
```

## API

### 应用 & 窗口
```c
BabaApp* baba_app_create(void);
void baba_app_destroy(BabaApp* app);
int baba_app_run(BabaApp* app);
void baba_app_quit(BabaApp* app);

BabaWindow* baba_window_create(BabaApp* app, const char* title, int width, int height);
void baba_window_destroy(BabaWindow* window);
void baba_window_set_title(BabaWindow* window, const char* title);
void baba_window_show(BabaWindow* window);
void baba_window_close(BabaWindow* window);
void baba_window_get_size(BabaWindow* window, int* w, int* h);
```

### 控件
```c
// 按钮
BabaButton* baba_button_create(BabaWindow* window, const char* text, float x, float y, float w, float h);
void baba_button_set_text(BabaButton* button, const char* text);
void baba_button_set_callback(BabaButton* button, BabaButtonCallback callback, void* userdata);

// 标签
BabaLabel* baba_label_create(BabaWindow* window, const char* text, float x, float y, float w, float h);
void baba_label_set_text(BabaLabel* label, const char* text);

// 文本框
BabaTextField* baba_textfield_create(BabaWindow* window, const char* text, float x, float y, float w, float h, bool editable);
void baba_textfield_set_text(BabaTextField* field, const char* text);
const char* baba_textfield_get_text(BabaTextField* field);

// 表格
BabaTableView* baba_table_create(BabaWindow* window, float x, float y, float w, float h);
void baba_table_set_headers(BabaTableView* table, const char** headers, int count);
void baba_table_add_row(BabaTableView* table, const char** values, int count);
void baba_table_clear(BabaTableView* table);
int baba_table_get_row_count(BabaTableView* table);
void baba_table_get_cell(BabaTableView* table, int row, int col, char* buffer, int bufsize);

// 图片
BabaImageView* baba_image_create(BabaWindow* window, float x, float y, float w, float h);
bool baba_image_load_file(BabaImageView* image, const char* path);
bool baba_image_save_file(BabaImageView* image, const char* path);
void baba_image_get_size(BabaImageView* image, int* w, int* h);
```

### 文件对话框
```c
char* baba_file_open_dialog(BabaWindow* window, const char* title, const char* filter);
char* baba_file_save_dialog(BabaWindow* window, const char* title, const char* filter);
char* baba_file_open_folder_dialog(BabaWindow* window, const char* title);
```

## 示例

| 文件 | 功能 |
|------|------|
| `calculator.c` | 计算器，支持四则运算 |
| `csv_viewer.c` | CSV 文件查看器/编辑器 |
| `image_viewer.c` | 图片查看器，支持加载/保存 |
| `text_editor.c` | 简易文本编辑器 |
| `demo.c` | 最小示例 |

编译运行：
```sh
cd examples
clang -x objective-c -framework Cocoa calculator.c -o calculator
./calculator
```

## 平台支持

| 平台 | 窗口系统 | 按钮/标签 | 表格 | 图片 | 文件对话框 |
|------|----------|-----------|------|------|------------|
| macOS | Cocoa | NSButton/NSTextField | NSTableView | NSImageView | NSOpenPanel |
| Windows | Win32 | BUTTON/STATIC | LISTBOX | STATIC BITMAP | GetOpenFileName |
| Linux | X11 | 手绘 | 手绘列表 | 基础 | zenity(可选) |

## License

MIT