#!/usr/bin/env bash
# Baba - Build native library for current platform
# For cross-platform builds, use GitHub Actions (see .github/workflows/)
set -e
cd "$(dirname "${BASH_SOURCE[0]}")"

echo "=== Baba Native Build ==="
echo ""

PYTHON="${PYTHON:-python3}"
VERSION_FILE="python/baba/__init__.py"

get_version() {
    sed -n 's/.*__version__.*"\([^"]*\)".*/\1/p' "$VERSION_FILE" | head -1
}

VERSION=$(get_version)
echo "Version: $VERSION"
echo ""

echo "[1/4] Preparing directories..."
rm -rf build dist/lib dist/include
mkdir -p build dist/lib/macos dist/lib/linux-x64 dist/lib/windows-x64 dist/include
mkdir -p python/baba/lib python/baba/include

echo "[2/4] Copying headers..."
for dir in core widgets layout render platform theme; do
    mkdir -p "dist/include/$dir"
    cp src/$dir/*.h "dist/include/$dir/" 2>/dev/null || true
done
cp src/*.h dist/include/ 2>/dev/null || true
echo "  Headers copied to dist/include/"

echo "[3/4] Building native library..."

OS=$(uname -s)
CMAKE_GENERATOR="Unix Makefiles"
if command -v ninja &> /dev/null; then
    CMAKE_GENERATOR="Ninja"
    echo "  Using Ninja"
else
    echo "  Using Make"
fi

case "$OS" in
    Darwin)
        echo "  Building for macOS (Universal)..."
        cmake -B build -G "$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
            -DBABA_BUILD_EXAMPLES=OFF \
            -DBABA_BUILD_TESTS=OFF
        cmake --build build --config Release -j
        cp build/libbaba.a dist/lib/macos/libbaba.a
        cp build/libbaba.a python/baba/lib/libbaba_macos.a
        echo "    Built: dist/lib/macos/libbaba.a ($(ls -lh dist/lib/macos/libbaba.a | awk '{print $5}'))"
        ;;
    Linux)
        echo "  Building for Linux x64..."
        cmake -B build -G "$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=Release \
            -DBABA_BUILD_EXAMPLES=OFF \
            -DBABA_BUILD_TESTS=OFF
        cmake --build build --config Release -j
        cp build/libbaba.a dist/lib/linux-x64/libbaba.a
        cp build/libbaba.a python/baba/lib/libbaba_linux.a
        echo "    Built: dist/lib/linux-x64/libbaba.a"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        echo "  Building for Windows x64..."
        cmake -B build -G "$CMAKE_GENERATOR" -DCMAKE_BUILD_TYPE=Release \
            -DBABA_BUILD_EXAMPLES=OFF \
            -DBABA_BUILD_TESTS=OFF
        cmake --build build --config Release -j
        cp build/libbaba.a dist/lib/windows-x64/libbaba.a
        cp build/libbaba.a python/baba/lib/libbaba_windows.a
        echo "    Built: dist/lib/windows-x64/libbaba.a"
        ;;
    *)
        echo "  ERROR: Unknown platform: $OS"
        exit 1
        ;;
esac

echo "[4/4] Preparing Python package..."
cp -r dist/include/* python/baba/include/

echo ""
echo "=== Build Complete ==="
echo ""
echo "Libraries:"
ls -lh dist/lib/*/*.a 2>/dev/null || echo "  No libraries built"
echo ""
echo "For cross-platform builds, use GitHub Actions:"
echo "  See .github/workflows/ for automated multi-platform builds"
echo ""