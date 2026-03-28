@echo off
REM Baba - Build native library for current platform
REM For cross-platform builds, use GitHub Actions

echo ============================================
echo   Baba Native Build - Windows
echo ============================================
echo.

REM ============================================
REM Step 1: Check/Install MSYS2 + MinGW
REM ============================================

echo [1/4] Checking compiler...

set "GCC_PATH=C:\msys64\mingw64\bin"
if exist "%GCC_PATH%\gcc.exe" (
    set "PATH=%GCC_PATH%;%PATH%"
    goto :gcc_found
)

echo Installing MSYS2 via winget...
winget install MSYS2.MSYS2 --accept-source-agreements --accept-package-agreements

REM Install GCC in MSYS2
if exist "C:\msys64\usr\bin\bash.exe" (
    echo Installing GCC...
    C:\msys64\usr\bin\bash.exe -lc "pacman -Sy --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-vulkan"
    set "PATH=%GCC_PATH%;%PATH%"
)

:gcc_found
gcc --version 2>nul | findstr "gcc"
if %errorlevel% neq 0 (
    echo ERROR: GCC installation failed!
    exit /b 1
)
echo   GCC OK
echo.

REM ============================================
REM Step 2: Check/Install Vulkan SDK
REM ============================================

echo [2/4] Checking Vulkan SDK...

set "VULKAN_SDK="
if exist "C:\VulkanSDK" (
    for /f "tokens=*" %%d in ('dir /b /ad "C:\VulkanSDK" 2^>nul ^| findstr /r "^[0-9]"') do (
        if exist "C:\VulkanSDK\%%d\Include\vulkan\vulkan.h" (
            set "VULKAN_SDK=C:\VulkanSDK\%%d"
        )
    )
)

if not defined VULKAN_SDK (
    echo Installing Vulkan SDK via winget...
    winget install KhronosGroup.VulkanSDK --accept-source-agreements --accept-package-agreements
    
    REM Refresh
    for /f "tokens=*" %%d in ('dir /b /ad "C:\VulkanSDK" 2^>nul ^| findstr /r "^[0-9]"') do (
        if exist "C:\VulkanSDK\%%d\Include\vulkan\vulkan.h" (
            set "VULKAN_SDK=C:\VulkanSDK\%%d"
        )
    )
)

if defined VULKAN_SDK (
    echo   Vulkan SDK: %VULKAN_SDK%
    echo   Vulkan OK
) else (
    echo   ERROR: Vulkan SDK not found!
    exit /b 1
)
echo.

REM ============================================
REM Step 3: Build Library
REM ============================================

echo [3/4] Building...

REM Create directories
mkdir python\baba\lib 2>nul
mkdir python\baba\include 2>nul
mkdir build 2>nul

REM Build with CMake
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF
if %errorlevel% neq 0 (
    cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF
)

cmake --build build --config Release

if not exist build\libbaba.a (
    echo Build FAILED!
    exit /b 1
)

REM Copy library
copy /Y build\libbaba.a python\baba\lib\libbaba_windows.a
echo.
echo   Built: python\baba\lib\libbaba_windows.a
echo.

REM ============================================
REM Step 4: Copy Headers
REM ============================================

echo [4/4] Copying headers...

copy /Y src\*.h python\baba\include\ >nul 2>&1
for %%d in (core widgets layout render platform theme) do (
    mkdir python\baba\include\%%d 2>nul
    copy /Y src\%%d\*.h python\baba\include\%%d\ >nul 2>&1
)

echo   Headers copied
echo.

REM ============================================
REM Summary
REM ============================================

echo ============================================
echo   Build Complete!
echo ============================================
echo.
echo   Output:
dir /b python\baba\lib\*.a
echo.
echo   To build Python package:
echo     pip install build
echo     python -m build
echo.
echo   To publish:
echo     twine upload dist\*
echo.