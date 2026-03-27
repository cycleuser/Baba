import baba

current_value = 0.0
stored_value = 0.0
current_op = None
new_input = True


def update_display(display, text):
    display.text = text


def digit_pressed(button, digit):
    global new_input
    if new_input:
        update_display(button._parent._display, digit)
        new_input = False
    else:
        current = button._parent._display.text
        if current == "0":
            update_display(button._parent._display, digit)
        elif len(current) < 15:
            update_display(button._parent._display, current + digit)


def decimal_pressed(button):
    global new_input
    current = button._parent._display.text
    if new_input:
        update_display(button._parent._display, "0.")
        new_input = False
    elif "." not in current:
        update_display(button._parent._display, current + ".")


def clear_pressed(button):
    global current_value, stored_value, current_op, new_input
    update_display(button._parent._display, "0")
    current_value = 0.0
    stored_value = 0.0
    current_op = None
    new_input = True


def operator_pressed(button, op):
    global current_value, stored_value, current_op, new_input
    current_value = float(button._parent._display.text.replace(",", ""))
    stored_value = current_value
    current_op = op
    new_input = True


def equals_pressed(button):
    global current_value, stored_value, current_op, new_input
    try:
        second = float(button._parent._display.text.replace(",", ""))
        result = 0.0

        if current_op == "+":
            result = stored_value + second
        elif current_op == "-":
            result = stored_value - second
        elif current_op == "*":
            result = stored_value * second
        elif current_op == "/":
            if second != 0:
                result = stored_value / second
            else:
                update_display(button._parent._display, "Error")
                new_input = True
                return

        if result == int(result):
            text = str(int(result))
        else:
            text = str(result)

        update_display(button._parent._display, text)
        current_value = result
        current_op = None
        new_input = True
    except:
        update_display(button._parent._display, "Error")
        new_input = True


def percent_pressed(button):
    current = float(button._parent._display.text.replace(",", ""))
    result = current / 100.0
    if result == int(result):
        update_display(button._parent._display, str(int(result)))
    else:
        update_display(button._parent._display, str(result))


def negate_pressed(button):
    current = float(button._parent._display.text.replace(",", ""))
    result = -current
    if result == int(result):
        update_display(button._parent._display, str(int(result)))
    else:
        update_display(button._parent._display, str(result))


class CalculatorWindow:
    def __init__(self, app):
        self.window = app.create_window("把拔计算器", 320, 480)
        self._display = None

        root = self.window.root
        root._display = None

        display = baba.Label("0")
        display.font_size = 36.0
        display.color = baba.Color.WHITE()
        display._parent = self
        root._display = display
        root.add_child(display)

        buttons = [
            ("C", lambda b: clear_pressed(b)),
            ("±", lambda b: negate_pressed(b)),
            ("%", lambda b: percent_pressed(b)),
            ("/", lambda b: operator_pressed(b, "/")),
            ("7", lambda b: digit_pressed(b, "7")),
            ("8", lambda b: digit_pressed(b, "8")),
            ("9", lambda b: digit_pressed(b, "9")),
            ("*", lambda b: operator_pressed(b, "*")),
            ("4", lambda b: digit_pressed(b, "4")),
            ("5", lambda b: digit_pressed(b, "5")),
            ("6", lambda b: digit_pressed(b, "6")),
            ("-", lambda b: operator_pressed(b, "-")),
            ("1", lambda b: digit_pressed(b, "1")),
            ("2", lambda b: digit_pressed(b, "2")),
            ("3", lambda b: digit_pressed(b, "3")),
            ("+", lambda b: operator_pressed(b, "+")),
            ("0", lambda b: digit_pressed(b, "0")),
            (".", lambda b: decimal_pressed(b)),
            ("=", lambda b: equals_pressed(b)),
        ]

        for text, handler in buttons:
            btn = baba.Button(text)
            btn._parent = self
            btn.on_click(handler)
            root.add_child(btn)

    def show(self):
        self.window.show()


with baba.create_app() as app:
    calc = CalculatorWindow(app)
    calc.show()
    app.run()
