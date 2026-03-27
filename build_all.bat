@echo off
REM Baba - Build all platforms for PyPI distribution
REM This script builds native libraries using Zig for cross-compilation
setlocal enabledelayedexpansion

echo === Baba Multi-Platform Build ===
echo.

REM Get version
for /f "tokens=2 delims==" %%v in ('findstr /r "__version__.*=.*\"" python\baba\__init__.py') do (
    set "VERSION=%%~v"
)
echo Version: %VERSION%
echo.

echo [1/5] Cleaning...
if exist build rmdir /s /q build
if exist dist\lib rmdir /s /q dist\lib
if exist dist\include rmdir /s /q dist\include
mkdir dist\lib\macos
mkdir dist\lib\linux-x64
mkdir dist\lib\windows-x64
mkdir dist\include
if exist python\baba\lib rmdir /s /q python\baba\lib
if exist python\baba\include rmdir /s /q python\baba\include
mkdir python\baba\lib
mkdir python\baba\include

echo [2/5] Copying headers...
for %%d in (core widgets layout render platform theme) do (
    if not exist "dist\include\%%d" mkdir "dist\include\%%d"
    copy "src\%%d\*.h" "dist\include\%%d\" >nul 2>&1
)
copy src\*.h dist\include\ >nul 2>&1
echo   Headers copied to dist\include\

echo [3/5] Building libraries...

REM Check for Zig
where zig >nul 2>&1
if %errorlevel% equ 0 (
    set HAS_ZIG=1
    echo   Found Zig for cross-compilation
) else (
    set HAS_ZIG=0
)

echo.
echo   === Building Windows x64 ===

REM Build Windows (native or cross-compile)
where ninja >nul 2>&1
if %errorlevel% equ 0 (
    cmake -B build\windows -G Ninja -DCMAKE_BUILD_TYPE=Release -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF
    cmake --build build\windows --config Release
    if exist build\windows\libbaba.a (
        copy build\windows\libbaba.a dist\lib\windows-x64\libbaba.a >nul
        echo     Built: dist\lib\windows-x64\libbaba.a
    )
)

if "%HAS_ZIG%"=="1" (
    echo   === Building macOS with Zig ===
    cmake -B build\macos -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="zig cc -target aarch64-macos-none" -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF -DBABA_CROSS_COMPILE=ON -DBABA_TARGET_PLATFORM=macos
    cmake --build build\macos --config Release
    if exist build\macos\libbaba.a (
        copy build\macos\libbaba.a dist\lib\macos\libbaba.a >nul
        echo     Built: dist\lib\macos\libbaba.a
    )

    echo   === Building Linux with Zig ===
    cmake -B build\linux -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="zig cc -target x86_64-linux-gnu" -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF -DBABA_CROSS_COMPILE=ON -DBABA_TARGET_PLATFORM=linux
    cmake --build build\linux --config Release
    if exist build\linux\libbaba.a (
        copy build\linux\libbaba.a dist\lib\linux-x64\libbaba.a >nul
        echo     Built: dist\lib\linux-x64\libbaba.a
    )
) else (
    echo   SKIPPED macOS and Linux: Install Zig for cross-compilation
    echo     scoop install zig
)

echo.
echo [4/5] Preparing Python package...
xcopy /e /i /y dist\include\* python\baba\include\ >nul
if exist dist\lib\macos\libbaba.a copy dist\lib\macos\libbaba.a python\baba\lib\libbaba_macos.a >nul
if exist dist\lib\linux-x64\libbaba.a copy dist\lib\linux-x64\libbaba.a python\baba\lib\libbaba_linux.a >nul
if exist dist\lib\windows-x64\libbaba.a copy dist\lib\windows-x64\libbaba.a python\baba\lib\libbaba_windows.a >nul

echo [5/5] Summary...
echo.
echo   Libraries built:
dir /b /s dist\lib\*.a 2>nul || echo     No libraries found
echo.
echo   To build all platforms, install Zig:
echo     scoop install zig
echo.

if "%1"=="--upload" (
    echo Building and uploading to PyPI...
    python -m pip install --upgrade build twine -q
    python -m build
    python -m twine upload dist\*.tar.gz dist\*.whl
)

echo === Done! ===