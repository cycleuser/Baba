#!/usr/bin/env bash
# Baba - Build and upload to PyPI
# Builds for all platforms (macOS, Linux, Windows) and uploads to PyPI
set -e
cd "$(dirname "${BASH_SOURCE[0]}")"

PYTHON="${PYTHON:-python3}"
VERSION_FILE="python/baba/__init__.py"

echo "=== Baba PyPI Upload ==="
echo ""

echo "[1/6] Bumping patch version..."
"$PYTHON" -c "
import re, sys
p = '$VERSION_FILE'
t = open(p, encoding='utf-8').read()
m = re.search(r'(__version__\s*=\s*\"(\d+\.\d+\.)(\d+)\")', t)
if not m: print('ERROR: cannot parse version'); sys.exit(1)
old_v = m.group(2) + m.group(3)
new_v = m.group(2) + str(int(m.group(3)) + 1)
open(p, 'w', encoding='utf-8').write(t.replace(m.group(1), '__version__ = \"' + new_v + '\"'))
print(f'  {old_v} -> {new_v}')
"

VERSION=$("$PYTHON" -c "
import re
t = open('$VERSION_FILE', encoding='utf-8').read()
m = re.search(r'__version__\s*=\s*\"(\d+\.\d+\.\d+)\"', t)
print(m.group(1) if m else '0.0.0')
")
echo "  Version: $VERSION"
echo ""

echo "[2/6] Cleaning old builds..."
rm -rf dist/*.tar.gz dist/*.whl build python/baba/lib python/baba/include
mkdir -p python/baba/lib python/baba/include

echo "[3/6] Building for all platforms..."
./build_all.sh

echo ""
echo "[4/6] Checking build artifacts..."
LIB_COUNT=$(find python/baba/lib -name "*.a" 2>/dev/null | wc -l | tr -d ' ')
echo "  Libraries: $LIB_COUNT"
if [ "$LIB_COUNT" -eq 0 ]; then
    echo "  WARNING: No libraries built!"
    echo "  Install Zig for cross-compilation: brew install zig"
fi

echo "[5/6] Building Python package..."
"$PYTHON" -m pip install --upgrade build twine -q
"$PYTHON" -m build

echo "[6/6] Uploading to PyPI..."
"$PYTHON" -m twine check dist/*.tar.gz dist/*.whl 2>/dev/null || true
"$PYTHON" -m twine upload dist/*.tar.gz dist/*.whl

echo ""
echo "=== Done! ==="
echo "  Package: baba-gui==$VERSION"
echo "  Install: pip install baba-gui"
echo ""