@echo off
REM Baba GUI - Windows Example Build Script
REM 
REM This script compiles a C program using the installed baba-gui package.

echo === Baba GUI Windows Example Build ===
echo.

REM Get paths from Python
for /f "delims=" %%i in ('python -c "import baba; print(baba.get_include_dir())"') do set "INCLUDE_PATH=%%i"
for /f "delims=" %%i in ('python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))"') do set "LIB_PATH=%%i"

echo Include Path: %INCLUDE_PATH%
echo Library Path: %LIB_PATH%
echo.

REM Check if gcc is available
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: GCC not found. Please install MinGW-w64:
    echo   scoop install mingw
    exit /b 1
)

REM Check if Vulkan SDK is available
where glslc >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: Vulkan SDK may not be in PATH.
    echo If build fails, install Vulkan SDK from: https://vulkan.lunarg.com/
    echo.
)

echo Building...
gcc main.c ^
    -I"%INCLUDE_PATH%" ^
    -L"%LIB_PATH%" ^
    -lbaba_windows ^
    -lvulkan ^
    -luser32 ^
    -lgdi32 ^
    -o app.exe

if %errorlevel% equ 0 (
    echo.
    echo Build successful! Executable: app.exe
    echo.
    echo To run:
    echo   app.exe
) else (
    echo.
    echo Build failed!
    echo.
    echo Troubleshooting:
    echo 1. Make sure Vulkan SDK is installed
    echo 2. Make sure baba-gui is installed: pip install baba-gui
    echo 3. Make sure MinGW-w64 is installed: scoop install mingw
)

echo.