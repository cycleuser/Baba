@echo off
REM Baba - Build native C library for distribution
setlocal

echo === Baba Native Library Build ===

REM Detect architecture
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set ARCH=x64
) else (
    set ARCH=x86
)

echo Platform: Windows (%ARCH%)

echo [1/4] Cleaning...
if exist build rmdir /s /q build
if exist dist\lib rmdir /s /q dist\lib
if exist dist\include rmdir /s /q dist\include
mkdir dist\lib
mkdir dist\include

echo [2/4] Building for Windows...

REM Detect build system
where ninja >nul 2>&1
if %errorlevel% equ 0 (
    echo   Using Ninja...
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF
    cmake --build build --config Release
) else (
    echo   Using Visual Studio...
    cmake -B build -G "Visual Studio 17 2022" -A x64 -DBABA_BUILD_EXAMPLES=OFF -DBABA_BUILD_TESTS=OFF
    cmake --build build --config Release
)

mkdir dist\lib\windows-%ARCH%
if exist build\libbaba.a (
    copy build\libbaba.a dist\lib\windows-%ARCH%\libbaba.a
    echo   Built: dist\lib\windows-%ARCH%\libbaba.a
) else if exist build\Release\baba.lib (
    copy build\Release\baba.lib dist\lib\windows-%ARCH%\baba.lib
    echo   Built: dist\lib\windows-%ARCH%\baba.lib
) else if exist build\baba.lib (
    copy build\baba.lib dist\lib\windows-%ARCH%\baba.lib
    echo   Built: dist\lib\windows-%ARCH%\baba.lib
)

echo [3/4] Copying headers...

if not exist dist\include\core mkdir dist\include\core
if not exist dist\include\widgets mkdir dist\include\widgets
if not exist dist\include\layout mkdir dist\include\layout
if not exist dist\include\render mkdir dist\include\render
if not exist dist\include\platform mkdir dist\include\platform
if not exist dist\include\theme mkdir dist\include\theme

copy src\*.h dist\include\ >nul
copy src\core\*.h dist\include\core\ >nul
copy src\widgets\*.h dist\include\widgets\ >nul
copy src\layout\*.h dist\include\layout\ >nul
copy src\render\*.h dist\include\render\ >nul
copy src\platform\*.h dist\include\platform\ >nul
copy src\theme\*.h dist\include\theme\ >nul

echo   Headers copied to dist\include\

echo [4/4] Creating distribution package...

for /f "tokens=2 delims==" %%v in ('findstr /r "__version__.*=.*\"" python\baba\__init__.py') do (
    set "VERSION=%%~v"
)

set PACKAGE_NAME=baba-dev-%VERSION%-windows-%ARCH%

if exist dist\%PACKAGE_NAME% rmdir /s /q dist\%PACKAGE_NAME%
mkdir dist\%PACKAGE_NAME%

xcopy /e /i /y dist\include dist\%PACKAGE_NAME%\include >nul
xcopy /e /i /y dist\lib dist\%PACKAGE_NAME%\lib >nul
copy README.md dist\%PACKAGE_NAME%\ >nul 2>&1
copy LICENSE dist\%PACKAGE_NAME%\ >nul 2>&1

powershell -command "Compress-Archive -Path 'dist\%PACKAGE_NAME%' -DestinationPath 'dist\%PACKAGE_NAME%.zip' -Force"

echo   Package: dist\%PACKAGE_NAME%.zip

echo.
echo === Done! ===
echo.
echo Distribution files:
dir /b dist\*.zip 2>nul || echo   No packages created
echo.
echo Library: dist\lib\
echo Headers: dist\include\
echo.