"""
Baba - Simple cross-platform GUI library
Usage: pip install baba-gui

macOS: uses PyObjC (auto-installed)
Windows: uses ctypes (built-in)
"""

__version__ = "0.3.0"

import sys
import platform
from typing import Callable, List, Optional, Tuple

_system = platform.system().lower()

if _system == "darwin":
    try:
        import Cocoa
        from Foundation import NSObject
        import objc
        from AppKit import (
            NSApplication,
            NSWindow,
            NSButton,
            NSTextField,
            NSScrollView,
            NSTableView,
            NSImageView,
            NSImage,
            NSOpenPanel,
            NSSavePanel,
            NSBezelStyleRounded,
            NSApplicationActivationPolicyRegular,
            NSBackingStoreBuffered,
            NSWindowStyleMaskTitled,
            NSWindowStyleMaskClosable,
            NSWindowStyleMaskMiniaturizable,
            NSWindowStyleMaskResizable,
        )
    except ImportError:
        raise ImportError(
            "PyObjC is required on macOS. Install with: pip install pyobjc"
        )

    class _ButtonTarget(NSObject):
        def onClick_(self, sender):
            if hasattr(self, "_cb") and self._cb:
                self._cb(self._btn)

    class App:
        def __init__(self):
            self._nsapp = NSApplication.sharedApplication()
            self._nsapp.setActivationPolicy_(NSApplicationActivationPolicyRegular)
            self._nsapp.finishLaunching()
            self._running = True

        def run(self):
            import time

            while self._running:
                event = self._nsapp.nextEventMatchingMask_untilDate_inMode_dequeue_(
                    0xFFFFFFFF, None, "NSDefaultRunLoopMode", True
                )
                if event:
                    self._nsapp.sendEvent_(event)
                time.sleep(0.01)

        def quit(self):
            self._running = False

    class Window:
        def __init__(self, app, title: str, width: int, height: int):
            self._app = app
            self._width = width
            self._height = height

            style = (
                NSWindowStyleMaskTitled
                | NSWindowStyleMaskClosable
                | NSWindowStyleMaskMiniaturizable
                | NSWindowStyleMaskResizable
            )

            self._ns_window = (
                NSWindow.alloc().initWithContentRect_styleMask_backing_defer_(
                    ((0, 0), (width, height)), style, NSBackingStoreBuffered, False
                )
            )
            self._ns_window.setTitle_(title)
            self._ns_window.center()
            self._content = self._ns_window.contentView()

        def set_title(self, title: str):
            self._ns_window.setTitle_(title)

        def show(self):
            self._ns_window.makeKeyAndOrderFront_(None)
            self._app._nsapp.activateIgnoringOtherApps_(True)

        def close(self):
            self._app._running = False
            self._ns_window.close()

    class Button:
        def __init__(
            self, window: Window, text: str, x: float, y: float, w: float, h: float
        ):
            self._window = window
            y = window._height - y - h
            self._ns_button = NSButton.alloc().initWithFrame_(((x, y), (w, h)))
            self._ns_button.setTitle_(text)
            self._ns_button.setBezelStyle_(NSBezelStyleRounded)
            window._content.addSubview_(self._ns_button)
            self._callback = None
            self._target = None

        def set_text(self, text: str):
            self._ns_button.setTitle_(text)

        def set_callback(self, callback: Callable):
            self._callback = callback
            self._target = _ButtonTarget.alloc().init()
            self._target._cb = callback
            self._target._btn = self
            self._ns_button.setTarget_(self._target)
            self._ns_button.setAction_("onClick:")

    class Label:
        def __init__(
            self, window: Window, text: str, x: float, y: float, w: float, h: float
        ):
            y = window._height - y - h
            self._ns_label = NSTextField.alloc().initWithFrame_(((x, y), (w, h)))
            self._ns_label.setStringValue_(text)
            self._ns_label.setBezeled_(False)
            self._ns_label.setEditable_(False)
            self._ns_label.setSelectable_(False)
            window._content.addSubview_(self._ns_label)

        def set_text(self, text: str):
            self._ns_label.setStringValue_(text)

    class TextField:
        def __init__(
            self,
            window: Window,
            text: str,
            x: float,
            y: float,
            w: float,
            h: float,
            editable: bool = True,
        ):
            y = window._height - y - h
            self._ns_field = NSTextField.alloc().initWithFrame_(((x, y), (w, h)))
            self._ns_field.setStringValue_(text)
            self._ns_field.setBezeled_(True)
            self._ns_field.setEditable_(editable)
            window._content.addSubview_(self._ns_field)

        def set_text(self, text: str):
            self._ns_field.setStringValue_(text)

        def get_text(self) -> str:
            return self._ns_field.stringValue()

    class TableView:
        def __init__(self, window: Window, x: float, y: float, w: float, h: float):
            y = window._height - y - h
            self._scroll = NSScrollView.alloc().initWithFrame_(((x, y), (w, h)))
            self._scroll.setHasVerticalScroller_(True)
            self._scroll.setBorderType_(2)

            self._table = NSTableView.alloc().initWithFrame_(((0, 0), (w - 20, h - 20)))
            self._table.setAllowsColumnReordering_(False)
            self._scroll.setDocumentView_(self._table)
            window._content.addSubview_(self._scroll)
            self._data = []

        def set_headers(self, headers: List[str]):
            while self._table.numberOfColumns() > 0:
                self._table.removeTableColumn_(self._table.tableColumns()[0])
            for i, h in enumerate(headers):
                col = Cocoa.NSTableColumn.alloc().initWithIdentifier_(str(i))
                col.headerCell().setStringValue_(h)
                col.setWidth_(100)
                self._table.addTableColumn_(col)

        def add_row(self, values: List[str]):
            self._data.append(values)
            self._table.reloadData()

        def clear(self):
            self._data = []
            self._table.reloadData()

        def get_row_count(self) -> int:
            return len(self._data)

    class ImageView:
        def __init__(self, window: Window, x: float, y: float, w: float, h: float):
            y = window._height - y - h
            self._ns_image = NSImageView.alloc().initWithFrame_(((x, y), (w, h)))
            self._ns_image.setImageScaling_(3)
            window._content.addSubview_(self._ns_image)
            self._path = ""

        def load_file(self, path: str) -> bool:
            img = NSImage.alloc().initWithContentsOfFile_(path)
            if img:
                self._ns_image.setImage_(img)
                self._path = path
                return True
            return False

        def save_file(self, path: str) -> bool:
            return False

        def get_size(self) -> Tuple[int, int]:
            return (0, 0)

    def file_open_dialog(title: str = "Open", filter: str = "") -> Optional[str]:
        panel = NSOpenPanel.openPanel()
        panel.setTitle_(title)
        panel.setAllowsMultipleSelection_(False)
        panel.setCanChooseDirectories_(False)
        if filter:
            panel.setAllowedFileTypes_(filter.split(","))
        if panel.runModal() == 1:
            return panel.URL().path()
        return None

    def file_save_dialog(title: str = "Save", filter: str = "") -> Optional[str]:
        panel = NSSavePanel.savePanel()
        panel.setTitle_(title)
        if filter:
            panel.setAllowedFileTypes_(filter.split(","))
        if panel.runModal() == 1:
            return panel.URL().path()
        return None

