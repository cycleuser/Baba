# Baba Architecture

Baba is designed as a minimal, dependency-light GUI framework that leverages Vulkan for GPU-accelerated rendering. The architecture follows a layered approach with clear separation of concerns.

## Design Principles

1. **Minimal Dependencies**: Only Vulkan SDK is required; all other functionality is implemented natively
2. **Platform Abstraction**: Window management isolated behind a platform interface
3. **Immediate-Mode Friendly**: Can be used with both retained and immediate-mode patterns
4. **GPU-First**: Vulkan-centric rendering pipeline optimized for modern GPUs

## Layer Architecture

```
┌─────────────────────────────────────────────┐
│           Application Layer                 │
│  (BabaApp, BabaWindow, Event Loop)         │
├─────────────────────────────────────────────┤
│           Widget Layer                      │
│  (Button, Label, TextBox, Containers)      │
├─────────────────────────────────────────────┤
│           Layout Layer                      │
│  (Flex Layout, Grid Layout)                │
├─────────────────────────────────────────────┤
│           Rendering Layer                   │
│  (Painter API, Font Rendering, Shapes)     │
├─────────────────────────────────────────────┤
│           Vulkan Backend                    │
│  (Device, Swapchain, Render Pass, Pipelines)│
├─────────────────────────────────────────────┤
│           Platform Layer                    │
│  (macOS/Linux/Windows Window Management)   │
└─────────────────────────────────────────────┘
```

## Platform Layer

The platform layer provides an abstraction over native window management:

### Responsibilities
- Window creation and lifecycle
- Event handling (mouse, keyboard, resize)
- Vulkan surface creation
- Platform-specific extensions

### Implementation Strategy

Each platform implements:
```c
// Window management
BabaPlatformWindow* baba_platform_window_create(...);
void baba_platform_window_destroy(...);
void baba_platform_window_show(...);

// Vulkan integration
VkSurfaceKHR baba_platform_window_create_surface(...);
const char** baba_platform_get_required_vulkan_extensions(...);

// Event processing
void baba_platform_poll_events(void);
bool baba_platform_window_should_close(...);
```

### Platform Details

**macOS**
- Uses Cocoa framework for window management
- MoltenVK or VK_EXT_metal_surface for Vulkan interop
- CAMetalLayer as the rendering surface

**Linux**
- X11: Uses Xlib for window creation, VK_KHR_xlib_surface
- Wayland: Uses libwayland-client, VK_KHR_wayland_surface
- Runtime selection based on environment

**Windows**
- Win32 API for window management
- VK_KHR_win32_surface for surface creation

## Vulkan Renderer

### Initialization Sequence

```
1. Create Instance (with platform extensions)
2. Create Surface (platform-specific)
3. Select Physical Device (GPU)
4. Create Device & Queues
5. Create Swapchain
6. Create Render Pass
7. Create Framebuffers
8. Create Command Buffers
9. Create Synchronization Primitives
```

### Rendering Pipeline

```
1. Acquire next swapchain image
2. Begin command buffer
3. Begin render pass
4. Record drawing commands (via Painter)
5. End render pass
6. End command buffer
7. Submit to graphics queue
8. Present to present queue
```

### Resource Management

- **Vertex Buffers**: Dynamic ring buffer for UI geometry
- **Uniform Buffers**: Per-frame transforms and style data
- **Descriptor Sets**: Batching for efficient rendering
- **Pipeline Cache**: Cached pipelines for common operations

## Painter API

The Painter provides a high-level 2D drawing interface:

### Shapes
- Rectangles (filled, stroked)
- Rounded rectangles
- Circles and ellipses
- Lines and paths
- Bezier curves

### Text
- TrueType font loading (via stb_truetype)
- Glyph caching in texture atlas
- Unicode support
- Text measurement

### Images
- Image loading (via stb_image)
- Texture caching
- Scaling and transformation

### Clipping & Transform
- Rectangular clipping regions
- 2D affine transforms
- Push/pop stack

## Widget System

### Widget Lifecycle

```
1. Create (baba_widget_create)
2. Configure (set properties, callbacks)
3. Add to hierarchy (baba_widget_add_child)
4. Layout (automatic or manual)
5. Draw (baba_widget_draw_recursive)
6. Handle events (baba_widget_dispatch_event)
7. Destroy (baba_widget_destroy)
```

### Widget VTable

Each widget type implements a vtable:
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

### Event Propagation

Events propagate through the widget tree:
1. Capture phase: Root to target
2. Target phase: Target widget
3. Bubble phase: Target to root

## Layout System

### Flex Layout

Similar to CSS Flexbox:
- Direction: row/column
- Alignment: start/center/end/stretch
- Justify: space-between/around/evenly
- Wrapping: wrap/nowrap
- Gap: spacing between items

### Grid Layout

Similar to CSS Grid:
- Fixed columns/rows
- Configurable gaps
- Auto-sizing support
- Explicit cell sizing

### Measurement Protocol

1. Parent asks child to measure with available space
2. Child returns preferred size
3. Parent applies layout constraints
4. Parent sets final bounds

## Theme System

### Color Scheme
```c
typedef struct {
    BabaColor primary;
    BabaColor secondary;
    BabaColor background;
    BabaColor surface;
    BabaColor error;
    // ... text colors
} BabaColorScheme;
```

### Typography
```c
typedef struct {
    const char* regular;
    const char* bold;
    float size;
    float line_height;
} BabaFontFamily;
```

### Shape Styles
```c
typedef struct {
    float corner_radius;
    float border_width;
    float shadow_radius;
    // ...
} BabaShapeStyle;
```

## Memory Management

- **Widgets**: Reference-counted with parent-child relationships
- **Buffers**: Ring buffer for vertex data, pool for uniform buffers
- **Textures**: Cache with LRU eviction
- **Fonts**: Shared across widgets, cached globally

## Performance Considerations

1. **Batching**: Multiple draw calls combined into single Vulkan commands
2. **Culling**: Off-screen widgets skipped in draw pass
3. **Dirty Tracking**: Only redraw widgets that changed
4. **Texture Atlas**: Glyphs packed into shared texture
5. **Command Buffer Reuse**: Cache and reuse command buffers when possible