#import <Cocoa/Cocoa.h>
#include "../../platform/window.h"

@interface BabaNSWindowDelegate : NSObject<NSWindowDelegate>
@property (nonatomic, assign) BabaWindowCallbacks* callbacks;
@property (nonatomic, assign) void* userdata;
@end

@implementation BabaNSWindowDelegate

- (void)windowDidResize:(NSNotification*)notification {
    NSWindow* window = notification.object;
    NSSize size = window.contentView.bounds.size;
    if (self.callbacks && self.callbacks->on_resize) {
        self.callbacks->on_resize(self.userdata, (int)size.width, (int)size.height);
    }
}

- (BOOL)windowShouldClose:(id)sender {
    if (self.callbacks && self.callbacks->on_close) {
        self.callbacks->on_close(self.userdata);
    }
    return NO;
}

@end

struct BabaPlatformWindow {
    NSWindow* window;
    NSView* view;
    BabaNSWindowDelegate* delegate;
    BabaWindowCallbacks callbacks;
    bool should_close;
};

static NSApplication* g_app = nil;
static int g_window_count = 0;

BabaPlatformWindow* baba_platform_window_create(
    const char* title,
    int width,
    int height,
    BabaWindowCallbacks* callbacks
) {
    if (!g_app) {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp finishLaunching];
        g_app = NSApp;
    }
    
    BabaPlatformWindow* window = malloc(sizeof(BabaPlatformWindow));
    if (!window) return NULL;
    
    if (callbacks) {
        window->callbacks = *callbacks;
    }
    
    NSRect frame = NSMakeRect(0, 0, width, height);
    NSUInteger style_mask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | 
                           NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    
    window->window = [[NSWindow alloc] initWithContentRect:frame
                                                styleMask:style_mask
                                                  backing:NSBackingStoreBuffered
                                                    defer:NO];
    
    if (!window->window) {
        free(window);
        return NULL;
    }
    
    window->window.title = [NSString stringWithUTF8String:title];
    [window->window center];
    
    window->view = window->window.contentView;
    window->view.wantsLayer = YES;
    
    window->delegate = [[BabaNSWindowDelegate alloc] init];
    window->delegate.callbacks = &window->callbacks;
    window->delegate.userdata = callbacks ? callbacks->userdata : NULL;
    window->window.delegate = window->delegate;
    
    window->should_close = false;
    g_window_count++;
    
    return window;
}

void baba_platform_window_destroy(BabaPlatformWindow* window) {
    if (!window) return;
    
    window->window.delegate = nil;
    [window->window close];
    window->window = nil;
    window->view = nil;
    window->delegate = nil;
    
    g_window_count--;
    
    if (g_window_count == 0 && g_app) {
        [g_app terminate:nil];
        g_app = nil;
    }
    
    free(window);
}

void baba_platform_window_set_title(BabaPlatformWindow* window, const char* title) {
    if (!window || !title) return;
    window->window.title = [NSString stringWithUTF8String:title];
}

void baba_platform_window_get_size(BabaPlatformWindow* window, int* width, int* height) {
    if (!window) return;
    NSSize size = window->view.bounds.size;
    if (width) *width = (int)size.width;
    if (height) *height = (int)size.height;
}

void baba_platform_window_get_position(BabaPlatformWindow* window, int* x, int* y) {
    if (!window) return;
    NSRect frame = window->window.frame;
    if (x) *x = (int)frame.origin.x;
    if (y) *y = (int)frame.origin.y;
}

void baba_platform_window_set_size(BabaPlatformWindow* window, int width, int height) {
    if (!window) return;
    NSSize size = NSMakeSize(width, height);
    [window->window setContentSize:size];
}

void baba_platform_window_set_position(BabaPlatformWindow* window, int x, int y) {
    if (!window) return;
    NSScreen* screen = [NSScreen mainScreen];
    NSRect screen_frame = screen.frame;
    NSPoint point = NSMakePoint(x, screen_frame.size.height - y);
    [window->window setFrameOrigin:point];
}

void baba_platform_window_show(BabaPlatformWindow* window) {
    if (!window) return;
    [window->window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

void baba_platform_window_hide(BabaPlatformWindow* window) {
    if (!window) return;
    [window->window orderOut:nil];
}

void baba_platform_window_close(BabaPlatformWindow* window) {
    if (!window) return;
    window->should_close = true;
}

bool baba_platform_window_should_close(BabaPlatformWindow* window) {
    return window ? window->should_close : true;
}

const char** baba_platform_get_required_vulkan_extensions(uint32_t* count) {
    static const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        "VK_EXT_metal_surface"
    };
    if (count) *count = 2;
    return extensions;
}

double baba_platform_get_time(void) {
    return [NSDate timeIntervalSinceReferenceDate];
}

void baba_platform_poll_events(void) {
    @autoreleasepool {
        NSEvent* event;
        while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                           untilDate:[NSDate distantPast]
                                              inMode:NSDefaultRunLoopMode
                                             dequeue:YES])) {
            [NSApp sendEvent:event];
        }
    }
}