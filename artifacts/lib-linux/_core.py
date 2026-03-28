from __future__ import annotations
from typing import Callable, Optional, Any, List, Tuple
from contextlib import contextmanager
from ctypes import CFUNCTYPE, c_void_p

from ._ffi.ffi_build import ffi, load_baba

_lib = None


def _get_lib():
    global _lib
    if _lib is None:
        _lib = load_baba()
    return _lib


class Vec2:
    def __init__(self, x: float = 0.0, y: float = 0.0):
        self.x = x
        self.y = y

    def __repr__(self) -> str:
        return f"Vec2({self.x}, {self.y})"

    def to_c(self):
        return ffi.new("BabaVec2*", (self.x, self.y))[0]

    @classmethod
    def from_c(cls, c_vec) -> Vec2:
        return cls(c_vec.x, c_vec.y)


class Rect:
    def __init__(self, x: float = 0, y: float = 0, width: float = 0, height: float = 0):
        self.x = x
        self.y = y
        self.width = width
        self.height = height

    def __repr__(self) -> str:
        return f"Rect({self.x}, {self.y}, {self.width}, {self.height})"

    @property
    def left(self) -> float:
        return self.x

    @property
    def top(self) -> float:
        return self.y

    @property
    def right(self) -> float:
        return self.x + self.width

    @property
    def bottom(self) -> float:
        return self.y + self.height

    def to_c(self):
        return ffi.new("BabaRect*", (self.x, self.y, self.width, self.height))[0]

    @classmethod
    def from_c(cls, c_rect) -> Rect:
        return cls(c_rect.x, c_rect.y, c_rect.width, c_rect.height)


class Color:
    def __init__(self, r: float = 0.0, g: float = 0.0, b: float = 0.0, a: float = 1.0):
        self.r = min(1.0, max(0.0, r))
        self.g = min(1.0, max(0.0, g))
        self.b = min(1.0, max(0.0, b))
        self.a = min(1.0, max(0.0, a))

    def __repr__(self) -> str:
        return f"Color({self.r:.2f}, {self.g:.2f}, {self.b:.2f}, {self.a:.2f})"

    @classmethod
    def from_hex(cls, hex_color: str) -> Color:
        hex_color = hex_color.lstrip("#")
        if len(hex_color) == 6:
            r = int(hex_color[0:2], 16) / 255.0
            g = int(hex_color[2:4], 16) / 255.0
            b = int(hex_color[4:6], 16) / 255.0
            return cls(r, g, b, 1.0)
        elif len(hex_color) == 8:
            r = int(hex_color[0:2], 16) / 255.0
            g = int(hex_color[2:4], 16) / 255.0
            b = int(hex_color[4:6], 16) / 255.0
            a = int(hex_color[6:8], 16) / 255.0
            return cls(r, g, b, a)
        raise ValueError(f"Invalid hex color: {hex_color}")

    @classmethod
    def from_rgb(cls, r: int, g: int, b: int, a: int = 255) -> Color:
        return cls(r / 255.0, g / 255.0, b / 255.0, a / 255.0)

    def to_c(self):
        return ffi.new("BabaColor*", (self.r, self.g, self.b, self.a))[0]

    @classmethod
    def from_c(cls, c_color) -> Color:
        return cls(c_color.r, c_color.g, c_color.b, c_color.a)

    WHITE = lambda: Color(1, 1, 1, 1)
    BLACK = lambda: Color(0, 0, 0, 1)
    RED = lambda: Color(1, 0, 0, 1)
    GREEN = lambda: Color(0, 1, 0, 1)
    BLUE = lambda: Color(0, 0, 1, 1)
    TRANSPARENT = lambda: Color(0, 0, 0, 0)


class Widget:
    def __init__(self, ptr=None):
        self._ptr = ptr
        self._children: List[Widget] = []
        self._parent: Optional[Widget] = None
        self._callbacks: List[Any] = []

    @property
    def ptr(self):
        return self._ptr

    @property
    def bounds(self) -> Rect:
        if not self._ptr:
            return Rect()
        lib = _get_lib()
        c_rect = lib.baba_widget_get_bounds(self._ptr)
        return Rect.from_c(c_rect)

    @bounds.setter
    def bounds(self, rect: Rect):
        if self._ptr:
            lib = _get_lib()
            lib.baba_widget_set_bounds(self._ptr, rect.to_c())

    @property
    def visible(self) -> bool:
        if not self._ptr:
            return False
        lib = _get_lib()
        return lib.baba_widget_is_visible(self._ptr)

    @visible.setter
    def visible(self, value: bool):
        if self._ptr:
            lib = _get_lib()
            lib.baba_widget_set_visible(self._ptr, value)

    def add_child(self, child: Widget):
        if self._ptr and child._ptr:
            lib = _get_lib()
            lib.baba_widget_add_child(self._ptr, child._ptr)
            self._children.append(child)
            child._parent = self

    def remove_child(self, child: Widget):
        if self._ptr and child._ptr:
            lib = _get_lib()
            lib.baba_widget_remove_child(self._ptr, child._ptr)
            if child in self._children:
                self._children.remove(child)
            child._parent = None

    def invalidate(self):
        if self._ptr:
            lib = _get_lib()
            lib.baba_widget_invalidate(self._ptr)

    def destroy(self):
        if self._ptr:
            lib = _get_lib()
            lib.baba_widget_destroy(self._ptr)
            self._ptr = None


