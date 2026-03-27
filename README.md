# Baba

A lightweight, cross-platform C GUI framework using Vulkan for rendering.

## Features

- **Minimal Dependencies**: Only requires Vulkan SDK
- **Cross-Platform**: macOS, Linux (X11/Wayland), Windows
- **Native Window**: Uses platform-native APIs
- **Vulkan Rendering**: Modern GPU-accelerated rendering
- **Complete Widgets**: Buttons, Labels, TextBoxes, etc.
- **Flexible Layout**: Flexbox-like and Grid layouts
- **Python Bindings**: `pip install baba-gui`

## Documentation

- [Windows Build Guide](docs/WINDOWS_BUILD.md) - MSYS2/Scoop/Zig lightweight options
- [Linux Build Guide](docs/LINUX_BUILD.md) - Ubuntu/Arch/Fedora instructions
- [macOS Build Guide](docs/MACOS_BUILD.md) - Homebrew/Vulkan SDK installation
- [Architecture](docs/ARCHITECTURE.md) - Project architecture and design
- [Tutorial](docs/TUTORIAL.md) - Quick start guide
- [C Development Guide](docs/C_DEVELOPMENT.md) - Using Baba for C GUI development

[中文文档](README_CN.md)

## Installation

### Python (Recommended)

```bash
pip install baba-gui
```

### From Source

**Prerequisites:**
- CMake 3.20+
- C11 compiler
- Vulkan SDK

**Build Steps:**
```bash
mkdir build && cd build
cmake ..
make
```

**Platform-Specific:**

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

Recommended: MinGW-w64 or Clang (no Visual Studio required):

```powershell
# Option 1: MSYS2 + MinGW-w64 (Recommended)
# 1. Install MSYS2: https://www.msys2.org/
# 2. In MSYS2 MINGW64 terminal:
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-vulkan

cmake -B build -G Ninja
cmake --build build

# Option 2: Scoop + MinGW
scoop install mingw cmake ninja
# Install Vulkan SDK: https://vulkan.lunarg.com/
cmake -B build -G Ninja -DVULKAN_SDK="C:/VulkanSDK/1.x.x.x"
cmake --build build

# Option 3: Zig as C compiler (Lightest)
scoop install zig cmake ninja
cmake -B build -G Ninja -DCMAKE_C_COMPILER=zig cc
cmake --build build
```

If you must use Visual Studio:
```powershell
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

## Quick Start

### Python

```python
import baba

def on_click(button):
    print("Button clicked!")

with baba.create_app() as app:
    window = app.create_window("Baba Demo", 800, 600)
    
    button = baba.Button("Click Me")
    button.on_click(on_click)
    window.root.add_child(button)
    
    label = baba.Label("Hello, Baba GUI!")
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
    printf("Button clicked!\n");
}

int main() {
    BabaApp* app = baba_app_create();
    BabaWindow* win = baba_window_create(app, "Baba Demo", 800, 600);
    
    BabaWidget* root = baba_window_get_root(win);
    BabaWidget* button = baba_button_create("Click Me");
    baba_button_set_on_click(button, on_click, NULL);
    baba_widget_add_child(root, button);
    
    baba_window_show(win);
    baba_app_run(app);
    
    baba_window_destroy(win);
    baba_app_destroy(app);
    return 0;
}
```

## Architecture

```
Baba/
├── src/
│   ├── baba.h              # Main public API
│   ├── core/               # Core utilities
│   ├── platform/           # Platform abstraction (macOS/Linux/Windows)
│   ├── render/             # Vulkan renderer + Painter API
│   ├── widgets/            # UI widgets (Button, Label, TextBox)
│   ├── layout/             # Flex/Grid layouts
│   └── theme/              # Theming system
├── python/                 # Python bindings (cffi)
├── examples/               # Example applications
└── docs/                   # Documentation
```

## API Overview

### Application

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

### Window

```c
BabaWindow* win = baba_window_create(app, "Title", 800, 600);
baba_window_set_title(win, "New Title");
baba_window_show(win);
```

```python
window = app.create_window("Title", 800, 600)
window.title = "New Title"
window.show()
```

### Widgets

```c
// Button
BabaWidget* btn = baba_button_create("Button");
baba_button_set_on_click(btn, callback, NULL);

// Label
BabaWidget* label = baba_label_create("Text");
baba_label_set_font_size(label, 16.0f);

// TextBox
BabaWidget* input = baba_textbox_create("Placeholder");
const char* text = baba_textbox_get_text(input);
```

```python
# Button
btn = baba.Button("Button")
btn.on_click(lambda b: print("clicked"))

# Label
label = baba.Label("Text")
label.font_size = 16.0

# TextBox
input = baba.TextBox("Placeholder")
text = input.text
```

### Layout

```c
// Flex layout
BabaWidget* flex = baba_flex_layout_create(BABA_LAYOUT_DIRECTION_ROW);
baba_flex_layout_set_gap(flex, 10.0f);

// Grid layout
BabaWidget* grid = baba_grid_layout_create(3, 2);
```

## Publishing to PyPI

### Build Wheels

```bash
# Install build tools
pip install build cibuildwheel

# Build source distribution
python -m build --sdist

# Build wheels for all platforms
cibuildwheel --platform macos
cibuildwheel --platform linux
cibuildwheel --platform windows
```

### Upload to PyPI

```bash
pip install twine
twine upload dist/*
```

### GitHub Actions (CI/CD)

The project includes `cibuildwheel` configuration in `pyproject.toml` for automated wheel building:

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

## Dependencies

| Platform | Dependencies |
|----------|--------------|
| Rendering | Vulkan SDK |
| macOS | Cocoa, Metal, QuartzCore |
| Linux | X11 or Wayland |
| Windows | Win32 API (user32, gdi32) |
| Python | cffi >= 1.16.0 |

## Platform Support

| Platform | Window System | Status |
|----------|---------------|--------|
| macOS | Cocoa + Metal | ✅ Full Support |
| Linux | X11 | ✅ Full Support |
| Linux | Wayland | ✅ Full Support |
| Windows | Win32 | ✅ Full Support |

## C Development

After `pip install baba-gui`, you can use Baba for C GUI development:

```bash
# Get development paths
python -c "import baba; print(baba.get_include_dir())"  # Headers
python -c "import baba; print(baba.get_lib_path())"     # Library
```

```bash
# Compile your C app (macOS example)
INCLUDE=$(python -c "import baba; print(baba.get_include_dir())")
LIB=$(python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))")

clang main.c -I"$INCLUDE" -L"$LIB" -lbaba \
    -framework Cocoa -framework Metal -lvulkan -o myapp
```

See [C Development Guide](docs/C_DEVELOPMENT.md) for details.

## Building & Publishing

### Build Native Library

```bash
# Unix (macOS/Linux)
./build_native.sh

# Windows
build_native.bat
```

### Build & Upload to PyPI

```bash
# Unix (macOS/Linux)
./upload_pypi.sh

# Windows
upload_pypi.bat

# Or use Python script
python build_pypi.py --upload
```

## License

MIT License - see LICENSE file