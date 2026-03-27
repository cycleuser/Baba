# Windows构建指南

在Windows上构建Baba GUI应用，无需安装庞大的Visual Studio (~30GB)。

## 工具链对比

| 工具链 | 安装大小 | 编译器 | 速度 | 推荐度 |
|--------|----------|--------|------|--------|
| MSYS2 + MinGW | ~500MB | GCC | 快 | ⭐⭐⭐⭐⭐ |
| Scoop + MinGW | ~200MB | GCC | 快 | ⭐⭐⭐⭐ |
| Scoop + Zig | ~50MB | Zig cc | 快 | ⭐⭐⭐⭐⭐ |
| Visual Studio | ~30GB | MSVC | 慢 | ⭐ |

## 方案1: MSYS2 + MinGW-w64 (国内推荐)

MSYS2提供类Unix环境和pacman包管理器。

### 安装步骤

1. 下载MSYS2: https://www.msys2.org/
2. 安装到 `C:\msys64` (默认路径)
3. 从开始菜单打开 **MSYS2 MINGW64** 终端

### 配置清华镜像 (推荐)

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
          mingw-w64-x86_64-ninja mingw-w64-x86_64-vulkan \
          mingw-w64-x86_64-python mingw-w64-x86_64-python-pip
```

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

## 方案2: Scoop + MinGW

Scoop是Windows的命令行包管理器，类似macOS的Homebrew。

### 通过Winget安装Scoop

```powershell
winget install scoop
```

或手动安装：

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression
```

### 配置清华镜像 (推荐)

```powershell
# 设置Scoop本身的清华镜像
scoop config SCOOP_REPO "https://mirrors.tuna.tsinghua.edu.cn/git/scoop-installer/scoop"
scoop config SCOOP_BRANCH "master"

# 更新Scoop
scoop update

# 添加清华extras bucket
scoop bucket add extras https://mirrors.tuna.tsinghua.edu.cn/git/scoop-installer/scoop-extras

# 添加versions bucket (用于Vulkan SDK)
scoop bucket add versions https://mirrors.tuna.tsinghua.edu.cn/git/scoop-installer/versions
```

### 安装依赖

```powershell
scoop install mingw cmake ninja vulkan-sdk
```

### 构建项目

```powershell
cmake -B build -G Ninja
cmake --build build
```

### 故障排除

如果CMake找不到Vulkan SDK：

```powershell
$env:VULKAN_SDK = "C:\Users\$env:USERNAME\scoop\apps\vulkan-sdk\current"
cmake -B build -G Ninja
cmake --build build
```

## 方案3: Scoop + Zig (最轻量 - 50MB)

Zig通过`zig cc`命令提供C编译器。

### 安装Scoop并配置镜像

见上方方案2。

### 安装依赖

```powershell
scoop install zig cmake ninja vulkan-sdk
```

### 构建项目

```powershell
cmake -B build -G Ninja -DCMAKE_C_COMPILER="zig cc"
cmake --build build
```

## 方案4: Visual Studio (不推荐)

仅当你已安装Visual Studio时使用。

### 安装依赖

1. 安装Visual Studio 2022，选择"使用C++的桌面开发"工作负载
2. 下载Vulkan SDK: https://vulkan.lunarg.com/

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
| Zig cc | ~3秒 | 84KB |
| MSVC | ~5秒 | 120KB |

## 常见问题

### 找不到Vulkan SDK

```powershell
# 设置环境变量
$env:VULKAN_SDK = "C:\VulkanSDK\1.3.xxx.0"
# 或Scoop安装的
$env:VULKAN_SDK = "C:\Users\$env:USERNAME\scoop\apps\vulkan-sdk\current"
```

### 缺少DLL

如果出现"vulkan-1.dll not found"：

```powershell
# 添加Vulkan到PATH (MSYS2)
export PATH="/mingw64/bin:$PATH"

# 或复制DLL (Scoop)
cp $env:VULKAN_SDK\bin\vulkan-1.dll .
```