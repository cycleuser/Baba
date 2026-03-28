# 把拔

一个轻量级、跨平台的C语言GUI框架，使用Vulkan进行渲染。

## 特性

- **最小依赖**: 仅需Vulkan SDK
- **跨平台**: macOS、Linux (X11/Wayland)、Windows
- **原生窗口**: 使用平台原生API
- **Vulkan渲染**: 现代GPU加速渲染
- **完整控件**: 按钮、标签、文本框等
- **灵活布局**: Flex/Grid布局系统
- **Python绑定**: `pip install baba-gui`

## 文档

- [Windows构建指南](docs/WINDOWS_BUILD_CN.md) - MSYS2/Scoop/Zig三种轻量方案
- [Linux构建指南](docs/LINUX_BUILD_CN.md) - Ubuntu/Arch/Fedora安装说明
- [macOS构建指南](docs/MACOS_BUILD_CN.md) - Homebrew/Vulkan SDK安装
- [架构说明](docs/ARCHITECTURE_CN.md) - 项目架构与设计
- [使用教程](docs/TUTORIAL_CN.md) - 快速入门指南
- [C开发指南](docs/C_DEVELOPMENT.md) - 使用Baba进行C GUI开发

[English](README.md)

## 安装

### Python (推荐)

```bash
pip install baba-gui
```

### 从源码编译

**前置条件:**
- CMake 3.20+
- C11编译器
- Vulkan SDK

**构建步骤:**
```bash
mkdir build && cd build
cmake ..
make
```

**各平台说明:**

**macOS:**
```bash
brew install vulkan-loader vulkan-headers molten-vk
cmake -DCMAKE_PREFIX_PATH=$(brew --prefix) ..
make
```

**Linux:**
```bash
# Ubuntu/Debian
sudo apt install libvulkan-dev xorg-dev wayland-protocols

# Arch
sudo pacman -S vulkan-headers vulkan-tools libx11 libwayland

cmake ..
make
```

**Windows:**

推荐使用MSYS2 + MinGW-w64：

```powershell
# 通过winget安装依赖
winget install MSYS2.MSYS2
winget install KhronosGroup.VulkanSDK

# 打开MSYS2 MINGW64终端：
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja

# 构建
cmake -B build -G Ninja
cmake --build build
```

如果一定要用Visual Studio：
```powershell
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

## 快速开始

### Python

```python
import baba

def on_click(button):
    print("按钮被点击了！")

with baba.create_app() as app:
    window = app.create_window("把拔 Demo", 800, 600)
    
    button = baba.Button("点击我")
    button.on_click(on_click)
    window.root.add_child(button)
    
    label = baba.Label("你好，把拔GUI！")
    label.font_size = 24.0
    label.color = baba.Color.from_hex("#333333")
    window.root.add_child(label)
    
    window.show()
    app.run()
```

### C

```c
#include <baba.h>
#include <widgets/button.h>
#include <widgets/label.h>

void on_click(BabaWidget* button, void* data) {
    printf("按钮被点击了！\n");
}

int main() {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "把拔 Demo", 800, 600);
    
    BabaWidget* root = baba_window_get_root(win);
    BabaWidget* button = baba_button_create("点击我");
    baba_button_set_on_click(button, on_click, NULL);
    baba_widget_add_child(root, button);
    
    baba_window_show(win);
    baba_app_run(app);
    
    baba_window_destroy(win);
    baba_app_destroy(app);
    return 0;
}
```

## 架构

```
Baba/
├── src/
│   ├── baba.h              # 主公共API
│   ├── core/               # 核心工具
│   ├── platform/           # 平台抽象 (macOS/Linux/Windows)
│   ├── render/             # Vulkan渲染器 + 绑定器API
│   ├── widgets/            # UI控件 (按钮, 标签, 文本框)
│   ├── layout/             # Flex/Grid布局
│   └── theme/              # 主题系统
├── python/                 # Python绑定 (cffi)
├── examples/               # 示例程序
└── docs/                   # 文档
```

## API概览

### 应用

```c
// C
BabaApp* app = baba_app_create();
int result = baba_app_run(app);
void baba_app_quit(BabaApp* app);
```

```python
# Python
app = baba.App()
result = app.run()
app.quit()
```

### 窗口

```c
BabaWindow* win = baba_window_create(app, "标题", 800, 600);
baba_window_set_title(win, "新标题");
baba_window_show(win);
```

```python
window = app.create_window("标题", 800, 600)
window.title = "新标题"
window.show()
```

### 控件

```c
// 按钮
BabaWidget* btn = baba_button_create("按钮");
baba_button_set_on_click(btn, callback, NULL);

