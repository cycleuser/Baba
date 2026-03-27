# Python包

本目录包含Baba GUI的Python绑定。

## 安装

```bash
pip install baba-gui
```

或从源码安装：

```bash
pip install .
```

## 快速开始

```python
import baba

def on_click(button):
    print("点击了！")

with baba.create_app() as app:
    window = app.create_window("演示", 800, 600)
    
    button = baba.Button("点击我")
    button.on_click(on_click)
    window.root.add_child(button)
    
    label = baba.Label("你好，把拔！")
    label.font_size = 24.0
    label.color = baba.Color.from_hex("#333333")
    window.root.add_child(label)
    
    window.show()
    app.run()
```

## API参考

### 应用

- `baba.App()` - 创建应用
- `app.create_window(title, width, height)` - 创建窗口
- `app.run()` - 运行主循环
- `app.quit()` - 退出应用

### 窗口

- `window.title` - 获取/设置标题
- `window.size` - 获取/设置尺寸，返回(width, height)
- `window.root` - 获取根控件
- `window.show()` - 显示窗口
- `window.hide()` - 隐藏窗口
- `window.close()` - 关闭窗口

### 控件

#### 按钮
- `baba.Button(text)` - 创建按钮
- `button.text` - 获取/设置文本
- `button.on_click(callback)` - 设置点击回调

#### 标签
- `baba.Label(text)` - 创建标签
- `label.text` - 获取/设置文本
- `label.color` - 设置文本颜色
- `label.font_size` - 设置字体大小

#### 文本框
- `baba.TextBox(placeholder)` - 创建文本框
- `textbox.text` - 获取/设置文本
- `textbox.password_mode` - 启用密码模式

### 颜色

```python
# RGB值 (0-255)
c = baba.Color.from_rgb(255, 128, 64)

# 十六进制字符串
c = baba.Color.from_hex("#FF8040")

# 直接浮点值 (0.0-1.0)
c = baba.Color(1.0, 0.5, 0.25, 1.0)

# 预设颜色
baba.Color.WHITE()
baba.Color.BLACK()
baba.Color.RED()
baba.Color.GREEN()
baba.Color.BLUE()
```