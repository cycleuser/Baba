from ._core import App, Window, Widget, Button, Label, TextBox, Color, Rect, Vec2
from pathlib import Path
import sys

__version__ = "0.1.1"
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
]

def get_include_dir() -> Path:
    """Get the directory containing C header files for Baba GUI development."""
    return Path(__file__).parent / "include"

def get_lib_dir() -> Path:
    """Get the directory containing compiled library files."""
    return Path(__file__).parent / "lib"

def get_lib_path() -> Path:
    """Get the path to the compiled library for the current platform."""
    lib_dir = get_lib_dir()
    if sys.platform == "darwin":
        return lib_dir / "libbaba_macos.a"
    elif sys.platform == "win32":
        if (lib_dir / "libbaba_windows.a").exists():
            return lib_dir / "libbaba_windows.a"
        return lib_dir / "baba_windows.lib"
    else:
        return lib_dir / "libbaba_linux.a"
