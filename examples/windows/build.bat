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
REM Step 2: Check and Install CMake
REM ============================================

echo [2/4] Checking CMake...

where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo CMake not found. Installing via winget...
    winget install Kitware.CMake --accept-source-agreements --accept-package-agreements
    set "PATH=C:\Program Files\CMake\bin;%PATH%"
)

cmake --version 2>nul | findstr "cmake"
echo   CMake OK
echo.

REM ============================================
REM Step 3: Check and Install Vulkan SDK
REM ============================================

echo [3/4] Checking Vulkan SDK...

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
    echo   Vulkan Include: %VULKAN_INCLUDE%
    echo   Vulkan OK
) else (
    echo   ERROR: Vulkan SDK installation failed!
    echo   Please install manually: winget install KhronosGroup.VulkanSDK
    exit /b 1
)
echo.

REM ============================================
REM Step 4: Check and Install baba-gui
REM ============================================

echo [4/4] Checking baba-gui Python package...

for /f "delims=" %%i in ('python -c "import baba; print(baba.get_include_dir())" 2^>nul') do set "INCLUDE_PATH=%%i"
for /f "delims=" %%i in ('python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))" 2^>nul') do set "LIB_PATH=%%i"

if not defined INCLUDE_PATH (
    echo baba-gui not found. Installing via pip...
    pip install baba-gui --quiet
    
    for /f "delims=" %%i in ('python -c "import baba; print(baba.get_include_dir())" 2^>nul') do set "INCLUDE_PATH=%%i"
    for /f "delims=" %%i in ('python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))" 2^>nul') do set "LIB_PATH=%%i"
)

if defined INCLUDE_PATH (
    echo   Baba Include: %INCLUDE_PATH%
    echo   Baba Library: %LIB_PATH%
    echo   baba-gui OK
) else (
    echo   ERROR: baba-gui installation failed!
    echo   Please install manually: pip install baba-gui
    exit /b 1
)
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
)

echo.