// 标签
BabaWidget* label = baba_label_create("文本");
baba_label_set_font_size(label, 16.0f);

// 文本框
BabaWidget* input = baba_textbox_create("占位符");
const char* text = baba_textbox_get_text(input);
```

```python
# 按钮
btn = baba.Button("按钮")
btn.on_click(lambda b: print("点击"))

# 标签
label = baba.Label("文本")
label.font_size = 16.0

# 文本框
input = baba.TextBox("占位符")
text = input.text
```

### 布局

```c
// 弹性布局
BabaWidget* flex = baba_flex_layout_create(BABA_LAYOUT_DIRECTION_ROW);
baba_flex_layout_set_gap(flex, 10.0f);

// 网格布局
BabaWidget* grid = baba_grid_layout_create(3, 2);
```

## 发布到PyPI

### 构建Wheel

```bash
# 安装构建工具
pip install build cibuildwheel

# 构建源码分发包
python -m build --sdist

# 为所有平台构建wheel
cibuildwheel --platform macos
cibuildwheel --platform linux
cibuildwheel --platform windows
```

### 上传到PyPI

```bash
pip install twine
twine upload dist/*
```

### GitHub Actions (CI/CD)

项目在`pyproject.toml`中包含`cibuildwheel`配置，用于自动构建wheel：

```yaml
# .github/workflows/publish.yml
name: Publish to PyPI
on:
  release:
    types: [published]
jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [macos-latest, ubuntu-latest, windows-latest]
    steps:
      - uses: actions/checkout@v4
      - uses: actions/setup-python@v5
      - run: pip install cibuildwheel
      - run: cibuildwheel --output-dir dist
      - uses: actions/upload-artifact@v4
        with:
          name: wheels-${{ matrix.os }}
          path: dist
  publish:
    needs: build
    runs-on: ubuntu-latest
    steps:
      - uses: actions/download-artifact@v4
      - run: pip install twine
      - run: twine upload wheels-*/*
        env:
          TWINE_USERNAME: __token__
          TWINE_PASSWORD: ${{ secrets.PYPI_TOKEN }}
```

## 依赖

| 平台 | 依赖 |
|------|------|
| 渲染 | Vulkan SDK |
| macOS | Cocoa, Metal, QuartzCore |
| Linux | X11 或 Wayland |
| Windows | Win32 API (user32, gdi32) |
| Python | cffi >= 1.16.0 |

## 平台支持

| 平台 | 窗口系统 | 状态 |
|------|----------|------|
| macOS | Cocoa + Metal | ✅ 完整支持 |
| Linux | X11 | ✅ 完整支持 |
| Linux | Wayland | ✅ 完整支持 |
| Windows | Win32 | ✅ 完整支持 |

## C开发

`pip install baba-gui` 后，可用于C GUI开发：

```bash
# 获取开发路径
python -c "import baba; print(baba.get_include_dir())"  # 头文件
python -c "import baba; print(baba.get_lib_path())"     # 库文件
```

```bash
# 编译C程序 (macOS示例)
INCLUDE=$(python -c "import baba; print(baba.get_include_dir())")
LIB=$(python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))")

clang main.c -I"$INCLUDE" -L"$LIB" -lbaba \
    -framework Cocoa -framework Metal -lvulkan -o myapp
```

详见 [C开发指南](docs/C_DEVELOPMENT.md)。

## 构建与发布

### 构建原生库（当前平台）

```bash
# Unix (macOS/Linux)
./build_all.sh

# Windows (MSYS2 MinGW)
./build_all.sh
```

这只会构建**当前平台**的库。

### 跨平台构建

对于生产发布，使用 **GitHub Actions** 在所有平台上构建：

1. 推送到 `main` 分支或创建发布
2. GitHub Actions 在 macOS、Linux、Windows 上分别构建
3. 构建产物合并后发布到 PyPI

触发 PyPI 发布：
1. 进入 GitHub → Releases → Draft a new release
2. 创建标签（如 `v0.1.2`）
3. 发布 - GitHub Actions 会自动构建并上传到 PyPI

### 手动多平台构建

如果需要本地构建所有平台：

1. **macOS**: 在 macOS 机器上运行
2. **Linux**: 在 Linux 机器上运行或使用 Docker
3. **Windows**: 在 Windows 上使用 MSYS2 MinGW 运行

然后手动复制库文件到 `python/baba/lib/`：
```
python/baba/lib/
├── libbaba_macos.a
├── libbaba_linux.a
└── libbaba_windows.a
```

### 构建并上传到PyPI（手动）

```bash
# 构建原生库
./build_all.sh

# 构建Python包
pip install build twine
python -m build

# 上传到PyPI
twine upload dist/*
```

## 许可证

MIT License - 详见LICENSE文件