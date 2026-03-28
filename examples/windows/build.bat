@echo off
REM Baba GUI - Windows Example Build Script
REM 
REM This script automatically installs all dependencies via winget if missing.
REM No external dependencies required - just run it!

echo ============================================
echo   Baba GUI - Windows Example Build
echo ============================================
echo.

REM ============================================
REM Step 1: Check and Install MSYS2 (for GCC)
REM ============================================

echo [1/4] Checking GCC compiler...

where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo GCC not found. Installing MSYS2 via winget...
    winget install MSYS2.MSYS2 --accept-source-agreements --accept-package-agreements
    
    echo.
    echo MSYS2 installed. Setting up GCC...
    set "PATH=C:\msys64\mingw64\bin;%PATH%"
    
    where gcc >nul 2>&1
    if %errorlevel% neq 0 (
        echo Installing GCC in MSYS2...
        C:\msys64\usr\bin\bash.exe -lc "pacman -Sy --noconfirm mingw-w64-x86_64-gcc"
        set "PATH=C:\msys64\mingw64\bin;%PATH%"
    )
)

gcc --version 2>nul | findstr "gcc"
echo   GCC OK
echo.

REM ============================================
REM Step 2: Check and Install Vulkan SDK
REM ============================================

echo [2/4] Checking Vulkan SDK...

set "VULKAN_INCLUDE="
set "VULKAN_LIB="

REM Check environment variable
if defined VULKAN_SDK (
    if exist "%VULKAN_SDK%\Include\vulkan\vulkan.h" (
        set "VULKAN_INCLUDE=%VULKAN_SDK%\Include"
        set "VULKAN_LIB=%VULKAN_SDK%\Lib"
    )
    if exist "%VULKAN_SDK%\Include\vulkan.h" (
        set "VULKAN_INCLUDE=%VULKAN_SDK%\Include"
        set "VULKAN_LIB=%VULKAN_SDK%\Lib"
    )
)

REM Check C:\VulkanSDK\
if not defined VULKAN_INCLUDE (
    if exist "C:\VulkanSDK" (
        for /f "tokens=*" %%d in ('dir /b /ad "C:\VulkanSDK" 2^>nul ^| findstr /r "^[0-9]"') do (
            if exist "C:\VulkanSDK\%%d\Include\vulkan\vulkan.h" (
                set "VULKAN_INCLUDE=C:\VulkanSDK\%%d\Include"
                set "VULKAN_LIB=C:\VulkanSDK\%%d\Lib"
                set "VULKAN_SDK=C:\VulkanSDK\%%d"
            )
            if exist "C:\VulkanSDK\%%d\Include\vulkan.h" (
                set "VULKAN_INCLUDE=C:\VulkanSDK\%%d\Include"
                set "VULKAN_LIB=C:\VulkanSDK\%%d\Lib"
                set "VULKAN_SDK=C:\VulkanSDK\%%d"
            )
        )
    )
)

if not defined VULKAN_INCLUDE (
    echo Vulkan SDK not found. Installing via winget...
    winget install KhronosGroup.VulkanSDK --accept-source-agreements --accept-package-agreements
    
    REM Refresh environment
    for /f "tokens=*" %%d in ('dir /b /ad "C:\VulkanSDK" 2^>nul ^| findstr /r "^[0-9]"') do (
        if exist "C:\VulkanSDK\%%d\Include\vulkan\vulkan.h" (
            set "VULKAN_INCLUDE=C:\VulkanSDK\%%d\Include"
            set "VULKAN_LIB=C:\VulkanSDK\%%d\Lib"
            set "VULKAN_SDK=C:\VulkanSDK\%%d"
        )
        if exist "C:\VulkanSDK\%%d\Include\vulkan.h" (
            set "VULKAN_INCLUDE=C:\VulkanSDK\%%d\Include"
            set "VULKAN_LIB=C:\VulkanSDK\%%d\Lib"
            set "VULKAN_SDK=C:\VulkanSDK\%%d"
        )
    )
)

if defined VULKAN_INCLUDE (
    echo   Vulkan SDK: %VULKAN_SDK%
    echo   Vulkan OK
) else (
    echo   ERROR: Vulkan SDK installation failed!
    echo   Please install manually: winget install KhronosGroup.VulkanSDK
    exit /b 1
)
echo.

REM ============================================
REM Step 3: Check and Install baba-gui
REM ============================================

echo [3/4] Checking baba-gui Python package...

python -c "import baba" 2>nul
if %errorlevel% neq 0 (
    echo baba-gui not found. Installing via pip...
    pip install baba-gui --quiet
)

REM Get paths
for /f "delims=" %%i in ('python -c "import baba; print(baba.get_include_dir())" 2^>nul') do set "INCLUDE_PATH=%%i"
for /f "delims=" %%i in ('python -c "import baba; print(baba.get_lib_dir())" 2^>nul') do set "LIB_DIR=%%i"

if not defined INCLUDE_PATH (
    echo   ERROR: baba-gui installation failed!
    echo   Please install manually: pip install baba-gui
    exit /b 1
)

echo   Baba Include: %INCLUDE_PATH%
echo   Baba Lib Dir: %LIB_DIR%

REM Find the library file
set "BABA_LIB="
if exist "%LIB_DIR%\libbaba_windows.a" (
    set "BABA_LIB=libbaba_windows"
    echo   Using: libbaba_windows.a
) else if exist "%LIB_DIR%\baba_windows.lib" (
    set "BABA_LIB=baba_windows"
    echo   Using: baba_windows.lib
) else if exist "%LIB_DIR%\libbaba.a" (
    set "BABA_LIB=baba"
    echo   Using: libbaba.a
) else if exist "%LIB_DIR%\baba.lib" (
    set "BABA_LIB=baba"
    echo   Using: baba.lib
) else (
    echo   ERROR: Baba library not found in %LIB_DIR%
    echo   Available files:
    dir /b "%LIB_DIR%\*.a" "%LIB_DIR%\*.lib" 2>nul
    echo.
    echo   The installed baba-gui package may not include Windows libraries.
    echo   Solutions:
    echo     1. Build from source: git clone https://github.com/cycleuser/Baba
    echo     2. Run: build_all.bat
    echo     3. pip install -e .
    exit /b 1
)
echo.

REM ============================================
REM Step 4: Build
REM ============================================

echo [4/4] Building...
echo.

gcc main.c ^
    -I"%INCLUDE_PATH%" ^
    -I"%VULKAN_INCLUDE%" ^
    -L"%LIB_DIR%" ^
    -L"%VULKAN_LIB%" ^
    -l%BABA_LIB% ^
    -lvulkan-1 ^
    -luser32 ^
    -lgdi32 ^
    -o app.exe

if %errorlevel% equ 0 (
    echo.
    echo ============================================
    echo   Build successful!
    echo ============================================
    echo.
    echo   Executable: app.exe
    echo   To run: .\app.exe
    echo.
) else (
    echo.
    echo Build FAILED! Check error messages above.
    echo.
    echo Troubleshooting:
    echo   1. Make sure baba-gui includes Windows library:
    echo      pip install baba-gui --force-reinstall --no-cache-dir
    echo.
    echo   2. Or build from source:
    echo      git clone https://github.com/cycleuser/Baba
    echo      cd Baba
    echo      build_all.bat
    echo      pip install -e .
    echo.
)

echo.