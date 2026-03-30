import baba


def on_click(btn):
    label.set_text("Button clicked!")


def on_open(btn):
    path = baba.file_open_dialog(title="Open CSV", filter="csv,txt")
    if path:
        label.set_text(f"Selected: {path}")


app = baba.App()
win = baba.Window(app, "Baba Demo", 400, 300)

btn = baba.Button(win, "Click Me", 150, 120, 100, 40)
btn.set_callback(on_click)

btn2 = baba.Button(win, "Open File", 150, 180, 100, 40)
btn2.set_callback(on_open)

label = baba.Label(win, "Hello, Baba!", 100, 50, 200, 30)

win.show()
app.run()
