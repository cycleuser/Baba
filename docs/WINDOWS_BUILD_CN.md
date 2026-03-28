# Windows构建指南

在Windows上构建Baba GUI应用，无需安装庞大的Visual Studio (~30GB)。

## 快速开始

```powershell
# 通过winget安装依赖
winget install MSYS2.MSYS2
winget install KhronosGroup.VulkanSDK
winget install Kitware.CMake

# 重启终端，然后在MSYS2 MINGW64终端中运行：
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-ninja

# 构建
cmake -B build -G Ninja
cmake --build build
```

## 工具链对比

| 工具链 | 安装大小 | 编译器 | 速度 | 推荐度 |
|--------|----------|--------|------|--------|
| MSYS2 + MinGW | ~500MB | GCC | 快 | ⭐⭐⭐⭐⭐ |
| Winget + MinGW | ~500MB | GCC | 快 | ⭐⭐⭐⭐⭐ |
| Visual Studio | ~30GB | MSVC | 慢 | ⭐ |

## 方案1: MSYS2 + MinGW-w64 (推荐)

MSYS2提供类Unix环境和pacman包管理器。

### 安装步骤

1. 安装MSYS2:
   ```powershell
   winget install MSYS2.MSYS2
   ```

2. 从开始菜单打开 **MSYS2 MINGW64** 终端

3. 安装Vulkan SDK:
   ```powershell
   winget install KhronosGroup.VulkanSDK
   ```
   安装后重启终端。

### 配置清华镜像 (国内推荐)

创建或编辑 `C:\msys64\etc\pacman.d\mirrorlist.mingw64`：

```
Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/mingw/x86_64/
Server = https://mirror.msys2.org/mingw/x86_64/
```

创建或编辑 `C:\msys64\etc\pacman.d\mirrorlist.msys`：

```
Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/msys/$arch/
Server = https://mirror.msys2.org/msys/$arch/
```

然后刷新：

```bash
pacman -Sy
```

### 安装依赖

```bash
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-ninja
```

Vulkan SDK 通过 winget 安装（见上文）。

### 构建项目

```bash
cd /c/Users/你的用户名/path/to/Baba
cmake -B build -G Ninja
cmake --build build
```

### 运行测试

```bash
./build/calculator_standalone.exe
```

## 方案2: Winget + MinGW (简单)

仅使用winget的简单安装方式。

### 安装依赖

```powershell
# 安装MinGW (通过MSYS2)
winget install MSYS2.MSYS2

# 安装Vulkan SDK
winget install KhronosGroup.VulkanSDK

# 安装CMake (可选，如果不使用MSYS2的cmake)
winget install Kitware.CMake
```

安装后重启终端。

### 构建项目

打开 **MSYS2 MINGW64** 终端：

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
cd /c/Users/你的用户名/path/to/Baba
cmake -B build -G Ninja
cmake --build build
```

## 方案3: Visual Studio (不推荐)

仅当你已安装Visual Studio时使用。

### 安装依赖

```powershell
# 先安装Visual Studio，然后：
winget install KhronosGroup.VulkanSDK
```

### 构建项目

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## 构建产物

构建成功后：

| 文件 | 说明 |
|------|------|
| `build/libbaba.a` | 静态库 |
| `build/calculator_standalone.exe` | 演示程序 |
| `build/examples/calculator.exe` | 框架演示 |

## 性能对比

| 工具链 | 编译时间 | 二进制大小 |
|--------|----------|------------|
| MinGW GCC | ~3秒 | 84KB |
| Clang | ~2.5秒 | 82KB |
| MSVC | ~5秒 | 120KB |

## 常见问题

### 找不到Vulkan SDK

如果CMake找不到Vulkan SDK：

```powershell
# 检查Vulkan SDK安装
echo $env:VULKAN_SDK

# 如果为空，手动查找并设置
dir C:\VulkanSDK\
$env:VULKAN_SDK = "C:\VulkanSDK\1.3.xxx.0"
```

### "vulkan/vulkan.h not found"

1. 安装Vulkan SDK:
   ```powershell
   winget install KhronosGroup.VulkanSDK
   ```

2. 重启终端

3. 验证安装:
   ```powershell
   echo $env:VULKAN_SDK
   dir C:\VulkanSDK\
   ```

### 缺少DLL

如果出现"vulkan-1.dll not found"：

```powershell
# 添加Vulkan到PATH (MSYS2)
export PATH="/mingw64/bin:$PATH"

# 或复制DLL
cp $env:VULKAN_SDK\bin\vulkan-1.dll .
```

### 找不到GCC

确保打开的是 **MSYS2 MINGW64** 终端（不是MSYS2 MSYS终端）。

或将MinGW添加到PATH：
```powershell
$env:PATH = "C:\msys64\mingw64\bin;" + $env:PATH
```