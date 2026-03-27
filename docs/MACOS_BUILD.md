# macOS Build Guide

Build Baba GUI applications on macOS. Baba uses Cocoa for native windows and Metal for GPU rendering via MoltenVK.

## System Requirements

| Requirement | Minimum | Recommended |
|-------------|---------|-------------|
| macOS | 10.14 (Mojave) | 13.0+ (Ventura) |
| Xcode Tools | 12.0 | 15.0+ |
| RAM | 4GB | 8GB+ |
| Disk | 1GB | 2GB+ |

## GPU Support

| GPU Type | Vulkan Support | Notes |
|----------|----------------|-------|
| Apple Silicon (M1/M2/M3) | ✅ Via MoltenVK | Native ARM64 |
| Intel Integrated | ✅ Via MoltenVK | MacBook Pro 2016+ |
| AMD Discrete | ✅ Via MoltenVK | MacBook Pro 2015+ |

## Install Dependencies

### Option 1: Homebrew (Recommended)

```bash
# Install Homebrew if not installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake ninja vulkan-loader vulkan-headers molten-vk

# Optional: Install Python for bindings
brew install python3
```

### Configure TUNA Mirror (Optional, for China)

```bash
# For bash users
echo 'export HOMEBREW_BREW_GIT_REMOTE="https://mirrors.tuna.tsinghua.edu.cn/git/homebrew/brew.git"' >> ~/.bashrc
echo 'export HOMEBREW_CORE_GIT_REMOTE="https://mirrors.tuna.tsinghua.edu.cn/git/homebrew/homebrew-core.git"' >> ~/.bashrc
echo 'export HOMEBREW_BOTTLE_DOMAIN="https://mirrors.tuna.tsinghua.edu.cn/homebrew-bottles"' >> ~/.bashrc
source ~/.bashrc

# For zsh users (default on macOS)
echo 'export HOMEBREW_BREW_GIT_REMOTE="https://mirrors.tuna.tsinghua.edu.cn/git/homebrew/brew.git"' >> ~/.zshrc
echo 'export HOMEBREW_CORE_GIT_REMOTE="https://mirrors.tuna.tsinghua.edu.cn/git/homebrew/homebrew-core.git"' >> ~/.zshrc
echo 'export HOMEBREW_BOTTLE_DOMAIN="https://mirrors.tuna.tsinghua.edu.cn/homebrew-bottles"' >> ~/.zshrc
source ~/.zshrc

# Update Homebrew
brew update
```

### Option 2: Manual Vulkan SDK

If you prefer official Vulkan SDK:

1. Download from: https://vulkan.lunarg.com/sdk/home
2. Install the package
3. Set environment variable:

```bash
export VULKAN_SDK=/path/to/vulkansdk-macos-1.3.xxx.0/macOS
```

## Build Project

### Clone and Build

```bash
cd /path/to/Baba

# Configure with Ninja (faster)
cmake -B build -G Ninja

# Or with Make
cmake -B build

# Build
cmake --build build -j$(sysctl -n hw.ncpu)
```

### Build with Xcode

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

## Run Demo

```bash
# Run the standalone calculator
./build/calculator_standalone

# Or the framework demo
./build/examples/calculator.app/Contents/MacOS/calculator
```

## Build Artifacts

After successful build:

| File | Description |
|------|-------------|
| `build/libbaba.a` | Static library (~100KB) |
| `build/calculator_standalone` | Standalone demo |
| `build/examples/calculator.app` | Framework demo bundle |

## Python Bindings

### Setup Virtual Environment

```bash
cd /path/to/Baba

# Create virtual environment
python3 -m venv venv
source venv/bin/activate

# Install dependencies
pip install cffi pytest

# Install Baba
pip install .
```

### Run Python Tests

```bash
pytest python/tests -v
```

### Run Python Demo

```bash
python examples/calculator.py
```

## Distribution

### Create Release Build

```bash
# Build with optimizations
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build

# Output: Universal binary supporting both Intel and Apple Silicon
```

### Create App Bundle

The calculator demo is automatically bundled as a macOS `.app`:

```bash
# Run the app
open build/examples/calculator.app
```

## Verify Installation

### Check Vulkan/MoltenVK

```bash
# Check Vulkan installation
vulkaninfo --summary

# Check MoltenVK
ls -la /usr/local/lib/libMoltenVK.dylib
```

### Check GPU

```bash
# List Metal devices (used by MoltenVK)
system_profiler SPDisplaysDataType
```

## Common Issues

### Vulkan SDK Not Found

```bash
# If using Homebrew Vulkan
export VK_ICD_FILENAMES=/usr/local/share/vulkan/icd.d/MoltenVK_icd.json

# If using official SDK
export VULKAN_SDK=/path/to/vulkansdk-macos-1.3.xxx.0/macOS
```

### Code Signing Issues

If you get code signing errors:

```bash
# Ad-hoc sign the binary
codesign --force --deep --sign - ./build/calculator_standalone
```

### Missing Metal Framework

```bash
# Ensure Xcode Command Line Tools are installed
xcode-select --install
```

### Architecture Mismatch

For Apple Silicon Macs:

```bash
# Build for ARM64 only
cmake -B build -G Ninja -DCMAKE_OSX_ARCHITECTURES=arm64

# For Intel Macs
cmake -B build -G Ninja -DCMAKE_OSX_ARCHITECTURES=x86_64

# Universal binary (both architectures)
cmake -B build -G Ninja -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
```

### Library Not Found

If linking fails:

```bash
# Set library path
export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
export CPATH=/usr/local/include:$CPATH

# For Homebrew on Apple Silicon
export LIBRARY_PATH=/opt/homebrew/lib:$LIBRARY_PATH
export CPATH=/opt/homebrew/include:$CPATH
```

## Performance

Baba is optimized for macOS:

| Metric | Intel Mac | Apple Silicon |
|--------|-----------|---------------|
| Compile time | ~2s | ~1.5s |
| Binary size | ~100KB | ~100KB |
| Memory usage | ~5MB | ~3MB |