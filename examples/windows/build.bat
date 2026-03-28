@echo off
REM Baba GUI - Windows Example Build Script
REM 
REM This script compiles a C program using the installed baba-gui package.
REM It automatically detects Vulkan SDK installed via winget, scoop, or manually.

echo === Baba GUI Windows Example Build ===
echo.

REM Get paths from Python
for /f "delims=" %%i in ('python -c "import baba; print(baba.get_include_dir())" 2^>nul') do set "INCLUDE_PATH=%%i"
for /f "delims=" %%i in ('python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))" 2^>nul') do set "LIB_PATH=%%i"

if not defined INCLUDE_PATH (
    echo ERROR: baba-gui not found!
    echo Please install: pip install baba-gui
    exit /b 1
)

echo Baba Include: %INCLUDE_PATH%
echo Baba Library: %LIB_PATH%
echo.

REM ============================================
REM Detect Vulkan SDK Installation
REM ============================================

set "VULKAN_INCLUDE="
set "VULKAN_LIB="
set "VULKAN_SDK_FOUND="

echo Searching for Vulkan SDK...

REM Method 1: Check VULKAN_SDK environment variable (set by installer)
if defined VULKAN_SDK (
    if exist "%VULKAN_SDK%\Include\vulkan\vulkan.h" (
        set "VULKAN_INCLUDE=%VULKAN_SDK%\Include"
        set "VULKAN_LIB=%VULKAN_SDK%\Lib"
        set "VULKAN_SDK_FOUND=env"
        echo   [OK] Found via VULKAN_SDK env: %VULKAN_SDK%
    )
)

REM Method 2: Check winget default installation (C:\VulkanSDK\)
if not defined VULKAN_INCLUDE (
    if exist "C:\VulkanSDK" (
        for /f "tokens=*" %%d in ('dir /b /ad "C:\VulkanSDK" 2^>nul ^| findstr /r "^[0-9]"') do (
            if exist "C:\VulkanSDK\%%d\Include\vulkan\vulkan.h" (
                set "VULKAN_INCLUDE=C:\VulkanSDK\%%d\Include"
                set "VULKAN_LIB=C:\VulkanSDK\%%d\Lib"
                set "VULKAN_SDK_FOUND=winget"
                set "VULKAN_SDK=C:\VulkanSDK\%%d"
                echo   [OK] Found in C:\VulkanSDK\%%d
            )
        )
    )
)

REM Method 3: Check scoop installation
if not defined VULKAN_INCLUDE (
    set "SCOOP_PATH=%USERPROFILE%\scoop\apps\vulkan-sdk"
    if exist "%SCOOP_PATH%" (
        for /f "tokens=*" %%d in ('dir /b /ad "%SCOOP_PATH%" 2^>nul ^| findstr /r "^[0-9]"') do (
            if exist "%SCOOP_PATH%\%%d\Include\vulkan\vulkan.h" (
                set "VULKAN_INCLUDE=%SCOOP_PATH%\%%d\Include"
                set "VULKAN_LIB=%SCOOP_PATH%\%%d\Lib"
                set "VULKAN_SDK_FOUND=scoop"
                echo   [OK] Found via scoop: %SCOOP_PATH%\%%d
            )
        )
    )
)

REM Method 4: Check scoop current symlink
if not defined VULKAN_INCLUDE (
    if exist "%USERPROFILE%\scoop\apps\vulkan-sdk\current\Include\vulkan\vulkan.h" (
        set "VULKAN_INCLUDE=%USERPROFILE%\scoop\apps\vulkan-sdk\current\Include"
        set "VULKAN_LIB=%USERPROFILE%\scoop\apps\vulkan-sdk\current\Lib"
        set "VULKAN_SDK_FOUND=scoop"
        echo   [OK] Found via scoop current
    )
)

REM Method 5: Search in Program Files
if not defined VULKAN_INCLUDE (
    if exist "C:\Program Files\VulkanSDK" (
        for /f "tokens=*" %%d in ('dir /b /ad "C:\Program Files\VulkanSDK" 2^>nul ^| findstr /r "^[0-9]"') do (
            if exist "C:\Program Files\VulkanSDK\%%d\Include\vulkan\vulkan.h" (
                set "VULKAN_INCLUDE=C:\Program Files\VulkanSDK\%%d\Include"
                set "VULKAN_LIB=C:\Program Files\VulkanSDK\%%d\Lib"
                set "VULKAN_SDK_FOUND=programfiles"
                echo   [OK] Found in Program Files: C:\Program Files\VulkanSDK\%%d
            )
        )
    )
)

echo.

REM ============================================
REM Verify Vulkan SDK Found
REM ============================================

if not defined VULKAN_INCLUDE (
    echo ============================================
    echo ERROR: Vulkan SDK not found!
    echo ============================================
    echo.
    echo Searched locations:
    echo   - VULKAN_SDK environment variable
    echo   - C:\VulkanSDK\ (winget default)
    echo   - %USERPROFILE%\scoop\apps\vulkan-sdk\ (scoop)
    echo   - C:\Program Files\VulkanSDK\
    echo.
    echo Please install Vulkan SDK:
    echo.
    echo   winget install KhronosGroup.VulkanSDK
    echo.
    echo After installation:
    echo   1. Close this terminal
    echo   2. Open a new terminal
    echo   3. Run this script again
    echo.
    echo Or set VULKAN_SDK manually:
    echo   set VULKAN_SDK=C:\VulkanSDK\1.3.xxx.0
    echo.
    exit /b 1
)

echo Vulkan SDK: Found via %VULKAN_SDK_FOUND%
echo Vulkan Include: %VULKAN_INCLUDE%
echo Vulkan Lib: %VULKAN_LIB%
echo.

REM Set VULKAN_SDK if not already set (for CMake)
if not defined VULKAN_SDK (
    for %%i in ("%VULKAN_INCLUDE%") do set "VULKAN_SDK=%%~dpi"
    set "VULKAN_SDK=%VULKAN_SDK:~0,-1%"
)

REM ============================================
REM Check for GCC
REM ============================================

where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: GCC not found!
    echo.
    echo Install MinGW via MSYS2:
    echo   winget install MSYS2.MSYS2
    echo.
    echo Then in MSYS2 MINGW64 terminal:
    echo   pacman -S mingw-w64-x86_64-gcc
    echo.
    exit /b 1
)

echo GCC found:
gcc --version | findstr /n "^" | findstr "1:"
echo.

REM ============================================
REM Build
REM ============================================

echo ============================================
echo Building...
echo ============================================
echo.

gcc main.c ^
    -I"%INCLUDE_PATH%" ^
    -I"%VULKAN_INCLUDE%" ^
    -L"%LIB_PATH%" ^
    -L"%VULKAN_LIB%" ^
    -lbaba_windows ^
    -lvulkan-1 ^
    -luser32 ^
    -lgdi32 ^
    -o app.exe

if %errorlevel% equ 0 (
    echo.
    echo ============================================
    echo Build successful!
    echo ============================================
    echo.
    echo Executable: app.exe
    echo.
    echo To run:
    echo   .\app.exe
    echo.
) else (
    echo.
    echo ============================================
    echo Build FAILED!
    echo ============================================
    echo.
    echo Troubleshooting:
    echo   1. Make sure baba-gui is installed:
    echo      pip install baba-gui
    echo.
    echo   2. Make sure Vulkan SDK is installed:
    echo      winget install KhronosGroup.VulkanSDK
    echo.
    echo   3. Make sure GCC is installed:
    echo      winget install MSYS2.MSYS2
    echo      (then: pacman -S mingw-w64-x86_64-gcc)
    echo.
)

echo.