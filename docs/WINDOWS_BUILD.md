# Windows Build Guide

Build Baba GUI applications on Windows without the heavy Visual Studio (~30GB).

## Toolchain Comparison

| Toolchain | Install Size | Compiler | Speed | Recommended |
|-----------|--------------|----------|-------|-------------|
| MSYS2 + MinGW | ~500MB | GCC | Fast | ⭐⭐⭐⭐⭐ |
| Scoop + MinGW | ~200MB | GCC | Fast | ⭐⭐⭐⭐ |
| Scoop + Zig | ~50MB | Zig cc | Fast | ⭐⭐⭐⭐⭐ |
| Visual Studio | ~30GB | MSVC | Slow | ⭐ |

## Option 1: MSYS2 + MinGW-w64 (Recommended for China)

MSYS2 provides a Unix-like environment with pacman package manager.

### Installation Steps

1. Download MSYS2: https://www.msys2.org/
2. Install to `C:\msys64` (default path)
3. Open **MSYS2 MINGW64** terminal from Start Menu

### Configure Tuna Mirror (Optional, Recommended for China)

Create or edit `C:\msys64\etc\pacman.d\mirrorlist.mingw64`:

```
Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/mingw/x86_64/
Server = https://mirror.msys2.org/mingw/x86_64/
```

Create or edit `C:\msys64\etc\pacman.d\mirrorlist.msys`:

```
Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/msys/$arch/
Server = https://mirror.msys2.org/msys/$arch/
```

Then refresh:

```bash
pacman -Sy
```

### Install Dependencies

```bash
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-ninja mingw-w64-x86_64-vulkan \
          mingw-w64-x86_64-python mingw-w64-x86_64-python-pip
```

### Build Project

```bash
cd /c/Users/YourName/path/to/Baba
cmake -B build -G Ninja
cmake --build build
```

### Run Tests

```bash
./build/calculator_standalone.exe
```

## Option 2: Scoop + MinGW

Scoop is a command-line installer for Windows, similar to Homebrew on macOS.

### Install Scoop via Winget

```powershell
winget install scoop
```

Or install manually:

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression
```

### Configure Tuna Mirror (Recommended for China)

```powershell
# Set Tuna mirror for Scoop itself
scoop config SCOOP_REPO "https://mirrors.tuna.tsinghua.edu.cn/git/scoop-installer/scoop"
scoop config SCOOP_BRANCH "master"

# Update Scoop
scoop update

# Add extras bucket from Tuna
scoop bucket add extras https://mirrors.tuna.tsinghua.edu.cn/git/scoop-installer/scoop-extras

# Add versions bucket (for Vulkan SDK)
scoop bucket add versions https://mirrors.tuna.tsinghua.edu.cn/git/scoop-installer/versions
```

### Install Dependencies

```powershell
scoop install mingw cmake ninja vulkan-sdk
```

### Build Project

```powershell
cmake -B build -G Ninja
cmake --build build
```

### Troubleshooting

If CMake cannot find Vulkan SDK:

```powershell
$env:VULKAN_SDK = "C:\Users\$env:USERNAME\scoop\apps\vulkan-sdk\current"
cmake -B build -G Ninja
cmake --build build
```

## Option 3: Scoop + Zig (Lightest - 50MB)

Zig provides a C compiler via `zig cc` command.

### Install Scoop and Configure Mirror

See Option 2 above.

### Install Dependencies

```powershell
scoop install zig cmake ninja vulkan-sdk
```

### Build Project

```powershell
cmake -B build -G Ninja -DCMAKE_C_COMPILER="zig cc"
cmake --build build
```

## Option 4: Visual Studio (Not Recommended)

Only use this if you already have Visual Studio installed.

### Install Dependencies

1. Install Visual Studio 2022 with "Desktop development with C++" workload
2. Download Vulkan SDK: https://vulkan.lunarg.com/

### Build Project

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## Build Artifacts

After successful build:

| File | Description |
|------|-------------|
| `build/libbaba.a` | Static library |
| `build/calculator_standalone.exe` | Demo application |
| `build/examples/calculator.exe` | Framework demo |

## Performance Comparison

| Toolchain | Compile Time | Binary Size |
|-----------|--------------|-------------|
| MinGW GCC | ~3s | 84KB |
| Clang | ~2.5s | 82KB |
| Zig cc | ~3s | 84KB |
| MSVC | ~5s | 120KB |

## Common Issues

### Vulkan SDK Not Found

```powershell
# Set environment variable
$env:VULKAN_SDK = "C:\VulkanSDK\1.3.xxx.0"
# Or for Scoop
$env:VULKAN_SDK = "C:\Users\$env:USERNAME\scoop\apps\vulkan-sdk\current"
```

### Missing DLLs

If you get "vulkan-1.dll not found":

```powershell
# Add Vulkan to PATH (MSYS2)
export PATH="/mingw64/bin:$PATH"

# Or copy DLLs (Scoop)
cp $env:VULKAN_SDK\bin\vulkan-1.dll .
```