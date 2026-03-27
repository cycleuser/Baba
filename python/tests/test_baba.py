import pytest
import baba


def test_color_creation():
    c = baba.Color(1.0, 0.5, 0.0, 1.0)
    assert c.r == 1.0
    assert c.g == 0.5
    assert c.b == 0.0
    assert c.a == 1.0


def test_color_from_hex():
    c = baba.Color.from_hex("#FF5500")
    assert c.r == 1.0
    assert c.g == pytest.approx(0x55 / 255.0)
    assert c.b == 0.0


def test_color_from_rgb():
    c = baba.Color.from_rgb(255, 128, 64)
    assert c.r == 1.0
    assert c.g == pytest.approx(128 / 255.0)
    assert c.b == pytest.approx(64 / 255.0)


def test_rect():
    r = baba.Rect(10, 20, 100, 200)
    assert r.x == 10
    assert r.y == 20
    assert r.width == 100
    assert r.height == 200
    assert r.right == 110
    assert r.bottom == 220


def test_vec2():
    v = baba.Vec2(3.0, 4.0)
    assert v.x == 3.0
    assert v.y == 4.0


def test_button_creation():
    btn = baba.Button("Test Button")
    assert btn.text == "Test Button"


def test_label_creation():
    label = baba.Label("Test Label")
    assert label.text == "Test Label"


def test_textbox_creation():
    tb = baba.TextBox("Enter text...")
    assert tb.text == ""
