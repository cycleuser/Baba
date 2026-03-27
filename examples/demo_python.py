import baba


def on_click(button):
    print("Button clicked!")


with baba.create_app() as app:
    window = app.create_window("Baba Python Demo", 800, 600)

    button = baba.Button("Click Me")
    button.on_click(on_click)
    window.root.add_child(button)

    label = baba.Label("Hello from Python!")
    label.font_size = 24.0
    label.color = baba.Color.from_hex("#333333")
    window.root.add_child(label)

    window.show()
    app.run()
