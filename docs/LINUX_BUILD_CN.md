# Linux构建指南

在Linux上构建Baba GUI应用。本指南以Ubuntu/Debian为主，同时提供其他发行版的说明。

## 系统要求

| 要求 | 最低 | 推荐 |
|------|------|------|
| 操作系统 | Ubuntu 20.04+ | Ubuntu 22.04+ |
| 内存 | 2GB | 4GB+ |
| 磁盘 | 500MB | 1GB+ |
| 显卡 | Vulkan 1.0+ | Vulkan 1.1+ |

## 显示服务器

Baba同时支持X11和Wayland：

| 显示服务器 | 状态 | 说明 |
|-----------|------|------|
| X11 | ✅ 完整支持 | 最常见 |
| Wayland | ✅ 完整支持 | 现代化，需要Wayland合成器 |

## Ubuntu/Debian

### 安装依赖

```bash
# 更新软件包列表
sudo apt update

# 安装构建工具
sudo apt install -y build-essential cmake ninja-build pkg-config

# 安装Vulkan SDK
sudo apt install -y libvulkan-dev vulkan-validationlayers

# 安装X11支持
sudo apt install -y libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

# 安装Wayland支持
sudo apt install -y libwayland-dev wayland-protocols libxkbcommon-dev

# 安装Python（用于Python绑定）
sudo apt install -y python3 python3-pip python3-venv
```

### 配置清华镜像 (国内推荐)

```bash
# 备份原始源
sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak

# Ubuntu 22.04 (Jammy)
sudo cat > /etc/apt/sources.list << 'EOF'
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-updates main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-backports main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy-security main restricted universe multiverse
EOF

# Ubuntu 20.04 (Focal)，将"jammy"替换为"focal"
sudo apt update
```

### 构建项目

```bash
cd /path/to/Baba

# 使用Ninja配置（更快）
cmake -B build -G Ninja

# 或使用Make
cmake -B build

# 构建
cmake --build build -j$(nproc)
```

### 运行演示

```bash
# X11
./build/calculator_standalone

# Wayland（如果可用）
WAYLAND_DISPLAY=wayland-0 ./build/calculator_standalone
```

## Arch Linux

### 安装依赖

```bash
# 安装所有依赖
sudo pacman -S --needed base-devel cmake ninja vulkan-headers vulkan-tools \
    libx11 libxrandr libxinerama libxcursor libxi \
    wayland wayland-protocols libxkbcommon \
    python python-pip

# Intel显卡
sudo pacman -S vulkan-intel

# AMD显卡
sudo pacman -S vulkan-radeon

# NVIDIA显卡
sudo pacman -S nvidia
```

### 配置清华镜像 (国内推荐)

```bash
# 编辑 /etc/pacman.d/mirrorlist
sudo sed -i 's|^Server = .*|Server = https://mirrors.tuna.tsinghua.edu.cn/archlinux/$repo/os/$arch|' /etc/pacman.d/mirrorlist
sudo pacman -Sy
```

### 构建项目

```bash
cmake -B build -G Ninja
cmake --build build -j$(nproc)
```

## Fedora

### 安装依赖

```bash
sudo dnf install -y gcc cmake ninja-build vulkan-loader-devel vulkan-headers \
    libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel \
    wayland-devel wayland-protocols-devel libxkbcommon-devel \
    python3 python3-pip
```

### 配置清华镜像 (国内推荐)

```bash
# 替换Fedora镜像为清华源
sudo sed -i 's|^#baseurl=http://download.example.com/pub/fedora/linux|baseurl=https://mirrors.tuna.tsinghua.edu.cn/fedora|' /etc/yum.repos.d/fedora*.repo
sudo sed -i 's|^metalink=|#metalink=|' /etc/yum.repos.d/fedora*.repo
sudo dnf makecache
```

### 构建项目

```bash
cmake -B build -G Ninja
cmake --build build -j$(nproc)
```

## 验证Vulkan安装

检查Vulkan是否正确安装：

```bash
# 查看Vulkan版本
vulkaninfo --summary

# 列出可用GPU
vulkaninfo --summary | grep deviceName

# 测试Vulkan
vkcube
```

如果`vkcube`失败，安装显卡驱动：

```bash
# Ubuntu - Intel
sudo apt install -y mesa-vulkan-drivers

# Ubuntu - AMD（通常已包含）
sudo apt install -y mesa-vulkan-drivers

# Ubuntu - NVIDIA
sudo apt install -y nvidia-driver-535  # 或最新版本
```

## 构建产物

构建成功后：

| 文件 | 说明 |
|------|------|
| `build/libbaba.a` | 静态库 |
| `build/calculator_standalone` | 演示程序 |
| `build/examples/calculator` | 框架演示 |

## Python绑定

### 安装Python包

```bash
cd /path/to/Baba
python3 -m venv venv
source venv/bin/activate
pip install cffi pytest
pip install .
```

### 运行Python测试

```bash
pytest python/tests -v
```

## 常见问题

### 找不到Vulkan

```bash
# 检查Vulkan安装
vulkaninfo

# 如果缺失，安装
sudo apt install -y libvulkan-dev vulkan-validationlayers
```

### 缺少X11头文件

```bash
sudo apt install -y libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

### 缺少Wayland头文件

```bash
sudo apt install -y libwayland-dev wayland-protocols libxkbcommon-dev
```

### 找不到显示器

```bash
# 检查DISPLAY变量
echo $DISPLAY

# X11
export DISPLAY=:0

# Wayland
echo $WAYLAND_DISPLAY
```