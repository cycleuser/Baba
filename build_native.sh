#!/usr/bin/env bash
# Baba - Build native C library for distribution
set -e
cd "$(dirname "${BASH_SOURCE[0]}")"

echo "=== Baba Native Library Build ==="

OS=$(uname -s)
ARCH=$(uname -m)

echo "Platform: $OS ($ARCH)"

clean_build() {
    echo "[1/4] Cleaning..."
    rm -rf build dist/lib dist/include
    mkdir -p dist/lib dist/include
}

build_macos() {
    echo "[2/4] Building for macOS (Universal)..."
    
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
        -DBABA_BUILD_EXAMPLES=OFF \
        -DBABA_BUILD_TESTS=OFF
    
    cmake --build build --config Release -j
    
    cp build/libbaba.a dist/lib/macos/libbaba.a
    echo "  Built: dist/lib/macos/libbaba.a"
}

build_linux() {
    echo "[2/4] Building for Linux..."
    
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DBABA_BUILD_EXAMPLES=OFF \
        -DBABA_BUILD_TESTS=OFF
    
    cmake --build build --config Release -j
    
    mkdir -p dist/lib/linux-x64
    cp build/libbaba.a dist/lib/linux-x64/libbaba.a
    echo "  Built: dist/lib/linux-x64/libbaba.a"
}

copy_headers() {
    echo "[3/4] Copying headers..."
    
    mkdir -p dist/include
    cp -r src/*.h dist/include/
    cp -r src/core/*.h dist/include/core/ 2>/dev/null || mkdir -p dist/include/core
    cp -r src/widgets/*.h dist/include/widgets/ 2>/dev/null || mkdir -p dist/include/widgets
    cp -r src/layout/*.h dist/include/layout/ 2>/dev/null || mkdir -p dist/include/layout
    cp -r src/render/*.h dist/include/render/ 2>/dev/null || mkdir -p dist/include/render
    cp -r src/platform/*.h dist/include/platform/ 2>/dev/null || mkdir -p dist/include/platform
    cp -r src/theme/*.h dist/include/theme/ 2>/dev/null || mkdir -p dist/include/theme
    
    echo "  Headers copied to dist/include/"
}

create_dist_package() {
    echo "[4/4] Creating distribution package..."
    
    VERSION=$(grep -oP '__version__\s*=\s*"\K[\d.]+' python/baba/__init__.py)
    PACKAGE_NAME="baba-dev-${VERSION}-${OS}-${ARCH}"
    
    mkdir -p "dist/${PACKAGE_NAME}"
    cp -r dist/include "dist/${PACKAGE_NAME}/"
    cp -r dist/lib "dist/${PACKAGE_NAME}/"
    cp README.md "dist/${PACKAGE_NAME}/" 2>/dev/null || true
    cp LICENSE "dist/${PACKAGE_NAME}/" 2>/dev/null || true
    
    tar -czf "dist/${PACKAGE_NAME}.tar.gz" -C dist "${PACKAGE_NAME}"
    
    echo "  Package: dist/${PACKAGE_NAME}.tar.gz"
}

clean_build

case "$OS" in
    Darwin)
        build_macos
        ;;
    Linux)
        build_linux
        ;;
    *)
        echo "Unsupported OS: $OS"
        exit 1
        ;;
esac

copy_headers
create_dist_package

echo ""
echo "=== Done! ==="
echo ""
echo "Distribution files:"
ls -la dist/*.tar.gz 2>/dev/null || echo "  No packages created"
echo ""
echo "Library: dist/lib/"
echo "Headers: dist/include/"
echo ""