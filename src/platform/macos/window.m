#import <Cocoa/Cocoa.h>
#include "../platform.h"

struct BabaPlatformWindow {
    NSWindow* ns_window;
    NSView* ns_view;
    BabaPlatformCallbacks callbacks;
    bool should_close;
    int width;
    int height;
};

@interface BabaWindowDelegate : NSObject <NSWindowDelegate>
@property (assign) BabaPlatformWindow* window;
@end

@implementation BabaWindowDelegate
- (void)windowDidResize:(NSNotification*)notification {
    if (self.window && self.window->callbacks.on_resize) {
        NSRect frame = [self.window->ns_view frame];
        self.window->width = (int)frame.size.width;
        self.window->height = (int)frame.size.height;
        self.window->callbacks.on_resize(
            self.window->callbacks.userdata,
            self.window->width,
            self.window->height
        );
    }
}

- (BOOL)windowShouldClose:(id)sender {
    if (self.window) {
        self.window->should_close = true;
        if (self.window->callbacks.on_close) {
            return self.window->callbacks.on_close(self.window->callbacks.userdata) ? YES : NO;
        }
    }
    return YES;
}
@end

@interface BabaView : NSView {
    BabaPlatformWindow* _babaWindow;
}
- (void)setBabaWindow:(BabaPlatformWindow*)window;
- (BabaPlatformWindow*)babaWindow;
@end

@implementation BabaView
- (void)setBabaWindow:(BabaPlatformWindow*)window {
    _babaWindow = window;
}
- (BabaPlatformWindow*)babaWindow {
    return _babaWindow;
}
- (void)mouseMoved:(NSEvent*)event {
    if (_babaWindow && _babaWindow->callbacks.on_mouse) {
        NSPoint point = [event locationInWindow];
        _babaWindow->callbacks.on_mouse(
            _babaWindow->callbacks.userdata,
            point.x,
            _babaWindow->height - point.y,
            0, false
        );
    }
}

- (void)mouseDown:(NSEvent*)event {
    if (_babaWindow && _babaWindow->callbacks.on_mouse) {
        NSPoint point = [event locationInWindow];
        _babaWindow->callbacks.on_mouse(
            _babaWindow->callbacks.userdata,
            point.x,
            _babaWindow->height - point.y,
            0, true
        );
    }
}

- (void)mouseUp:(NSEvent*)event {
    if (_babaWindow && _babaWindow->callbacks.on_mouse) {
        NSPoint point = [event locationInWindow];
        _babaWindow->callbacks.on_mouse(
            _babaWindow->callbacks.userdata,
            point.x,
            _babaWindow->height - point.y,
            0, false
        );
    }
}

- (void)keyDown:(NSEvent*)event {
    if (_babaWindow && _babaWindow->callbacks.on_key) {
        _babaWindow->callbacks.on_key(
            _babaWindow->callbacks.userdata,
            (int)[event keyCode],
            true
        );
    }
}

- (void)keyUp:(NSEvent*)event {
    if (_babaWindow && _babaWindow->callbacks.on_key) {
        _babaWindow->callbacks.on_key(
            _babaWindow->callbacks.userdata,
            (int)[event keyCode],
            false
        );
    }
}
@end

static NSApplication* g_app = NULL;

BabaPlatformWindow* baba_platform_window_create(
    const char* title,
    int width,
    int height,
    BabaPlatformCallbacks* callbacks
) {
    @autoreleasepool {
        if (!g_app) {
            [NSApplication sharedApplication];
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
            [NSApp finishLaunching];
            g_app = NSApp;
        }
        
        BabaPlatformWindow* window = calloc(1, sizeof(BabaPlatformWindow));
        if (!window) return NULL;
        
        if (callbacks) {
            window->callbacks = *callbacks;
        }
        window->width = width;
        window->height = height;
        
        NSRect frame = NSMakeRect(0, 0, width, height);
        
        NSUInteger styleMask = NSWindowStyleMaskTitled | 
                               NSWindowStyleMaskClosable | 
                               NSWindowStyleMaskMiniaturizable | 
                               NSWindowStyleMaskResizable;
        
        window->ns_window = [[NSWindow alloc] initWithContentRect:frame
                                                       styleMask:styleMask
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];
        
        if (!window->ns_window) {
            free(window);
            return NULL;
        }
        
        BabaView* view = [[BabaView alloc] initWithFrame:frame];
        [view setBabaWindow:window];
        window->ns_view = view;
        [window->ns_window setContentView:view];
        
        BabaWindowDelegate* delegate = [[BabaWindowDelegate alloc] init];
        delegate.window = window;
        [window->ns_window setDelegate:delegate];
        
        [window->ns_window setTitle:[NSString stringWithUTF8String:title ? title : "Baba"]];
        [window->ns_window center];
        [window->ns_window setAcceptsMouseMovedEvents:YES];
        
        return window;
    }
}

