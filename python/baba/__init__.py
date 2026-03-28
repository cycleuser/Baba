from ._core import App, Window, Widget, Button, Label, TextBox, Color, Rect, Vec2
from pathlib import Path
import sys
import platform

__version__ = "0.1.2"
__all__ = [
    "App",
    "Window",
    "Widget",
    "Button",
    "Label",
    "TextBox",
    "Color",
    "Rect",
    "Vec2",
    "get_include_dir",
    "get_lib_dir",
    "get_lib_path",
    "get_platform_info",
    "get_compile_command",
]


def get_platform_info():
    """Get current platform information."""
    system = platform.system().lower()
    machine = platform.machine().lower()

    if system == "darwin":
        return "macos", machine
    elif system == "windows":
        return "windows", "x64" if machine in ("amd64", "x86_64") else "x86"
    elif system == "linux":
        return "linux", machine
    else:
        return system, machine


def get_include_dir() -> Path:
    """Get the directory containing C header files."""
    return Path(__file__).parent / "include"


def get_lib_dir() -> Path:
    """Get the directory containing compiled library files."""
    return Path(__file__).parent / "lib"


def get_lib_path() -> Path:
    """Get the path to the compiled library for the current platform."""
    lib_dir = get_lib_dir()
    system, arch = get_platform_info()

    if system == "macos":
        candidates = [
            lib_dir / "libbaba_macos.a",
            lib_dir / "libbaba.a",
        ]
    elif system == "windows":
        candidates = [
            lib_dir / "libbaba_windows.a",
            lib_dir / "baba_windows.lib",
            lib_dir / "libbaba.a",
            lib_dir / "baba.lib",
        ]
    else:
        candidates = [
            lib_dir / "libbaba_linux.a",
            lib_dir / "libbaba.a",
        ]

    for path in candidates:
        if path.exists():
            return path

    available = list(lib_dir.glob("*")) if lib_dir.exists() else []
    available_str = "\n    ".join(str(p.name) for p in available) if available else "(empty)"

    raise RuntimeError(
        f"Baba library not found for {system} ({arch})!\n"
        f"\n"
        f"  Library directory: {lib_dir}\n"
        f"  Available files:\n    {available_str}\n"
        f"\n"
        f"  This package may not include pre-built libraries for your platform.\n"
        f"\n"
        f"  Solutions:\n"
        f"    1. pip install baba-gui --force-reinstall --no-cache-dir\n"
        f"    2. Build from source: https://github.com/cycleuser/Baba\n"
    )


def get_compile_command(source: str = "main.c", output: str = "app") -> str:
    """Get the compile command for a C source file."""
    include_dir = get_include_dir()
    lib_path = get_lib_path()
    lib_dir = lib_path.parent
    system, _ = get_platform_info()

    lib_name = lib_path.stem
    if lib_name.startswith("lib"):
        lib_name = lib_name[3:]

    if system == "macos":
        return f'gcc {source} -I"{include_dir}" -L"{lib_dir}" -l{lib_name} -framework Cocoa -framework Metal -lvulkan -o {output}'
    elif system == "windows":
        return f'gcc {source} -I"{include_dir}" -L"{lib_dir}" -l{lib_name} -lvulkan-1 -luser32 -lgdi32 -o {output}.exe'
    else:
        return (
            f'gcc {source} -I"{include_dir}" -L"{lib_dir}" -l{lib_name} -lvulkan -lX11 -o {output}'
        )
