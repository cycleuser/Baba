# Baba C Development Guide / 把拔 C开发指南

[English](#english) | [中文](#中文)

---

<a name="english"></a>
# English

## Installing Baba for C Development

After installing Baba via pip, you can use it for C GUI development:

```bash
pip install baba-gui
```

## Locating Development Files

Use Python to find the installed files:

```bash
# Get include directory
python -c "import baba; print(baba.get_include_dir())"

# Get library directory
python -c "import baba; print(baba.get_lib_dir())"

# Get library path for current platform
python -c "import baba; print(baba.get_lib_path())"
```

## Compiling C Programs

### macOS

```bash
# Get paths
INCLUDE_DIR=$(python -c "import baba; print(baba.get_include_dir())")
LIB_PATH=$(python -c "import baba; print(baba.get_lib_path())")

# Compile
clang main.c -I"$INCLUDE_DIR" -L"$(dirname $LIB_PATH)" -lbaba \
    -framework Cocoa -framework Metal -framework QuartzCore \
    -lvulkan -o myapp
```

### Linux

```bash
# Get paths
INCLUDE_DIR=$(python -c "import baba; print(baba.get_include_dir())")
LIB_PATH=$(python -c "import baba; print(baba.get_lib_path())")

# Compile (X11)
gcc main.c -I"$INCLUDE_DIR" -L"$(dirname $LIB_PATH)" -lbaba \
    -lvulkan -lX11 -lXrandr -lXinerama -lXcursor -lXi \
    -o myapp

# Compile (Wayland)
gcc main.c -I"$INCLUDE_DIR" -L"$(dirname $LIB_PATH)" -lbaba \
    -lvulkan -lwayland-client -lxkbcommon \
    -o myapp
```

### Windows (MinGW)

```powershell
# Install dependencies
winget install MSYS2.MSYS2
winget install KhronosGroup.VulkanSDK

# Restart terminal, then in MSYS2 MINGW64:
pacman -S mingw-w64-x86_64-gcc

# Get paths
$INCLUDE_DIR = python -c "import baba; print(baba.get_include_dir())"
$LIB_DIR = python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))"

# Compile
gcc main.c -I"$INCLUDE_DIR" -I"C:\VulkanSDK\1.3.xxx.0\Include" `
    -L"$LIB_DIR" -L"C:\VulkanSDK\1.3.xxx.0\Lib" `
    -lbaba_windows -lvulkan-1 -luser32 -lgdi32 -o myapp.exe
```

**Note**: The library name is `baba_windows` on Windows. Replace `1.3.xxx.0` with your Vulkan SDK version.

## Example Projects

Complete working examples are available in the `examples/` directory:

- **examples/windows/** - Windows C example with build script
- **examples/demo.c** - Basic demo
- **examples/calculator.c** - Calculator application

To build the Windows example:
```powershell
cd examples\windows
.\build.bat
```

## Example Makefile

```makefile
# Baba GUI Makefile
PYTHON ?= python3

INCLUDE_DIR := $(shell $(PYTHON) -c "import baba; print(baba.get_include_dir())")
LIB_DIR := $(shell $(PYTHON) -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))")

CC := gcc
CFLAGS := -I$(INCLUDE_DIR) -Wall -Wextra -O2
LDFLAGS := -L$(LIB_DIR) -lbaba -lvulkan

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LDFLAGS += -framework Cocoa -framework Metal -framework QuartzCore
endif
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -lX11 -lXrandr -lXinerama -lXcursor -lXi
endif

SRCS := main.c
OBJS := $(SRCS:.c=.o)
TARGET := myapp

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
```

## Example CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyApp LANGUAGES C)

# Find Baba
find_package(Python3 REQUIRED COMPONENTS Interpreter)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import baba; print(baba.get_include_dir())"
    OUTPUT_VARIABLE BABA_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))"
    OUTPUT_VARIABLE BABA_LIB_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "Baba include: ${BABA_INCLUDE_DIR}")
message(STATUS "Baba lib: ${BABA_LIB_DIR}")

# Find Vulkan
find_package(Vulkan REQUIRED)

add_executable(myapp main.c)

target_include_directories(myapp PRIVATE ${BABA_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS})
target_link_libraries(myapp PRIVATE ${BABA_LIB_DIR}/libbaba.a ${Vulkan_LIBRARIES})

# Platform-specific
if(APPLE)
    target_link_libraries(myapp PRIVATE "-framework Cocoa" "-framework Metal" "-framework QuartzCore")
elseif(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(myapp PRIVATE ${X11_LIBRARIES})
elseif(WIN32)
    target_link_libraries(myapp PRIVATE user32 gdi32)
endif()
```

---

<a name="中文"></a>
# 中文

## 安装Baba用于C开发

通过pip安装Baba后，可以用于C GUI开发：

```bash
pip install baba-gui
```

## 查找开发文件

使用Python查找已安装的文件：

```bash
# 获取头文件目录
python -c "import baba; print(baba.get_include_dir())"

# 获取库文件目录
python -c "import baba; print(baba.get_lib_dir())"

# 获取当前平台的库文件路径
python -c "import baba; print(baba.get_lib_path())"
```

## 编译C程序

### macOS

```bash
# 获取路径
INCLUDE_DIR=$(python -c "import baba; print(baba.get_include_dir())")
LIB_PATH=$(python -c "import baba; print(baba.get_lib_path())")

# 编译
clang main.c -I"$INCLUDE_DIR" -L"$(dirname $LIB_PATH)" -lbaba \
    -framework Cocoa -framework Metal -framework QuartzCore \
    -lvulkan -o myapp
```

### Linux

```bash
# 获取路径
INCLUDE_DIR=$(python -c "import baba; print(baba.get_include_dir())")
LIB_PATH=$(python -c "import baba; print(baba.get_lib_path())")

# 编译 (X11)
gcc main.c -I"$INCLUDE_DIR" -L"$(dirname $LIB_PATH)" -lbaba \
    -lvulkan -lX11 -lXrandr -lXinerama -lXcursor -lXi \
    -o myapp

# 编译 (Wayland)
gcc main.c -I"$INCLUDE_DIR" -L"$(dirname $LIB_PATH)" -lbaba \
    -lvulkan -lwayland-client -lxkbcommon \
    -o myapp
```

### Windows (MinGW)

```powershell
# 获取路径
$INCLUDE_DIR = python -c "import baba; print(baba.get_include_dir())"
$LIB_DIR = python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))"

# 编译
gcc main.c -I"$INCLUDE_DIR" -L"$LIB_DIR" -lbaba `
    -lvulkan -luser32 -lgdi32 -o myapp.exe
```

## 示例Makefile

```makefile
# Baba GUI Makefile
PYTHON ?= python3

INCLUDE_DIR := $(shell $(PYTHON) -c "import baba; print(baba.get_include_dir())")
LIB_DIR := $(shell $(PYTHON) -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))")

CC := gcc
CFLAGS := -I$(INCLUDE_DIR) -Wall -Wextra -O2
LDFLAGS := -L$(LIB_DIR) -lbaba -lvulkan

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LDFLAGS += -framework Cocoa -framework Metal -framework QuartzCore
endif
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -lX11 -lXrandr -lXinerama -lXcursor -lXi
endif

SRCS := main.c
OBJS := $(SRCS:.c=.o)
TARGET := myapp

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
```

## 示例CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyApp LANGUAGES C)

# 查找Baba
find_package(Python3 REQUIRED COMPONENTS Interpreter)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import baba; print(baba.get_include_dir())"
    OUTPUT_VARIABLE BABA_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))"
    OUTPUT_VARIABLE BABA_LIB_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "Baba include: ${BABA_INCLUDE_DIR}")
message(STATUS "Baba lib: ${BABA_LIB_DIR}")

# 查找Vulkan
find_package(Vulkan REQUIRED)

add_executable(myapp main.c)

target_include_directories(myapp PRIVATE ${BABA_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS})
target_link_libraries(myapp PRIVATE ${BABA_LIB_DIR}/libbaba.a ${Vulkan_LIBRARIES})

# 平台特定
if(APPLE)
    target_link_libraries(myapp PRIVATE "-framework Cocoa" "-framework Metal" "-framework QuartzCore")
elseif(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(myapp PRIVATE ${X11_LIBRARIES})
elseif(WIN32)
    target_link_libraries(myapp PRIVATE user32 gdi32)
endif()
```