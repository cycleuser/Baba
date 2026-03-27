# Python Package

This directory contains the Python bindings for Baba GUI.

## Installation

```bash
pip install baba-gui
```

Or from source:

```bash
pip install .
```

## Quick Start

```python
import baba

def on_click(button):
    print("Clicked!")

with baba.create_app() as app:
    window = app.create_window("Demo", 800, 600)
    
    button = baba.Button("Click Me")
    button.on_click(on_click)
    window.root.add_child(button)
    
    label = baba.Label("Hello, Baba!")
    label.font_size = 24.0
    label.color = baba.Color.from_hex("#333333")
    window.root.add_child(label)
    
    window.show()
    app.run()
```

## API Reference

### App

- `baba.App()` - Create application
- `app.create_window(title, width, height)` - Create window
- `app.run()` - Run main loop
- `app.quit()` - Quit application

### Window

- `window.title` - Get/set title
- `window.size` - Get/set size as (width, height)
- `window.root` - Get root widget
- `window.show()` - Show window
- `window.hide()` - Hide window
- `window.close()` - Close window

### Widgets

#### Button
- `baba.Button(text)` - Create button
- `button.text` - Get/set text
- `button.on_click(callback)` - Set click handler

#### Label
- `baba.Label(text)` - Create label
- `label.text` - Get/set text
- `label.color` - Set text color
- `label.font_size` - Set font size

#### TextBox
- `baba.TextBox(placeholder)` - Create textbox
- `textbox.text` - Get/set text
- `textbox.password_mode` - Enable password mode

### Colors

```python
# RGB values (0-255)
c = baba.Color.from_rgb(255, 128, 64)

# Hex string
c = baba.Color.from_hex("#FF8040")

# Direct float values (0.0-1.0)
c = baba.Color(1.0, 0.5, 0.25, 1.0)

# Preset colors
baba.Color.WHITE()
baba.Color.BLACK()
baba.Color.RED()
baba.Color.GREEN()
baba.Color.BLUE()
```