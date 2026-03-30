# Baba

简单跨平台GUI库。

## 两种使用方式

### 1. Python (推荐)

```bash
pip install baba-gui
```

```python
import baba

app = baba.App()
win = baba.Window(app, "Baba Demo", 400, 300)
btn = baba.Button(win, "Click Me", 150, 100, 100, 40)
btn.set_callback(lambda b: print("clicked!"))
win.show()
app.run()
```

### 2. C语言

单头文件 `baba.h`：

```c
#define BABA_IMPLEMENTATION
#include "baba.h"

int main(void) {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "Hello", 400, 300);
    baba_window_show(win);
    baba_app_run(app);
    return 0;
}
```

编译：
```bash
# macOS
clang -x objective-c -framework Cocoa main.c -o app

# Windows
gcc main.c -luser32 -lgdi32 -o app.exe

# Linux
gcc main.c -lX11 -o app
```

## Python API

```python
import baba

# 应用
app = baba.App()
app.run()
app.quit()

# 窗口
win = baba.Window(app, "Title", width, height)
win.set_title("New Title")
win.show()
win.close()

# 按钮
btn = baba.Button(win, "Text", x, y, w, h)
btn.set_text("New Text")
btn.set_callback(callback_func)

# 标签
label = baba.Label(win, "Text", x, y, w, h)
label.set_text("New Text")

# 文本框
field = baba.TextField(win, "initial", x, y, w, h, editable=True)
field.set_text("New Text")
text = field.get_text()

# 表格
table = baba.TableView(win, x, y, w, h)
table.set_headers(["A", "B", "C"])
table.add_row(["1", "2", "3"])
table.clear()

# 图片
img = baba.ImageView(win, x, y, w, h)
img.load_file("/path/to/image.png")
img.save_file("/path/to/save.png")

# 文件对话框
path = baba.file_open_dialog("Open", "csv,txt")
path = baba.file_save_dialog("Save", "png")
```

## 示例

- `examples/calculator.c` - 计算器 (C)
- `examples/csv_viewer.c` - CSV查看器 (C)
- `examples/image_viewer.c` - 图片查看器 (C)
- `python/demo.py` - 简单示例 (Python)

## 平台支持

| 平台 | Python | C |
|------|--------|---|
| macOS | PyObjC | Cocoa |
| Windows | ctypes | Win32 |
| Linux | 计划中 | X11 |

## License

MIT