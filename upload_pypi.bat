@echo off
REM Baba - Build and upload to PyPI
REM Builds for all platforms and uploads to PyPI
setlocal enabledelayedexpansion

echo === Baba PyPI Upload ===
echo.

REM Get version
for /f "tokens=2 delims==" %%v in ('findstr /r "__version__.*=.*\"" python\baba\__init__.py') do (
    set "VERSION=%%~v"
)

echo [1/6] Bumping patch version...
python -c "import re; t=open('python/baba/__init__.py','r',encoding='utf-8').read(); m=re.search(r'(__version__\s*=\s*\"(\d+\.\d+\.)(\d+)\")', t); new_v=m.group(2)+str(int(m.group(3))+1); open('python/baba/__init__.py','w',encoding='utf-8').write(t.replace(m.group(1), '__version__ = \"'+new_v+'\"')); print(f'  {m.group(2)+m.group(3)} -> {new_v}')"

for /f "tokens=2 delims==" %%v in ('findstr /r "__version__.*=.*\"" python\baba\__init__.py') do (
    set "VERSION=%%~v"
)
echo   Version: %VERSION%
echo.

echo [2/6] Cleaning old builds...
if exist dist\*.tar.gz del /q dist\*.tar.gz
if exist dist\*.whl del /q dist\*.whl
if exist build rmdir /s /q build
if exist python\baba\lib rmdir /s /q python\baba\lib
if exist python\baba\include rmdir /s /q python\baba\include
mkdir python\baba\lib
mkdir python\baba\include

echo [3/6] Building for all platforms...
call build_all.bat

echo.
echo [4/6] Checking build artifacts...
set LIB_COUNT=0
for %%f in (python\baba\lib\*.a) do set /a LIB_COUNT+=1
echo   Libraries: %LIB_COUNT%
if %LIB_COUNT%==0 (
    echo   WARNING: No libraries built!
    echo   Install Zig for cross-compilation: scoop install zig
)

echo [5/6] Building Python package...
python -m pip install --upgrade build twine -q
python -m build

echo [6/6] Uploading to PyPI...
python -m twine check dist\*.tar.gz dist\*.whl 2>nul
python -m twine upload dist\*.tar.gz dist\*.whl

echo.
echo === Done! ===
echo   Package: baba-gui==%VERSION%
echo   Install: pip install baba-gui
echo.