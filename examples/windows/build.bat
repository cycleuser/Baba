@echo off
REM Baba GUI - Windows Example Build Script

echo === Baba GUI Windows Example Build ===
echo.

REM Get paths from Python
for /f "delims=" %%i in ('python -c "import baba; print(baba.get_include_dir())"') do set "INCLUDE_PATH=%%i"
for /f "delims=" %%i in ('python -c "import baba; import os; print(os.path.dirname(baba.get_lib_path()))"') do set "LIB_PATH=%%i"

echo Baba Include: %INCLUDE_PATH%
echo Baba Library: %LIB_PATH%
echo.

REM Find Vulkan SDK - check multiple possible locations
set "VULKAN_INCLUDE="
set "VULKAN_LIB="

REM Method 1: Check VULKAN_SDK environment variable
if defined VULKAN_SDK (
    if exist "%VULKAN_SDK%\Include" (
        set "VULKAN_INCLUDE=%VULKAN_SDK%\Include"
        set "VULKAN_LIB=%VULKAN_SDK%\Lib"
        echo Vulkan SDK (env): %VULKAN_SDK%
    )
)

REM Method 2: Check C:\VulkanSDK\
if not defined VULKAN_INCLUDE (
    if exist "C:\VulkanSDK" (
        for /f "tokens=*" %%d in ('dir /b /ad "C:\VulkanSDK" 2^>nul ^| findstr /r "^[0-9]"') do (
            if exist "C:\VulkanSDK\%%d\Include" (
                set "VULKAN_INCLUDE=C:\VulkanSDK\%%d\Include"
                set "VULKAN_LIB=C:\VulkanSDK\%%d\Lib"
                set "VULKAN_SDK=C:\VulkanSDK\%%d"
                echo Vulkan SDK (C:\VulkanSDK): C:\VulkanSDK\%%d
            )
        )
    )
)

REM Method 3: Check scoop installation
if not defined VULKAN_INCLUDE (
    if exist "%USERPROFILE%\scoop\apps\vulkan-sdk" (
        for /f "tokens=*" %%d in ('dir /b /ad "%USERPROFILE%\scoop\apps\vulkan-sdk" 2^>nul ^| findstr /r "^[0-9]"') do (
            if exist "%USERPROFILE%\scoop\apps\vulkan-sdk\%%d\Include" (
                set "VULKAN_INCLUDE=%USERPROFILE%\scoop\apps\vulkan-sdk\%%d\Include"
                set "VULKAN_LIB=%USERPROFILE%\scoop\apps\vulkan-sdk\%%d\Lib"
                echo Vulkan SDK (scoop): %USERPROFILE%\scoop\apps\vulkan-sdk\%%d
            )
        )
    )
)

REM Method 4: Check current directory scoop
if not defined VULKAN_INCLUDE (
    if exist "C:\Users\%USERNAME%\scoop\apps\vulkan-sdk\current\Include" (
        set "VULKAN_INCLUDE=C:\Users\%USERNAME%\scoop\apps\vulkan-sdk\current\Include"
        set "VULKAN_LIB=C:\Users\%USERNAME%\scoop\apps\vulkan-sdk\current\Lib"
        echo Vulkan SDK (scoop current): C:\Users\%USERNAME%\scoop\apps\vulkan-sdk\current
    )
)

if not defined VULKAN_INCLUDE (
    echo ERROR: Vulkan SDK headers not found!
    echo.
    echo Searched locations:
    echo   - VULKAN_SDK environment variable
    echo   - C:\VulkanSDK\
    echo   - %USERPROFILE%\scoop\apps\vulkan-sdk\
    echo.
    echo Please install Vulkan SDK:
    echo   winget install KhronosGroup.VulkanSDK
    echo   OR
    echo   scoop install vulkan-sdk
    echo.
    echo Then restart your terminal.
    exit /b 1
)

echo Vulkan Include: %VULKAN_INCLUDE%
echo Vulkan Lib: %VULKAN_LIB%
echo.

REM Check GCC
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: GCC not found!
    echo Install MinGW: scoop install mingw
    exit /b 1
)

echo Building...
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
    echo ========================================
    echo Build successful!
    echo Executable: app.exe
    echo ========================================
    echo.
    echo Run: .\app.exe
) else (
    echo.
    echo Build FAILED!
    echo.
    echo Check that all paths are correct above.
)

echo.