class Button(Widget):
    _click_callbacks = {}

    def __init__(self, text: str = ""):
        super().__init__()
        lib = _get_lib()
        self._ptr = lib.baba_button_create(text.encode("utf-8"))
        self._text = text
        self._click_handler: Optional[Callable[[Button], None]] = None
        self._id = id(self)

    @property
    def text(self) -> str:
        return self._text

    @text.setter
    def text(self, value: str):
        self._text = value
        if self._ptr:
            lib = _get_lib()
            lib.baba_button_set_text(self._ptr, value.encode("utf-8"))

    def on_click(self, callback: Callable[[Button], None]):
        self._click_handler = callback
        Button._click_callbacks[self._id] = (self, callback)

        lib = _get_lib()

        @ffi.callback("void(BabaWidget*, void*)")
        def _callback(widget_ptr, user_data):
            if self._click_handler:
                self._click_handler(self)

        self._callbacks.append(_callback)
        lib.baba_button_set_on_click(self._ptr, _callback, ffi.NULL)

    def destroy(self):
        if self._id in Button._click_callbacks:
            del Button._click_callbacks[self._id]
        super().destroy()


class Label(Widget):
    def __init__(self, text: str = ""):
        super().__init__()
        lib = _get_lib()
        self._ptr = lib.baba_label_create(text.encode("utf-8"))
        self._text = text
        self._color = Color.BLACK()
        self._font_size = 14.0

    @property
    def text(self) -> str:
        return self._text

    @text.setter
    def text(self, value: str):
        self._text = value
        if self._ptr:
            lib = _get_lib()
            lib.baba_label_set_text(self._ptr, value.encode("utf-8"))

    @property
    def color(self) -> Color:
        return self._color

    @color.setter
    def color(self, value: Color):
        self._color = value
        if self._ptr:
            lib = _get_lib()
            lib.baba_label_set_color(self._ptr, value.to_c())

    @property
    def font_size(self) -> float:
        return self._font_size

    @font_size.setter
    def font_size(self, value: float):
        self._font_size = value
        if self._ptr:
            lib = _get_lib()
            lib.baba_label_set_font_size(self._ptr, value)


class TextBox(Widget):
    def __init__(self, placeholder: str = ""):
        super().__init__()
        lib = _get_lib()
        self._ptr = lib.baba_textbox_create(placeholder.encode("utf-8"))
        self._placeholder = placeholder
        self._password_mode = False

    @property
    def text(self) -> str:
        if not self._ptr:
            return ""
        lib = _get_lib()
        c_text = lib.baba_textbox_get_text(self._ptr)
        return ffi.string(c_text).decode("utf-8")

    @text.setter
    def text(self, value: str):
        if self._ptr:
            lib = _get_lib()
            lib.baba_textbox_set_text(self._ptr, value.encode("utf-8"))

    @property
    def password_mode(self) -> bool:
        return self._password_mode

    @password_mode.setter
    def password_mode(self, value: bool):
        self._password_mode = value
        if self._ptr:
            lib = _get_lib()
            lib.baba_textbox_set_password_mode(self._ptr, value)


class Window:
    def __init__(self, app: App, title: str, width: int = 800, height: int = 600):
        self._app = app
        lib = _get_lib()
        self._ptr = lib.baba_window_create(app._ptr, title.encode("utf-8"), width, height)
        self._title = title
        self._root: Optional[Widget] = None

    @property
    def title(self) -> str:
        return self._title

    @title.setter
    def title(self, value: str):
        self._title = value
        if self._ptr:
            lib = _get_lib()
            lib.baba_window_set_title(self._ptr, value.encode("utf-8"))

    @property
    def size(self) -> Tuple[int, int]:
        if not self._ptr:
            return (0, 0)
        lib = _get_lib()
        w = ffi.new("int*")
        h = ffi.new("int*")
        lib.baba_window_get_size(self._ptr, w, h)
        return (w[0], h[0])

    @size.setter
    def size(self, value: Tuple[int, int]):
        if self._ptr:
            lib = _get_lib()
            lib.baba_window_set_size(self._ptr, value[0], value[1])

    @property
    def root(self) -> Widget:
        if self._root is None:
            lib = _get_lib()
            ptr = lib.baba_window_get_root(self._ptr)
            self._root = Widget(ptr)
        return self._root

    def show(self):
        if self._ptr:
            lib = _get_lib()
            lib.baba_window_show(self._ptr)

    def hide(self):
        if self._ptr:
            lib = _get_lib()
            lib.baba_window_hide(self._ptr)

    def close(self):
        if self._ptr:
            lib = _get_lib()
            lib.baba_window_close(self._ptr)

    def destroy(self):
        if self._ptr:
            lib = _get_lib()
            lib.baba_window_destroy(self._ptr)
            self._ptr = None


class App:
    def __init__(self):
        lib = _get_lib()
        self._ptr = lib.baba_app_create()
        self._windows: List[Window] = []
        self._running = False

    def create_window(self, title: str, width: int = 800, height: int = 600) -> Window:
        window = Window(self, title, width, height)
        self._windows.append(window)
        return window

    def run(self) -> int:
        if not self._ptr:
            return -1
        lib = _get_lib()
        self._running = True
        return lib.baba_app_run(self._ptr)

    def quit(self):
        if self._ptr:
            lib = _get_lib()
            lib.baba_app_quit(self._ptr)
            self._running = False

    def destroy(self):
        if self._ptr:
            lib = _get_lib()
            for window in self._windows:
                window.destroy()
            self._windows.clear()
            lib.baba_app_destroy(self._ptr)
            self._ptr = None

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.destroy()
        return False


@contextmanager
def create_app():
    app = App()
    try:
        yield app
    finally:
        app.destroy()
