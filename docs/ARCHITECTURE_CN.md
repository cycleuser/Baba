# 把拔架构

把拔（Baba）设计为一个最小依赖、轻量级的GUI框架，利用Vulkan实现GPU加速渲染。架构采用分层方法，关注点清晰分离。

## 设计原则

1. **最小依赖**: 仅需Vulkan SDK，其他功能全部原生实现
2. **平台抽象**: 窗口管理通过平台接口隔离
3. **即期模式友好**: 同时支持保留模式和即期模式
4. **GPU优先**: 以Vulkan为中心的渲染管线，针对现代GPU优化

## 分层架构

```
┌─────────────────────────────────────────────┐
│             应用层                           │
│      (BabaApp, BabaWindow, 事件循环)        │
├─────────────────────────────────────────────┤
│             控件层                           │
│      (按钮, 标签, 文本框, 容器)              │
├─────────────────────────────────────────────┤
│             布局层                           │
│      (弹性布局, 网格布局)                    │
├─────────────────────────────────────────────┤
│             渲染层                           │
│      (绑定器API, 字体渲染, 图形)            │
├─────────────────────────────────────────────┤
│             Vulkan后端                       │
│      (设备, 交换链, 渲染通道, 管线)          │
├─────────────────────────────────────────────┤
│             平台层                           │
│      (macOS/Linux/Windows 窗口管理)         │
└─────────────────────────────────────────────┘
```

## 平台层

平台层提供原生窗口管理的抽象：

### 职责
- 窗口创建和生命周期
- 事件处理（鼠标、键盘、调整大小）
- Vulkan Surface创建
- 平台特定扩展

### 实现策略

每个平台实现：
```c
// 窗口管理
BabaPlatformWindow* baba_platform_window_create(...);
void baba_platform_window_destroy(...);
void baba_platform_window_show(...);

// Vulkan集成
VkSurfaceKHR baba_platform_window_create_surface(...);
const char** baba_platform_get_required_vulkan_extensions(...);

// 事件处理
void baba_platform_poll_events(void);
bool baba_platform_window_should_close(...);
```

### 平台详情

**macOS**
- 使用Cocoa框架进行窗口管理
- MoltenVK或VK_EXT_metal_surface实现Vulkan互操作
- CAMetalLayer作为渲染表面

**Linux**
- X11: 使用Xlib创建窗口，VK_KHR_xlib_surface
- Wayland: 使用libwayland-client，VK_KHR_wayland_surface
- 基于环境运行时选择

**Windows**
- Win32 API进行窗口管理
- VK_KHR_win32_surface创建Surface

## Vulkan渲染器

### 初始化序列

```
1. 创建Instance（带平台扩展）
2. 创建Surface（平台特定）
3. 选择物理设备（GPU）
4. 创建设备和队列
5. 创建交换链
6. 创建渲染通道
7. 创建帧缓冲
8. 创建命令缓冲
9. 创建同步原语
```

### 渲染管线

```
1. 获取下一个交换链图像
2. 开始命令缓冲
3. 开始渲染通道
4. 记录绘制命令（通过绑定器）
5. 结束渲染通道
6. 结束命令缓冲
7. 提交到图形队列
8. 呈现到呈现队列
```

### 资源管理

- **顶点缓冲**: UI几何的动态环形缓冲
- **统一缓冲**: 每帧变换和样式数据
- **描述符集**: 批处理以高效渲染
- **管线缓存**: 常用操作的缓存管线

## 绑定器API（Painter）

绑定器提供高级2D绘图接口：

### 图形
- 矩形（填充、描边）
- 圆角矩形
- 圆和椭圆
- 线条和路径
- 贝塞尔曲线

### 文字
- TrueType字体加载（通过stb_truetype）
- 字形缓存在纹理图集中
- Unicode支持
- 文字测量

### 图像
- 图像加载（通过stb_image）
- 纹理缓存
- 缩放和变换

### 裁剪和变换
- 矩形裁剪区域
- 2D仿射变换
- 压入/弹出栈

## 控件系统

### 控件生命周期

```
1. 创建 (baba_widget_create)
2. 配置 (设置属性、回调)
3. 添加到层次结构 (baba_widget_add_child)
4. 布局 (自动或手动)
5. 绘制 (baba_widget_draw_recursive)
6. 处理事件 (baba_widget_dispatch_event)
7. 销毁 (baba_widget_destroy)
```

### 控件虚表

每种控件类型实现一个虚表：
```c
typedef struct {
    int (*init)(BabaWidget* widget);
    void (*destroy)(BabaWidget* widget);
    void (*layout)(BabaWidget* widget, BabaRect bounds);
    void (*draw)(BabaWidget* widget, BabaPainter* painter);
    bool (*event)(BabaWidget* widget, BabaEvent* event);
    BabaVec2 (*measure)(BabaWidget* widget, BabaVec2 available);
} BabaWidgetVTable;
```

### 事件传播

事件通过控件树传播：
1. 捕获阶段: 从根到目标
2. 目标阶段: 目标控件
3. 冒泡阶段: 从目标到根

## 布局系统

### 弹性布局

类似CSS Flexbox：
- 方向: 行/列
- 对齐: 起点/中心/终点/拉伸
- 分布: 间隔/环绕/均匀
- 换行: 换行/不换行
- 间距: 项目间距

### 网格布局

类似CSS Grid：
- 固定列/行
- 可配置间距
- 自动尺寸支持
- 显式单元格尺寸

### 测量协议

1. 父控件请求子控件在可用空间内测量
2. 子控件返回首选尺寸
3. 父控件应用布局约束
4. 父控件设置最终边界

## 主题系统

### 颜色方案
```c
typedef struct {
    BabaColor primary;      // 主色
    BabaColor secondary;    // 辅助色
    BabaColor background;   // 背景色
    BabaColor surface;      // 表面色
    BabaColor error;        // 错误色
    // ... 文字颜色
} BabaColorScheme;
```

### 字体排版
```c
typedef struct {
    const char* regular;    // 常规字体
    const char* bold;       // 粗体
    float size;             // 大小
    float line_height;      // 行高
} BabaFontFamily;
```

### 形状样式
```c
typedef struct {
    float corner_radius;    // 圆角半径
    float border_width;     // 边框宽度
    float shadow_radius;    // 阴影半径
    // ...
} BabaShapeStyle;
```

## 内存管理

- **控件**: 引用计数，父子关系
- **缓冲**: 顶点数据用环形缓冲，统一缓冲用池
- **纹理**: LRU淘汰的缓存
- **字体**: 全局缓存，跨控件共享

## 性能考量

1. **批处理**: 多个绘制调用合并为单个Vulkan命令
2. **剔除**: 跳过屏幕外控件的绘制
3. **脏标记**: 仅重绘变化的控件
4. **纹理图集**: 字形打包到共享纹理
5. **命令缓冲复用**: 尽可能缓存和复用命令缓冲