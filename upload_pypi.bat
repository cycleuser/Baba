@echo off
REM Baba - Build and upload to PyPI
setlocal enabledelayedexpansion

echo === Baba PyPI Upload ===
echo.

REM Get version
for /f "tokens=2 delims==" %%v in ('findstr /r "__version__.*=.*\"" python\baba\__init__.py') do (
    set "VERSION=%%~v"
)

echo [1/4] Bumping patch version...
python -c "import re; t=open('python/baba/__init__.py','r',encoding='utf-8').read(); m=re.search(r'(__version__\s*=\s*\"(\d+\.\d+\.)(\d+)\")', t); new_v=m.group(2)+str(int(m.group(3))+1); open('python/baba/__init__.py','w',encoding='utf-8').write(t.replace(m.group(1), '__version__ = \"'+new_v+'\"')); print(f'  {m.group(2)+m.group(3)} -> {new_v}')"

for /f "tokens=2 delims==" %%v in ('findstr /r "__version__.*=.*\"" python\baba\__init__.py') do (
    set "VERSION=%%~v"
)
echo   Version: %VERSION%
echo.

echo [2/4] Cleaning old builds...
del /q dist\*.tar.gz 2>nul
del /q dist\*.whl 2>nul
if exist build rmdir /s /q build

echo [3/4] Building Python package...
python -m pip install --upgrade build twine -q
python -m build

echo [4/4] Uploading to PyPI...
python -m twine check dist\*.tar.gz dist\*.whl
python -m twine upload dist\*.tar.gz dist\*.whl

echo.
echo === Done! ===
echo   Package: baba-gui==%VERSION%
echo   Install: pip install baba-gui
echo.