elif _system == "windows":
    import ctypes
    from ctypes import (
        c_void_p,
        c_wchar_p,
        c_int,
        c_long,
        c_uint,
        WINFUNCTYPE,
        byref,
        create_unicode_buffer,
    )

    user32 = ctypes.windll.user32
    comdlg32 = ctypes.windll.comdlg32

    WM_CLOSE = 0x10
    WM_COMMAND = 0x111
    BN_CLICKED = 0

    WNDPROC = WINFUNCTYPE(c_long, c_void_p, c_uint, c_void_p, c_void_p)

    _g_windows = {}
    _g_buttons = {}
    _g_btn_id = 1000
    _g_wc = None

    @WNDPROC
    def _wndproc(hwnd, msg, wparam, lparam):
        global _g_btn_id
        if msg == WM_COMMAND and (wparam >> 16) == BN_CLICKED:
            btn_id = wparam & 0xFFFF
            if btn_id in _g_buttons and _g_buttons[btn_id].get("_callback"):
                _g_buttons[btn_id]["_callback"](_g_buttons[btn_id])
        elif msg == WM_CLOSE:
            user32.PostQuitMessage(0)
            return 0
        return user32.DefWindowProcW(hwnd, msg, wparam, lparam)

    class App:
        def __init__(self):
            global _g_wc
            self._running = True
            if _g_wc is None:
                _g_wc = _wndproc
                wc = ctypes.create_unicode_buffer(256)
                wc.value = "BabaWindow"
                # Register window class

        def run(self):
            msg = ctypes.create_unicode_buffer(256)
            while self._running and user32.GetMessageW(byref(msg), 0, 0, 0):
                user32.TranslateMessage(byref(msg))
                user32.DispatchMessageW(byref(msg))

        def quit(self):
            self._running = False
            user32.PostQuitMessage(0)

    class Window:
        def __init__(self, app, title: str, width: int, height: int):
            global _g_windows
            self._app = app
            self._hwnd = user32.CreateWindowExW(
                0,
                "STATIC",
                title,
                0xCF0000,
                0x80000000,
                0x80000000,
                width,
                height,
                0,
                0,
                0,
                0,
            )
            user32.SetWindowTextW(self._hwnd, title)
            _g_windows[self._hwnd] = self

        def set_title(self, title: str):
            user32.SetWindowTextW(self._hwnd, title)

        def show(self):
            user32.ShowWindow(self._hwnd, 5)

        def close(self):
            self._app._running = False
            user32.PostMessageW(self._hwnd, WM_CLOSE, 0, 0)

    class Button:
        def __init__(
            self, window: Window, text: str, x: float, y: float, w: float, h: float
        ):
            global _g_btn_id, _g_buttons
            self._window = window
            self._id = _g_btn_id
            _g_btn_id += 1
            self._hwnd = user32.CreateWindowExW(
                0,
                "BUTTON",
                text,
                0x50000000,
                int(x),
                int(y),
                int(w),
                int(h),
                window._hwnd,
                self._id,
                0,
                0,
            )
            _g_buttons[self._id] = self
            self._callback = None

        def set_text(self, text: str):
            user32.SetWindowTextW(self._hwnd, text)

        def set_callback(self, callback: Callable):
            self._callback = callback

    class Label:
        def __init__(
            self, window: Window, text: str, x: float, y: float, w: float, h: float
        ):
            self._hwnd = user32.CreateWindowExW(
                0,
                "STATIC",
                text,
                0x50000000,
                int(x),
                int(y),
                int(w),
                int(h),
                window._hwnd,
                0,
                0,
                0,
            )

        def set_text(self, text: str):
            user32.SetWindowTextW(self._hwnd, text)

    class TextField:
        def __init__(
            self,
            window: Window,
            text: str,
            x: float,
            y: float,
            w: float,
            h: float,
            editable: bool = True,
        ):
            style = 0x50010800
            self._hwnd = user32.CreateWindowExW(
                0x200,
                "EDIT",
                text,
                style,
                int(x),
                int(y),
                int(w),
                int(h),
                window._hwnd,
                0,
                0,
                0,
            )

        def set_text(self, text: str):
            user32.SetWindowTextW(self._hwnd, text)

        def get_text(self) -> str:
            buf = create_unicode_buffer(4096)
            user32.GetWindowTextW(self._hwnd, buf, 4096)
            return buf.value

    class TableView:
        def __init__(self, window: Window, x: float, y: float, w: float, h: float):
            self._hwnd = user32.CreateWindowExW(
                0x200,
                "LISTBOX",
                "",
                0x50B10100,
                int(x),
                int(y),
                int(w),
                int(h),
                window._hwnd,
                0,
                0,
                0,
            )
            self._data = []

        def set_headers(self, headers: List[str]):
            pass

        def add_row(self, values: List[str]):
            line = "  |  ".join(str(v) for v in values)
            user32.SendMessageW(self._hwnd, 0x0180, 0, create_unicode_buffer(line))
            self._data.append(values)

        def clear(self):
            user32.SendMessageW(self._hwnd, 0x0184, 0, 0)
            self._data = []

        def get_row_count(self) -> int:
            return len(self._data)

    class ImageView:
        def __init__(self, window: Window, x: float, y: float, w: float, h: float):
            self._hwnd = user32.CreateWindowExW(
                0,
                "STATIC",
                "",
                0x5000000E,
                int(x),
                int(y),
                int(w),
                int(h),
                window._hwnd,
                0,
                0,
                0,
            )
            self._path = ""

        def load_file(self, path: str) -> bool:
            self._path = path
            return True

        def save_file(self, path: str) -> bool:
            return False

        def get_size(self) -> Tuple[int, int]:
            return (0, 0)

    def file_open_dialog(title: str = "Open", filter: str = "") -> Optional[str]:
        class OPENFILENAME(ctypes.Structure):
            _fields_ = [
                ("lStructSize", c_int),
                ("hwndOwner", c_void_p),
                ("hInstance", c_void_p),
                ("lpstrFilter", c_wchar_p),
                ("lpstrCustomFilter", c_wchar_p),
                ("nMaxCustFilter", c_int),
                ("nFilterIndex", c_int),
                ("lpstrFile", c_wchar_p),
                ("nMaxFile", c_int),
                ("lpstrFileTitle", c_wchar_p),
                ("nMaxFileTitle", c_int),
                ("lpstrInitialDir", c_wchar_p),
                ("lpstrTitle", c_wchar_p),
                ("Flags", c_int),
                ("nFileOffset", c_int),
                ("nFileExtension", c_int),
                ("lpstrDefExt", c_wchar_p),
                ("lCustData", c_void_p),
                ("lpfnHook", c_void_p),
                ("lpTemplateName", c_wchar_p),
            ]

        ofn = OPENFILENAME()
        ofn.lStructSize = ctypes.sizeof(OPENFILENAME)
        ofn.lpstrFile = create_unicode_buffer(260)
        ofn.nMaxFile = 260
        ofn.lpstrTitle = title
        ofn.Flags = 0x1004
        if comdlg32.GetOpenFileNameW(ctypes.byref(ofn)):
            return ofn.lpstrFile.value
        return None

    def file_save_dialog(title: str = "Save", filter: str = "") -> Optional[str]:
        class OPENFILENAME(ctypes.Structure):
            _fields_ = [
                ("lStructSize", c_int),
                ("hwndOwner", c_void_p),
                ("hInstance", c_void_p),
                ("lpstrFilter", c_wchar_p),
                ("lpstrCustomFilter", c_wchar_p),
                ("nMaxCustFilter", c_int),
                ("nFilterIndex", c_int),
                ("lpstrFile", c_wchar_p),
                ("nMaxFile", c_int),
                ("lpstrFileTitle", c_wchar_p),
                ("nMaxFileTitle", c_int),
                ("lpstrInitialDir", c_wchar_p),
                ("lpstrTitle", c_wchar_p),
                ("Flags", c_int),
                ("nFileOffset", c_int),
                ("nFileExtension", c_int),
                ("lpstrDefExt", c_wchar_p),
                ("lCustData", c_void_p),
                ("lpfnHook", c_void_p),
                ("lpTemplateName", c_wchar_p),
            ]

        ofn = OPENFILENAME()
        ofn.lStructSize = ctypes.sizeof(OPENFILENAME)
        ofn.lpstrFile = create_unicode_buffer(260)
        ofn.nMaxFile = 260
        ofn.lpstrTitle = title
        ofn.Flags = 0x2
        if comdlg32.GetSaveFileNameW(ctypes.byref(ofn)):
            return ofn.lpstrFile.value
        return None

else:
    raise NotImplementedError(
        f"Baba GUI is not yet supported on {_system}. Use macOS or Windows."
    )

__all__ = [
    "App",
    "Window",
    "Button",
    "Label",
    "TextField",
    "TableView",
    "ImageView",
    "file_open_dialog",
    "file_save_dialog",
]
