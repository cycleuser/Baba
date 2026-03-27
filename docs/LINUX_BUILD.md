# Linux Build Guide

Build Baba GUI applications on Linux. This guide covers Ubuntu/Debian, with notes for other distributions.

## System Requirements

| Requirement | Minimum | Recommended |
|-------------|---------|-------------|
| OS | Ubuntu 20.04+ | Ubuntu 22.04+ |
| RAM | 2GB | 4GB+ |
| Disk | 500MB | 1GB+ |
| GPU | Vulkan 1.0+ | Vulkan 1.1+ |

## Display Servers

Baba supports both X11 and Wayland:

| Display Server | Status | Notes |
|----------------|--------|-------|
| X11 | ✅ Full Support | Most common |
| Wayland | ✅ Full Support | Modern, requires Wayland compositor |

## Ubuntu/Debian

### Install Dependencies

```bash
# Update package lists
sudo apt update

# Install build tools
sudo apt install -y build-essential cmake ninja-build pkg-config

# Install Vulkan SDK
sudo apt install -y libvulkan-dev vulkan-validationlayers

# Install X11 support
sudo apt install -y libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

# Install Wayland support
sudo apt install -y libwayland-dev wayland-protocols libxkbcommon-dev

# Install Python (for Python bindings)
sudo apt install -y python3 python3-pip python3-venv
```

### Configure Tuna Mirror (Optional, for China)

```bash
# Backup original sources
sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak

# For Ubuntu 22.04 (Jammy)
sudo cat > /etc/apt/sources.list << 'EOF'
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-updates main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-backports main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-security main restricted universe multiverse
EOF

# For Ubuntu 20.04 (Focal), replace "jammy" with "focal"
sudo apt update
```

### Build Project

```bash
cd /path/to/Baba

# Configure with Ninja (faster)
cmake -B build -G Ninja

# Or with Make
cmake -B build

# Build
cmake --build build -j$(nproc)
```

### Run Demo

```bash
# X11
./build/calculator_standalone

# Wayland (if available)
WAYLAND_DISPLAY=wayland-0 ./build/calculator_standalone
```

## Arch Linux

### Install Dependencies

```bash
# Install all dependencies
sudo pacman -S --needed base-devel cmake ninja vulkan-headers vulkan-tools \
    libx11 libxrandr libxinerama libxcursor libxi \
    wayland wayland-protocols libxkbcommon \
    python python-pip

# For Intel GPUs
sudo pacman -S vulkan-intel

# For AMD GPUs
sudo pacman -S vulkan-radeon

# For NVIDIA GPUs
sudo pacman -S nvidia
```

### Configure TUNA Mirror (Optional, for China)

```bash
# Edit /etc/pacman.d/mirrorlist
sudo sed -i 's|^Server = .*|Server = https://mirrors.tuna.tsinghua.edu.cn/archlinux/$repo/os/$arch|' /etc/pacman.d/mirrorlist
sudo pacman -Sy
```

### Build Project

```bash
cmake -B build -G Ninja
cmake --build build -j$(nproc)
```

## Fedora

### Install Dependencies

```bash
sudo dnf install -y gcc cmake ninja-build vulkan-loader-devel vulkan-headers \
    libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel \
    wayland-devel wayland-protocols-devel libxkbcommon-devel \
    python3 python3-pip
```

### Configure TUNA Mirror (Optional, for China)

```bash
# Replace Fedora mirrors with TUNA
sudo sed -i 's|^#baseurl=http://download.example.com/pub/fedora/linux|baseurl=https://mirrors.tuna.tsinghua.edu.cn/fedora|' /etc/yum.repos.d/fedora*.repo
sudo sed -i 's|^metalink=|#metalink=|' /etc/yum.repos.d/fedora*.repo
sudo dnf makecache
```

### Build Project

```bash
cmake -B build -G Ninja
cmake --build build -j$(nproc)
```

## Verify Vulkan Installation

Check if Vulkan is properly installed:

```bash
# Check Vulkan version
vulkaninfo --summary

# List available GPUs
vulkaninfo --summary | grep deviceName

# Test Vulkan
vkcube
```

If `vkcube` fails, install GPU drivers:

```bash
# Ubuntu - Intel
sudo apt install -y mesa-vulkan-drivers

# Ubuntu - AMD (usually included)
sudo apt install -y mesa-vulkan-drivers

# Ubuntu - NVIDIA
sudo apt install -y nvidia-driver-535  # or latest
```

## Build Artifacts

After successful build:

| File | Description |
|------|-------------|
| `build/libbaba.a` | Static library |
| `build/calculator_standalone` | Demo application |
| `build/examples/calculator` | Framework demo |

## Python Bindings

### Install Python Package

```bash
cd /path/to/Baba
python3 -m venv venv
source venv/bin/activate
pip install cffi pytest
pip install .
```

### Run Python Tests

```bash
pytest python/tests -v
```

## Common Issues

### Vulkan Not Found

```bash
# Check Vulkan installation
vulkaninfo

# If missing, install
sudo apt install -y libvulkan-dev vulkan-validationlayers
```

### X11 Headers Missing

```bash
sudo apt install -y libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

### Wayland Headers Missing

```bash
sudo apt install -y libwayland-dev wayland-protocols libxkbcommon-dev
```

### Display Not Found

```bash
# Check DISPLAY variable
echo $DISPLAY

# For X11
export DISPLAY=:0

# For Wayland
echo $WAYLAND_DISPLAY
```