void baba_platform_window_destroy(BabaPlatformWindow* window) {
    if (!window) return;
    @autoreleasepool {
        if (window->ns_window) {
            [window->ns_window close];
        }
    }
    free(window);
}

void baba_platform_window_set_title(BabaPlatformWindow* window, const char* title) {
    if (!window || !window->ns_window || !title) return;
    @autoreleasepool {
        [window->ns_window setTitle:[NSString stringWithUTF8String:title]];
    }
}

void baba_platform_window_get_size(BabaPlatformWindow* window, int* width, int* height) {
    if (!window) return;
    if (width) *width = window->width;
    if (height) *height = window->height;
}

void baba_platform_window_set_size(BabaPlatformWindow* window, int width, int height) {
    if (!window || !window->ns_window) return;
    @autoreleasepool {
        NSRect frame = [window->ns_window frame];
        frame.size.width = width;
        frame.size.height = height;
        [window->ns_window setFrame:frame display:YES];
        window->width = width;
        window->height = height;
    }
}

void baba_platform_window_get_position(BabaPlatformWindow* window, int* x, int* y) {
    if (!window || !window->ns_window) return;
    @autoreleasepool {
        NSRect frame = [window->ns_window frame];
        if (x) *x = (int)frame.origin.x;
        if (y) *y = (int)frame.origin.y;
    }
}

void baba_platform_window_set_position(BabaPlatformWindow* window, int x, int y) {
    if (!window || !window->ns_window) return;
    @autoreleasepool {
        NSRect frame = [window->ns_window frame];
        frame.origin.x = x;
        frame.origin.y = y;
        [window->ns_window setFrame:frame display:YES];
    }
}

void baba_platform_window_show(BabaPlatformWindow* window) {
    if (!window || !window->ns_window) return;
    @autoreleasepool {
        [window->ns_window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];
    }
}

void baba_platform_window_hide(BabaPlatformWindow* window) {
    if (!window || !window->ns_window) return;
    @autoreleasepool {
        [window->ns_window orderOut:nil];
    }
}

void baba_platform_window_close(BabaPlatformWindow* window) {
    if (!window) return;
    window->should_close = true;
}

void* baba_platform_window_get_native(BabaPlatformWindow* window) {
    return window ? (__bridge void*)window->ns_window : NULL;
}

void baba_platform_window_invalidate(BabaPlatformWindow* window) {
    if (!window || !window->ns_view) return;
    @autoreleasepool {
        [window->ns_view setNeedsDisplay:YES];
    }
}

double baba_platform_get_time(void) {
    static double start_time = 0;
    static bool initialized = false;
    
    if (!initialized) {
        start_time = CFAbsoluteTimeGetCurrent();
        initialized = true;
    }
    
    return CFAbsoluteTimeGetCurrent() - start_time;
}

void baba_platform_poll_events(void) {
    @autoreleasepool {
        NSEvent* event;
        while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                           untilDate:nil
                                              inMode:NSDefaultRunLoopMode
                                             dequeue:YES])) {
            [NSApp sendEvent:event];
        }
    }
}

bool baba_platform_window_should_close(BabaPlatformWindow* window) {
    return window ? window->should_close : true;
}

struct BabaPlatformButton {
    NSButton* ns_button;
    BabaButtonCallback callback;
    void* userdata;
};

@interface BabaButtonTarget : NSObject
@property (assign) BabaPlatformButton* button;
@end

