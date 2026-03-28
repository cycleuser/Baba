# Baba GUI - Windows C开发示例

本示例演示如何在Windows上使用C语言开发Baba GUI应用（通过pip安装后）。

## 前置条件

1. **安装 Baba GUI**:
   ```powershell
   pip install baba-gui
   ```

2. **安装 MinGW-w64** (GCC编译器):
   ```powershell
   winget install MSYS2.MSYS2
   # 或使用 scoop: scoop install mingw
   ```

3. **安装 Vulkan SDK**:
   ```powershell
   winget install KhronosGroup.VulkanSDK
   ```
   安装后重启终端。

## 文件说明

```
examples/windows/
├── main.c       # C源代码
├── build.bat    # 构建脚本
└── README.md    # 本文件
```

## 快速开始

```powershell
# 进入示例目录
cd examples\windows

# 运行构建脚本
.\build.bat

# 运行程序
.\app.exe
```

## 手动构建

```powershell
# 获取路径
$INCLUDE = python -c "import baba; print(baba.get_include_dir())"
$LIB = python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))"

# 编译
gcc main.c -I"$INCLUDE" -L"$LIB" -lbaba_windows -lvulkan -luser32 -lgdi32 -o app.exe
```

## 在自己的项目中使用

1. **包含头文件**:
   ```c
   #include <baba.h>
   #include <widgets/button.h>
   #include <widgets/label.h>
   ```

2. **链接库**:
   ```
   -lbaba_windows    # Baba GUI库
   -lvulkan          # Vulkan库
   -luser32          # Windows用户界面
   -lgdi32           # Windows GDI
   ```

3. **示例Makefile**:
   ```makefile
   # 从Python获取路径
   INCLUDE_PATH := $(shell python -c "import baba; print(baba.get_include_dir())")
   LIB_PATH := $(shell python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))")
   
   CC = gcc
   CFLAGS = -I"$(INCLUDE_PATH)"
   LDFLAGS = -L"$(LIB_PATH)" -lbaba_windows -lvulkan -luser32 -lgdi32
   
   app: main.c
       $(CC) $(CFLAGS) $< $(LDFLAGS) -o $@
   ```

## 故障排除

### "vulkan.h not found"
- 安装 Vulkan SDK: `winget install KhronosGroup.VulkanSDK`
- 安装后重启终端
- 验证安装: `echo $env:VULKAN_SDK`

### "libbaba_windows.a not found"
- 重新安装: `pip install baba-gui --force-reinstall`
- 检查库文件: `python -c "import baba; print(baba.get_lib_path())"`

### "undefined reference to `baba_app_create`"
- 确保链接了 `-lbaba_windows`
- 检查库路径是否正确

### 窗口不显示
- 确保在 `baba_app_run()` 之前调用了 `baba_window_show()`
- 检查显卡是否安装了Vulkan驱动