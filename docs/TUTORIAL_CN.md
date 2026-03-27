# 把拔GUI教程

## 目录

1. [简介](#简介)
2. [快速开始](#快速开始)
3. [安装配置](#安装配置)
4. [核心概念](#核心概念)
5. [基础教程](#基础教程)
6. [进阶教程](#进阶教程)
7. [API参考](#api参考)
8. [最佳实践](#最佳实践)
9. [常见问题](#常见问题)

---

## 简介

**把拔（Baba）** 是一个轻量级、跨平台的C语言GUI框架，使用Vulkan进行GPU加速渲染。

### 核心特点

| 特性 | 描述 |
|------|------|
| 最小依赖 | 仅需Vulkan SDK |
| 跨平台 | macOS / Linux / Windows |
| 原生窗口 | 平台原生API |
| 高性能 | Vulkan GPU渲染 |
| 双语支持 | C API + Python绑定 |
| 轻量级 | 静态库约100KB |

### 与其他框架对比

| 框架 | 依赖 | 大小 | 渲染 | 语言 |
|------|------|------|------|------|
| **Baba** | Vulkan | ~100KB | Vulkan | C/Python |
| Qt | 多模块 | ~500MB | OpenGL/Vulkan | C++ |
| GTK | 多库 | ~50MB | Cairo | C |
| Dear ImGui | 少 | ~1MB | 用户实现 | C++ |

---

## 快速开始

### Python版本（5分钟）

```bash
pip install baba-gui
```

```python
import baba

with baba.create_app() as app:
    window = app.create_window("你好 把拔", 800, 600)
    
    label = baba.Label("你好，世界！")
    label.font_size = 32
    window.root.add_child(label)
    
    window.show()
    app.run()
```

### C语言版本

```bash
# macOS
brew install vulkan-loader vulkan-headers molten-vk
git clone https://github.com/cycleuser/Baba
cd Baba && mkdir build && cd build && cmake .. && make
```

```c
#include <baba.h>
#include <widgets/label.h>

int main() {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "你好 把拔", 800, 600);
    
    BabaWidget* label = baba_label_create("你好，世界！");
    baba_label_set_font_size(label, 32.0f);
    baba_widget_add_child(baba_window_get_root(win), label);
    
    baba_window_show(win);
    baba_app_run(app);
    
    baba_window_destroy(win);
    baba_app_destroy(app);
    return 0;
}
```

```bash
clang main.c -I dist/include -L dist/lib/macos -lbaba \
    -framework Cocoa -framework Metal -lvulkan -o myapp
```

---

## 安装配置

### 系统要求

| 平台 | 最低版本 | 编译器 |
|------|----------|--------|
| macOS | 10.14 Mojave | Clang 10+ |
| Linux | glibc 2.17+ | GCC 7+ / Clang 8+ |
| Windows | 10 | MinGW-w64 / MSVC 2019+ |

### Vulkan SDK安装

#### macOS

```bash
brew install vulkan-loader vulkan-headers molten-vk
```

#### Linux

```bash
# Ubuntu/Debian
sudo apt install libvulkan-dev

# Arch Linux
sudo pacman -S vulkan-headers vulkan-tools
```

#### Windows

```powershell
scoop install vulkan-sdk
# 或从 https://vulkan.lunarg.com/ 下载
```

### 从源码编译

```bash
git clone https://github.com/cycleuser/Baba
cd Baba
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
sudo cmake --install .
```

---

## 核心概念

### 架构

```
┌─────────────────────────────────────────────┐
│             应用层                           │
│         (App, Window, 事件循环)             │
├─────────────────────────────────────────────┤
│             控件层                           │
│         (Button, Label, TextBox)           │
├─────────────────────────────────────────────┤
│             布局层                           │
│         (Flex, Grid Layout)                │
├─────────────────────────────────────────────┤
│             渲染层                           │
│         (Painter, Font, Shapes)            │
├─────────────────────────────────────────────┤
│             Vulkan后端                       │
│       (Device, Swapchain, Pipeline)        │
├─────────────────────────────────────────────┤
│             平台层                           │
│         (macOS/Linux/Windows)              │
└─────────────────────────────────────────────┘
```

### 核心对象

- **BabaApp**: 应用根对象，管理窗口和事件循环
- **BabaWindow**: UI的顶层容器
- **BabaWidget**: 所有UI元素的基类

### 坐标系统

- 原点在左上角
- X轴向右为正
- Y轴向下为正
- 单位为逻辑像素

---

## 基础教程

### 第一课：Hello World

```c
#include <baba.h>
#include <widgets/label.h>

int main() {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "你好", 640, 480)
    
    BabaWidget* label = baba_label_create("你好，把拔！");
    baba_label_set_font_size(label, 32.0f);
    baba_widget_add_child(baba_window_get_root(win), label)
    
    baba_window_show(win);
    int result = baba_app_run(app);
    
    baba_window_destroy(win);
    baba_app_destroy(app);
    return result;
}
```

### 第二课：按钮与事件

```c
#include <baba.h>
#include <widgets/button.h>
#include <widgets/label.h>
#include <stdio.h>

static BabaWidget* g_label;

void on_click(BabaWidget* button, void* user_data) {
    static int count = 0;
    count++;
    char text[64];
    snprintf(text, sizeof(text), "点击次数: %d", count);
    baba_label_set_text(g_label, text);
}

int main() {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "按钮示例", 400, 300)
    
    g_label = baba_label_create("点击次数: 0");
    baba_label_set_font_size(g_label, 24.0f);
    baba_widget_add_child(baba_window_get_root(win), g_label)
    
    BabaWidget* btn = baba_button_create("点击我");
    baba_button_set_on_click(btn, on_click, NULL);
    baba_widget_add_child(baba_window_get_root(win), btn)
    
    baba_window_show(win);
    baba_app_run(app);
    
    baba_window_destroy(win);
    baba_app_destroy(app);
    return 0;
}
```

### 第三课：布局系统

```c
#include <baba.h>
#include <widgets/button.h>
#include <layout/layout.h>

int main() {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "布局示例", 600, 400)
    
    BabaWidget* flex = baba_flex_layout_create(BABA_LAYOUT_DIRECTION_ROW);
    baba_flex_layout_set_gap(flex, 10.0f);
    baba_widget_add_child(baba_window_get_root(win), flex)
    
    const char* labels[] = {"按钮1", "按钮2", "按钮3", "按钮4"};
    for (int i = 0; i < 4; i++) {
        BabaWidget* btn = baba_button_create(labels[i]);
        baba_widget_add_child(flex, btn);
    }
    
    baba_window_show(win);
    baba_app_run(app);
    
    baba_window_destroy(win);
    baba_app_destroy(app);
    return 0;
}
```

---

## 进阶教程

### 自定义控件

```c
typedef struct {
    float value;
    BabaColor fill_color;
} ProgressBarData;

static void progress_bar_draw(BabaWidget* widget, BabaPainter* painter) {
    ProgressBarData* data = widget->user_data;
    BabaRect bounds = baba_widget_get_bounds(widget);
    
    // 背景
    baba_painter_set_fill_color(painter, (BabaColor){0.2f, 0.2f, 0.2f, 1.0f});
    baba_painter_rounded_rect(painter, bounds, 5.0f);
    
    // 进度
    BabaRect fill = {bounds.x, bounds.y, bounds.width * data->value, bounds.height};
    baba_painter_set_fill_color(painter, data->fill_color);
    baba_painter_rounded_rect(painter, fill, 5.0f);
}

BabaWidget* progress_bar_create(float value) {
    BabaWidget* widget = baba_widget_create(&(BabaWidgetVTable){
        .draw = progress_bar_draw
    }, sizeof(ProgressBarData));
    
    ProgressBarData* data = widget->user_data;
    data->value = value;
    data->fill_color = (BabaColor){0.3f, 0.7f, 0.3f, 1.0f};
    return widget;
}
```

---

## API参考

### 应用管理

```c
BabaApp* baba_app_create(void);
void baba_app_destroy(BabaApp* app);
int baba_app_run(BabaApp* app);
void baba_app_quit(BabaApp* app);
```

### 窗口操作

```c
BabaWindow* baba_window_create(BabaApp* app, const char* title, int width, int height);
void baba_window_destroy(BabaWindow* window);
void baba_window_set_title(BabaWindow* window, const char* title);
void baba_window_set_size(BabaWindow* window, int width, int height);
void baba_window_show(BabaWindow* window);
void baba_window_close(BabaWindow* window);
BabaWidget* baba_window_get_root(BabaWindow* window);
```

### 控件系统

```c
// 按钮
BabaWidget* baba_button_create(const char* text);
void baba_button_set_text(BabaWidget* button, const char* text);
void baba_button_set_on_click(BabaWidget* button, void (*cb)(BabaWidget*, void*), void* data);

// 标签
BabaWidget* baba_label_create(const char* text);
void baba_label_set_text(BabaWidget* label, const char* text);
void baba_label_set_font_size(BabaWidget* label, float size);
void baba_label_set_color(BabaWidget* label, BabaColor color);

// 文本框
BabaWidget* baba_textbox_create(const char* placeholder);
void baba_textbox_set_text(BabaWidget* textbox, const char* text);
const char* baba_textbox_get_text(BabaWidget* textbox);
```

### 布局系统

```c
BabaWidget* baba_flex_layout_create(BabaLayoutDirection direction);
void baba_flex_layout_set_gap(BabaWidget* layout, float gap);

BabaWidget* baba_grid_layout_create(int columns, int rows);
```

---

## 最佳实践

### 内存管理

```c
// 正确：按相反顺序销毁
BabaApp* app = baba_app_create();
BabaWindow* win = baba_window_create(app, ...);
baba_window_destroy(win);
baba_app_destroy(app);
```

### 性能优化

```c
// 只在需要时更新
if (data_changed) {
    baba_widget_invalidate(widget);
}
```

---

## 常见问题

### Q: 找不到vulkan.h？

```bash
export VULKAN_SDK=/path/to/vulkansdk
export CMAKE_PREFIX_PATH=$VULKAN_SDK
```

### Q: 窗口显示但无内容？

1. 检查是否调用了 `baba_window_show()`
2. 检查是否调用了 `baba_app_run()`
3. 检查控件是否添加到root