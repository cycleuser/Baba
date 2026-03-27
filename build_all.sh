#!/usr/bin/env bash
# Baba - Build all platforms for PyPI distribution
# This script builds native libraries for macOS, Linux, and Windows
set -e
cd "$(dirname "${BASH_SOURCE[0]}")"

PYTHON="${PYTHON:-python3}"
VERSION_FILE="python/baba/__init__.py"

echo "=== Baba Multi-Platform Build ==="
echo ""

get_version() {
    sed -n 's/.*__version__.*"\([^"]*\)".*/\1/p' "$VERSION_FILE" | head -1
}

VERSION=$(get_version)
echo "Version: $VERSION"
echo ""

echo "[1/5] Cleaning..."
rm -rf build/* dist/lib dist/include
mkdir -p dist/lib/macos dist/lib/linux-x64 dist/lib/windows-x64
mkdir -p dist/include
mkdir -p python/baba/lib python/baba/include

echo "[2/5] Copying headers..."
for dir in core widgets layout render platform theme; do
    mkdir -p "dist/include/$dir"
    cp src/$dir/*.h "dist/include/$dir/" 2>/dev/null || true
done
cp src/*.h dist/include/
echo "  Headers copied to dist/include/"

echo "[3/5] Building libraries..."

HAS_ZIG=false
HAS_DOCKER=false
HAS_NINJA=false

if command -v zig &> /dev/null; then
    HAS_ZIG=true
    echo "  Found Zig for cross-compilation"
fi

if command -v docker &> /dev/null; then
    HAS_DOCKER=true
    echo "  Found Docker for Linux build"
fi

if command -v ninja &> /dev/null; then
    HAS_NINJA=true
    echo "  Found Ninja build tool"
fi

CMAKE_GENERATOR="Unix Makefiles"
if [ "$HAS_NINJA" = true ]; then
    CMAKE_GENERATOR="Ninja"
fi

build_macos() {
    echo ""
    echo "  === Building macOS (Universal) ==="
    
    if [[ "$OSTYPE" == "darwin"* ]]; then
        cmake -B build/macos -G "$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
            -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF
        cmake --build build/macos --config Release -j
        cp build/macos/libbaba.a dist/lib/macos/libbaba.a
        echo "    Built: dist/lib/macos/libbaba.a"
    elif [ "$HAS_ZIG" = true ]; then
        echo "    Cross-compiling with Zig..."
        cmake -B build/macos -G "$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_C_COMPILER="zig cc -target aarch64-macos-none" \
            -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF \
            -DBABA_CROSS_COMPILE=ON -DBABA_TARGET_PLATFORM=macos
        cmake --build build/macos --config Release -j
        cp build/macos/libbaba.a dist/lib/macos/libbaba.a
        echo "    Built: dist/lib/macos/libbaba.a (cross-compiled)"
    else
        echo "    SKIPPED: Run on macOS or install Zig for cross-compilation"
    fi
}

build_linux() {
    echo ""
    echo "  === Building Linux x64 ==="
    
    if [ "$HAS_DOCKER" = true ]; then
        echo "    Using Docker..."
        docker run --rm -v "$(pwd):/src" -w /src \
            -e DEBIAN_FRONTEND=noninteractive \
            ubuntu:22.04 \
            bash -c "apt-get update && apt-get install -y build-essential cmake ninja-build libvulkan-dev libx11-dev libwayland-dev && \
                cmake -B build/linux -G Ninja -DCMAKE_BUILD_TYPE=Release -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF && \
                cmake --build build/linux --config Release -j"
        cp build/linux/libbaba.a dist/lib/linux-x64/libbaba.a
        echo "    Built: dist/lib/linux-x64/libbaba.a"
    elif [ "$HAS_ZIG" = true ]; then
        echo "    Cross-compiling with Zig..."
        cmake -B build/linux -G "$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_C_COMPILER="zig cc -target x86_64-linux-gnu" \
            -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF \
            -DBABA_CROSS_COMPILE=ON -DBABA_TARGET_PLATFORM=linux
        cmake --build build/linux --config Release -j
        cp build/linux/libbaba.a dist/lib/linux-x64/libbaba.a
        echo "    Built: dist/lib/linux-x64/libbaba.a (cross-compiled)"
    elif [[ "$OSTYPE" == "linux"* ]]; then
        cmake -B build/linux -G "$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=Release \
            -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF
        cmake --build build/linux --config Release -j
        cp build/linux/libbaba.a dist/lib/linux-x64/libbaba.a
        echo "    Built: dist/lib/linux-x64/libbaba.a"
    else
        echo "    SKIPPED: Install Docker or Zig for cross-compilation"
    fi
}

build_windows() {
    echo ""
    echo "  === Building Windows x64 ==="
    
    if [ "$HAS_ZIG" = true ]; then
        echo "    Cross-compiling with Zig..."
        cmake -B build/windows -G "$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_C_COMPILER="zig cc -target x86_64-windows-gnu" \
            -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF \
            -DBABA_CROSS_COMPILE=ON -DBABA_TARGET_PLATFORM=windows
        cmake --build build/windows --config Release -j
        cp build/windows/libbaba.a dist/lib/windows-x64/libbaba.a
        echo "    Built: dist/lib/windows-x64/libbaba.a (cross-compiled)"
    elif [[ "$OSTYPE" == "msys"* ]] || [[ "$OSTYPE" == "cygwin"* ]]; then
        cmake -B build/windows -G "$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=Release \
            -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF
        cmake --build build/windows --config Release -j
        cp build/windows/libbaba.a dist/lib/windows-x64/libbaba.a
        echo "    Built: dist/lib/windows-x64/libbaba.a"
    else
        echo "    SKIPPED: Install Zig for cross-compilation"
    fi
}

build_macos
build_linux
build_windows

echo ""
echo "[4/5] Preparing Python package..."
cp -r dist/include/* python/baba/include/
mkdir -p python/baba/lib
if [ -f dist/lib/macos/libbaba.a ]; then
    cp dist/lib/macos/libbaba.a python/baba/lib/libbaba_macos.a
fi
if [ -f dist/lib/linux-x64/libbaba.a ]; then
    cp dist/lib/linux-x64/libbaba.a python/baba/lib/libbaba_linux.a
fi
if [ -f dist/lib/windows-x64/libbaba.a ]; then
    cp dist/lib/windows-x64/libbaba.a python/baba/lib/libbaba_windows.a
fi

echo "[5/5] Summary..."
echo ""
echo "  Libraries built:"
ls -la dist/lib/*/*.a 2>/dev/null || echo "    No libraries found"
echo ""
echo "  To build missing platforms, install:"
echo "    - Zig:  brew install zig (or scoop install zig on Windows)"
echo "    - Docker: brew install docker (for native Linux build)"
echo ""

if [ "$1" == "--upload" ]; then
    echo "Building and uploading to PyPI..."
    "$PYTHON" -m pip install --upgrade build twine -q
    "$PYTHON" -m build
    "$PYTHON" -m twine check dist/*.tar.gz dist/*.whl 2>/dev/null || true
    "$PYTHON" -m twine upload dist/*.tar.gz dist/*.whl 2>/dev/null || \
        echo "Upload failed. Run: twine upload dist/*"
fi

echo "=== Done! ==="