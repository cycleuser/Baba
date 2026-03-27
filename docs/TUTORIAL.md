# Baba GUI Tutorial

## Table of Contents

1. [Introduction](#introduction)
2. [Quick Start](#quick-start)
3. [Installation](#installation)
4. [Core Concepts](#core-concepts)
5. [Basic Tutorial](#basic-tutorial)
6. [Advanced Tutorial](#advanced-tutorial)
7. [API Reference](#api-reference)
8. [Best Practices](#best-practices)
9. [FAQ](#faq)

---

## Introduction

**Baba** is a lightweight, cross-platform C GUI framework using Vulkan for GPU-accelerated rendering.

### Key Features

| Feature | Description |
|---------|-------------|
| Minimal Dependencies | Only Vulkan SDK required |
| Cross-Platform | macOS / Linux / Windows |
| Native Windows | Platform-native APIs |
| High Performance | Vulkan GPU rendering |
| Dual Language | C API + Python bindings |
| Lightweight | Static library ~100KB |

### Comparison with Other Frameworks

| Framework | Dependencies | Size | Rendering | Language |
|-----------|--------------|------|-----------|----------|
| **Baba** | Vulkan | ~100KB | Vulkan | C/Python |
| Qt | Multiple | ~500MB | OpenGL/Vulkan | C++ |
| GTK | Multiple | ~50MB | Cairo | C |
| Dear ImGui | Minimal | ~1MB | User-defined | C++ |

---

## Quick Start

### Python (5 Minutes)

```bash
pip install baba-gui
```

```python
import baba

with baba.create_app() as app:
    window = app.create_window("Hello Baba", 800, 600)
    
    label = baba.Label("Hello, World!")
    label.font_size = 32
    window.root.add_child(label)
    
    window.show()
    app.run()
```

### C Language

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
    BabaWindow* win = baba_window_create(app, "Hello Baba", 800, 600);
    
    BabaWidget* label = baba_label_create("Hello, World!");
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

## Installation

### System Requirements

| Platform | Minimum Version | Compiler |
|----------|-----------------|----------|
| macOS | 10.14 Mojave | Clang 10+ |
| Linux | glibc 2.17+ | GCC 7+ / Clang 8+ |
| Windows | 10 | MinGW-w64 / MSVC 2019+ |

### Vulkan SDK Installation

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
# Or download from https://vulkan.lunarg.com/
```

### Build from Source

```bash
git clone https://github.com/cycleuser/Baba
cd Baba
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
sudo cmake --install .
```

---

## Core Concepts

### Architecture

```
┌─────────────────────────────────────────────┐
│           Application Layer                 │
│         (App, Window, Event Loop)          │
├─────────────────────────────────────────────┤
│           Widget Layer                      │
│       (Button, Label, TextBox)             │
├─────────────────────────────────────────────┤
│           Layout Layer                      │
│         (Flex, Grid Layout)                │
├─────────────────────────────────────────────┤
│           Rendering Layer                   │
│        (Painter, Font, Shapes)             │
├─────────────────────────────────────────────┤
│           Vulkan Backend                    │
│   (Device, Swapchain, Pipeline)            │
├─────────────────────────────────────────────┤
│           Platform Layer                    │
│      (macOS/Linux/Windows)                 │
└─────────────────────────────────────────────┘
```

### Core Objects

- **BabaApp**: Application root, manages windows and event loop
- **BabaWindow**: Top-level container for UI
- **BabaWidget**: Base class for all UI elements

### Coordinate System

- Origin at top-left corner
- X-axis positive to the right
- Y-axis positive downward
- Units in logical pixels

---

## Basic Tutorial

### Lesson 1: Hello World

```c
#include <baba.h>
#include <widgets/label.h>

int main() {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "Hello", 640, 480);
    
    BabaWidget* label = baba_label_create("Hello, Baba!");
    baba_label_set_font_size(label, 32.0f);
    baba_widget_add_child(baba_window_get_root(win), label);
    
    baba_window_show(win);
    int result = baba_app_run(app);
    
    baba_window_destroy(win);
    baba_app_destroy(app);
    return result;
}
```

### Lesson 2: Button and Events

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
    snprintf(text, sizeof(text), "Clicks: %d", count);
    baba_label_set_text(g_label, text);
}

int main() {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "Button Demo", 400, 300);
    
    g_label = baba_label_create("Clicks: 0");
    baba_label_set_font_size(g_label, 24.0f);
    baba_widget_add_child(baba_window_get_root(win), g_label);
    
    BabaWidget* btn = baba_button_create("Click Me");
    baba_button_set_on_click(btn, on_click, NULL);
    baba_widget_add_child(baba_window_get_root(win), btn);
    
    baba_window_show(win);
    baba_app_run(app);
    
    baba_window_destroy(win);
    baba_app_destroy(app);
    return 0;
}
```

### Lesson 3: Layout System

```c
#include <baba.h>
#include <widgets/button.h>
#include <layout/layout.h>

int main() {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "Layout Demo", 600, 400);
    
    BabaWidget* flex = baba_flex_layout_create(BABA_LAYOUT_DIRECTION_ROW);
    baba_flex_layout_set_gap(flex, 10.0f);
    baba_widget_add_child(baba_window_get_root(win), flex);
    
    const char* labels[] = {"Btn1", "Btn2", "Btn3", "Btn4"};
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

## Advanced Tutorial

### Custom Widget

```c
typedef struct {
    float value;
    BabaColor fill_color;
} ProgressBarData;

static void progress_bar_draw(BabaWidget* widget, BabaPainter* painter) {
    ProgressBarData* data = widget->user_data;
    BabaRect bounds = baba_widget_get_bounds(widget);
    
    // Background
    baba_painter_set_fill_color(painter, (BabaColor){0.2f, 0.2f, 0.2f, 1.0f});
    baba_painter_rounded_rect(painter, bounds, 5.0f);
    
    // Progress
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

## API Reference

### Application

```c
BabaApp* baba_app_create(void);
void baba_app_destroy(BabaApp* app);
int baba_app_run(BabaApp* app);
void baba_app_quit(BabaApp* app);
```

### Window

```c
BabaWindow* baba_window_create(BabaApp* app, const char* title, int width, int height);
void baba_window_destroy(BabaWindow* window);
void baba_window_set_title(BabaWindow* window, const char* title);
void baba_window_set_size(BabaWindow* window, int width, int height);
void baba_window_show(BabaWindow* window);
void baba_window_close(BabaWindow* window);
BabaWidget* baba_window_get_root(BabaWindow* window);
```

### Widgets

```c
// Button
BabaWidget* baba_button_create(const char* text);
void baba_button_set_text(BabaWidget* button, const char* text);
void baba_button_set_on_click(BabaWidget* button, void (*cb)(BabaWidget*, void*), void* data);

// Label
BabaWidget* baba_label_create(const char* text);
void baba_label_set_text(BabaWidget* label, const char* text);
void baba_label_set_font_size(BabaWidget* label, float size);
void baba_label_set_color(BabaWidget* label, BabaColor color);

// TextBox
BabaWidget* baba_textbox_create(const char* placeholder);
void baba_textbox_set_text(BabaWidget* textbox, const char* text);
const char* baba_textbox_get_text(BabaWidget* textbox);
```

### Layout

```c
BabaWidget* baba_flex_layout_create(BabaLayoutDirection direction);
void baba_flex_layout_set_gap(BabaWidget* layout, float gap);

BabaWidget* baba_grid_layout_create(int columns, int rows);
```

---

## Best Practices

### Memory Management

```c
// Correct: Destroy in reverse order
BabaApp* app = baba_app_create();
BabaWindow* win = baba_window_create(app, ...);
baba_window_destroy(win);
baba_app_destroy(app);
```

### Performance

```c
// Only update when needed
if (data_changed) {
    baba_widget_invalidate(widget);
}
```

---

## FAQ

### Q: vulkan.h not found?

```bash
export VULKAN_SDK=/path/to/vulkansdk
export CMAKE_PREFIX_PATH=$VULKAN_SDK
```

### Q: Window shows but no content?

1. Check if `baba_window_show()` was called
2. Check if `baba_app_run()` was called
3. Check if widgets were added to root