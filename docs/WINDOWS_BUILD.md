# Windows Build Guide

Build Baba GUI applications on Windows without the heavy Visual Studio (~30GB).

## Quick Start

```powershell
# Install dependencies via winget
winget install MSYS2.MSYS2
winget install KhronosGroup.VulkanSDK
winget install Kitware.CMake

# Restart terminal, then in MSYS2 MINGW64 terminal:
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-ninja

# Build
cmake -B build -G Ninja
cmake --build build
```

## Toolchain Comparison

| Toolchain | Install Size | Compiler | Speed | Recommended |
|-----------|--------------|----------|-------|-------------|
| MSYS2 + MinGW | ~500MB | GCC | Fast | ⭐⭐⭐⭐⭐ |
| Winget + MinGW | ~500MB | GCC | Fast | ⭐⭐⭐⭐⭐ |
| Visual Studio | ~30GB | MSVC | Slow | ⭐ |

## Option 1: MSYS2 + MinGW-w64 (Recommended)

MSYS2 provides a Unix-like environment with pacman package manager.

### Installation Steps

1. Install MSYS2:
   ```powershell
   winget install MSYS2.MSYS2
   ```

2. Open **MSYS2 MINGW64** terminal from Start Menu

3. Install Vulkan SDK:
   ```powershell
   winget install KhronosGroup.VulkanSDK
   ```
   Restart terminal after installation.

### Configure Tuna Mirror (Optional, for China)

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
          mingw-w64-x86_64-ninja
```

Vulkan SDK is installed via winget (see above).

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

## Option 2: Winget + MinGW (Simple)

Simple installation using only winget.

### Install Dependencies

```powershell
# Install MinGW (via MSYS2)
winget install MSYS2.MSYS2

# Install Vulkan SDK
winget install KhronosGroup.VulkanSDK

# Install CMake (optional, if not using MSYS2's cmake)
winget install Kitware.CMake
```

Restart terminal after installation.

### Build Project

Open **MSYS2 MINGW64** terminal:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
cd /c/Users/YourName/path/to/Baba
cmake -B build -G Ninja
cmake --build build
```

## Option 3: Visual Studio (Not Recommended)

Only use this if you already have Visual Studio installed.

### Install Dependencies

```powershell
# Install Visual Studio first, then:
winget install KhronosGroup.VulkanSDK
```

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
| MSVC | ~5s | 120KB |

## Common Issues

### Vulkan SDK Not Found

If CMake cannot find Vulkan SDK:

```powershell
# Check Vulkan SDK installation
echo $env:VULKAN_SDK

# If empty, find and set it manually
dir C:\VulkanSDK\
$env:VULKAN_SDK = "C:\VulkanSDK\1.3.xxx.0"
```

### "vulkan/vulkan.h not found"

1. Install Vulkan SDK:
   ```powershell
   winget install KhronosGroup.VulkanSDK
   ```

2. Restart terminal

3. Verify installation:
   ```powershell
   echo $env:VULKAN_SDK
   dir C:\VulkanSDK\
   ```

### Missing DLLs

If you get "vulkan-1.dll not found":

```powershell
# Add Vulkan to PATH (MSYS2)
export PATH="/mingw64/bin:$PATH"

# Or copy DLLs
cp $env:VULKAN_SDK\bin\vulkan-1.dll .
```

### GCC Not Found

Make sure to open **MSYS2 MINGW64** terminal (not MSYS2 MSYS terminal).

Or add MinGW to PATH:
```powershell
$env:PATH = "C:\msys64\mingw64\bin;" + $env:PATH
```