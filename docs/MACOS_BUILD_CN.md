# macOS构建指南

在macOS上构建Baba GUI应用。Baba使用Cocoa创建原生窗口，通过MoltenVK实现Vulkan到Metal的转换。

## 系统要求

| 要求 | 最低 | 推荐 |
|------|------|------|
| macOS | 10.14 (Mojave) | 13.0+ (Ventura) |
| Xcode工具 | 12.0 | 15.0+ |
| 内存 | 4GB | 8GB+ |
| 磁盘 | 1GB | 2GB+ |

## GPU支持

| GPU类型 | Vulkan支持 | 说明 |
|---------|-----------|------|
| Apple Silicon (M1/M2/M3) | ✅ 通过MoltenVK | 原生ARM64 |
| Intel核显 | ✅ 通过MoltenVK | MacBook Pro 2016+ |
| AMD独显 | ✅ 通过MoltenVK | MacBook Pro 2015+ |

## 安装依赖

### 方式1: Homebrew (推荐)

```bash
# 如果未安装Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装依赖
brew install cmake ninja vulkan-loader vulkan-headers molten-vk

# 可选：安装Python用于绑定
brew install python3
```

### 配置清华镜像 (国内推荐)

```bash
# bash用户
echo 'export HOMEBREW_BREW_GIT_REMOTE="https://mirrors.tuna.tsinghua.edu.cn/git/homebrew/brew.git"' >> ~/.bashrc
echo 'export HOMEBREW_CORE_GIT_REMOTE="https://mirrors.tuna.tsinghua.edu.cn/git/homebrew/homebrew-core.git"' >> ~/.bashrc
echo 'export HOMEBREW_BOTTLE_DOMAIN="https://mirrors.tuna.tsinghua.edu.cn/homebrew-bottles"' >> ~/.bashrc
source ~/.bashrc

# zsh用户 (macOS默认)
echo 'export HOMEBREW_BREW_GIT_REMOTE="https://mirrors.tuna.tsinghua.edu.cn/git/homebrew/brew.git"' >> ~/.zshrc
echo 'export HOMEBREW_CORE_GIT_REMOTE="https://mirrors.tuna.tsinghua.edu.cn/git/homebrew/homebrew-core.git"' >> ~/.zshrc
echo 'export HOMEBREW_BOTTLE_DOMAIN="https://mirrors.tuna.tsinghua.edu.cn/homebrew-bottles"' >> ~/.zshrc
source ~/.zshrc

# 更新Homebrew
brew update
```

### 方式2: 手动安装Vulkan SDK

如果偏好官方Vulkan SDK：

1. 从以下地址下载：https://vulkan.lunarg.com/sdk/home
2. 安装软件包
3. 设置环境变量：

```bash
export VULKAN_SDK=/path/to/vulkansdk-macos-1.3.xxx.0/macOS
```

## 构建项目

### 克隆并构建

```bash
cd /path/to/Baba

# 使用Ninja配置（更快）
cmake -B build -G Ninja

# 或使用Make
cmake -B build

# 构建
cmake --build build -j$(sysctl -n hw.ncpu)
```

### 使用Xcode构建

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

## 运行演示

```bash
# 运行独立计算器
./build/calculator_standalone

# 或框架演示
./build/examples/calculator.app/Contents/MacOS/calculator
```

## 构建产物

构建成功后：

| 文件 | 说明 |
|------|------|
| `build/libbaba.a` | 静态库 (~100KB) |
| `build/calculator_standalone` | 独立演示 |
| `build/examples/calculator.app` | 框架演示包 |

## Python绑定

### 设置虚拟环境

```bash
cd /path/to/Baba

# 创建虚拟环境
python3 -m venv venv
source venv/bin/activate

# 安装依赖
pip install cffi pytest

# 安装Baba
pip install .
```

### 运行Python测试

```bash
pytest python/tests -v
```

### 运行Python演示

```bash
python examples/calculator.py
```

## 分发

### 创建发布版本

```bash
# 使用优化构建
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build

# 输出：同时支持Intel和Apple Silicon的通用二进制
```

### 创建应用包

计算器演示会自动打包为macOS `.app`：

```bash
# 运行应用
open build/examples/calculator.app
```

## 验证安装

### 检查Vulkan/MoltenVK

```bash
# 检查Vulkan安装
vulkaninfo --summary

# 检查MoltenVK
ls -la /usr/local/lib/libMoltenVK.dylib
```

### 检查GPU

```bash
# 列出Metal设备（MoltenVK使用）
system_profiler SPDisplaysDataType
```

## 常见问题

### 找不到Vulkan SDK

```bash
# 如果使用Homebrew Vulkan
export VK_ICD_FILENAMES=/usr/local/share/vulkan/icd.d/MoltenVK_icd.json

# 如果使用官方SDK
export VULKAN_SDK=/path/to/vulkansdk-macos-1.3.xxx.0/macOS
```

### 代码签名问题

如果遇到代码签名错误：

```bash
# 临时签名二进制
codesign --force --deep --sign - ./build/calculator_standalone
```

### 缺少Metal框架

```bash
# 确保安装了Xcode命令行工具
xcode-select --install
```

### 架构不匹配

对于Apple Silicon Mac：

```bash
# 仅构建ARM64
cmake -B build -G Ninja -DCMAKE_OSX_ARCHITECTURES=arm64

# 仅构建Intel
cmake -B build -G Ninja -DCMAKE_OSX_ARCHITECTURES=x86_64

# 通用二进制（两种架构）
cmake -B build -G Ninja -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
```

### 找不到库

如果链接失败：

```bash
# 设置库路径
export LIBRARY_PATH=/usr/local/lib:$LIBRARY_PATH
export CPATH=/usr/local/include:$CPATH

# Apple Silicon上的Homebrew
export LIBRARY_PATH=/opt/homebrew/lib:$LIBRARY_PATH
export CPATH=/opt/homebrew/include:$CPATH
```

## 性能

Baba针对macOS进行了优化：

| 指标 | Intel Mac | Apple Silicon |
|------|-----------|---------------|
| 编译时间 | ~2秒 | ~1.5秒 |
| 二进制大小 | ~100KB | ~100KB |
| 内存使用 | ~5MB | ~3MB |