@implementation BabaButtonTarget
- (void)onClick:(id)sender {
    if (self.button && self.button->callback) {
        self.button->callback(self.button->userdata);
    }
}
@end

BabaPlatformButton* baba_platform_button_create(
    BabaPlatformWindow* window,
    const char* text,
    float x, float y, float width, float height,
    BabaButtonCallback callback,
    void* userdata
) {
    if (!window || !window->ns_view) return NULL;
    
    @autoreleasepool {
        BabaPlatformButton* button = calloc(1, sizeof(BabaPlatformButton));
        if (!button) return NULL;
        
        button->callback = callback;
        button->userdata = userdata;
        
        NSRect frame = NSMakeRect(x, window->height - y - height, width, height);
        
        BabaButtonTarget* target = [[BabaButtonTarget alloc] init];
        target.button = button;
        
        button->ns_button = [[NSButton alloc] initWithFrame:frame];
        [button->ns_button setTitle:[NSString stringWithUTF8String:text ? text : ""]];
        [button->ns_button setBezelStyle:NSBezelStyleRounded];
        [button->ns_button setTarget:target];
        [button->ns_button setAction:@selector(onClick:)];
        
        [window->ns_view addSubview:button->ns_button];
        
        return button;
    }
}

void baba_platform_button_destroy(BabaPlatformButton* button) {
    if (!button) return;
    @autoreleasepool {
        if (button->ns_button) {
            [button->ns_button removeFromSuperview];
        }
    }
    free(button);
}

void baba_platform_button_set_text(BabaPlatformButton* button, const char* text) {
    if (!button || !button->ns_button) return;
    @autoreleasepool {
        [button->ns_button setTitle:[NSString stringWithUTF8String:text ? text : ""]];
    }
}

struct BabaPlatformTextField {
    NSTextField* ns_field;
    char* text_buffer;
    int buffer_size;
};

BabaPlatformTextField* baba_platform_textfield_create(
    BabaPlatformWindow* window,
    const char* text,
    float x, float y, float width, float height
) {
    if (!window || !window->ns_view) return NULL;
    
    @autoreleasepool {
        BabaPlatformTextField* field = calloc(1, sizeof(BabaPlatformTextField));
        if (!field) return NULL;
        
        field->buffer_size = 256;
        field->text_buffer = calloc(field->buffer_size, sizeof(char));
        
        NSRect frame = NSMakeRect(x, window->height - y - height, width, height);
        
        field->ns_field = [[NSTextField alloc] initWithFrame:frame];
        [field->ns_field setStringValue:[NSString stringWithUTF8String:text ? text : ""]];
        [field->ns_field setBezeled:YES];
        [field->ns_field setEditable:NO];
        [field->ns_field setAlignment:NSTextAlignmentRight];
        [field->ns_field setFont:[NSFont systemFontOfSize:24.0]];
        
        [window->ns_view addSubview:field->ns_field];
        
        if (text) {
            strncpy(field->text_buffer, text, field->buffer_size - 1);
        }
        
        return field;
    }
}

void baba_platform_textfield_destroy(BabaPlatformTextField* field) {
    if (!field) return;
    @autoreleasepool {
        if (field->ns_field) {
            [field->ns_field removeFromSuperview];
        }
        if (field->text_buffer) {
            free(field->text_buffer);
        }
    }
    free(field);
}

void baba_platform_textfield_set_text(BabaPlatformTextField* field, const char* text) {
    if (!field || !field->ns_field) return;
    @autoreleasepool {
        [field->ns_field setStringValue:[NSString stringWithUTF8String:text ? text : ""]];
        if (text && field->text_buffer) {
            strncpy(field->text_buffer, text, field->buffer_size - 1);
        }
    }
}

const char* baba_platform_textfield_get_text(BabaPlatformTextField* field) {
    if (!field) return NULL;
    @autoreleasepool {
        NSString* str = [field->ns_field stringValue];
        if (str && field->text_buffer) {
            strncpy(field->text_buffer, [str UTF8String], field->buffer_size - 1);
        }
        return field->text_buffer;
    }
}