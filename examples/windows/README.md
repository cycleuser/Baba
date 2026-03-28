# Baba GUI - Windows C Example

This example demonstrates how to use Baba GUI from C on Windows after installing via pip.

## Prerequisites

1. **Install Baba GUI**:
   ```powershell
   pip install baba-gui
   ```

2. **Install MinGW-w64** (for GCC compiler):
   ```powershell
   scoop install mingw
   ```

3. **Install Vulkan SDK**:
   - Download from: https://vulkan.lunarg.com/
   - Or via scoop: `scoop install vulkan-sdk`

## Files

```
examples/windows/
├── main.c       # Example C source code
├── build.bat    # Build script
└── README.md    # This file
```

## Quick Start

```powershell
# Navigate to the example directory
cd examples\windows

# Run the build script
.\build.bat

# Run the application
.\app.exe
```

## Manual Build

```powershell
# Get paths
$INCLUDE = python -c "import baba; print(baba.get_include_dir())"
$LIB = python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))"

# Compile
gcc main.c -I"$INCLUDE" -L"$LIB" -lbaba_windows -lvulkan -luser32 -lgdi32 -o app.exe
```

## Using in Your Own Project

1. **Include headers**:
   ```c
   #include <baba.h>
   #include <widgets/button.h>
   #include <widgets/label.h>
   ```

2. **Link libraries**:
   ```
   -lbaba_windows    # Baba GUI library
   -lvulkan          # Vulkan library
   -luser32          # Windows user interface
   -lgdi32           # Windows GDI
   ```

3. **Example Makefile**:
   ```makefile
   # Get paths from Python
   INCLUDE_PATH := $(shell python -c "import baba; print(baba.get_include_dir())")
   LIB_PATH := $(shell python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))")
   
   CC = gcc
   CFLAGS = -I"$(INCLUDE_PATH)"
   LDFLAGS = -L"$(LIB_PATH)" -lbaba_windows -lvulkan -luser32 -lgdi32
   
   app: main.c
       $(CC) $(CFLAGS) $< $(LDFLAGS) -o $@
   ```

## Troubleshooting

### "vulkan.h not found"
- Install Vulkan SDK from https://vulkan.lunarg.com/
- Make sure VULKAN_SDK environment variable is set

### "libbaba_windows.a not found"
- Reinstall baba-gui: `pip install baba-gui --force-reinstall`
- Check if library exists: `python -c "import baba; print(baba.get_lib_path())"`

### "undefined reference to `baba_app_create`"
- Make sure to link `-lbaba_windows`
- Check library path is correct

### Window doesn't appear
- Make sure to call `baba_window_show()` before `baba_app_run()`
- Check Vulkan drivers are installed for your GPU