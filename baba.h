#ifndef BABA_H
#define BABA_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BABA_VERSION "0.3.0"

typedef struct BabaApp BabaApp;
typedef struct BabaWindow BabaWindow;
typedef struct BabaButton BabaButton;
typedef struct BabaLabel BabaLabel;
typedef struct BabaTextField BabaTextField;
typedef struct BabaTableView BabaTableView;
typedef struct BabaImageView BabaImageView;

typedef void (*BabaButtonCallback)(BabaButton* button, void* userdata);
typedef void (*BabaTableSelectCallback)(BabaTableView* table, int row, int col, void* userdata);

BabaApp* baba_app_create(void);
void baba_app_destroy(BabaApp* app);
int baba_app_run(BabaApp* app);
void baba_app_quit(BabaApp* app);

BabaWindow* baba_window_create(BabaApp* app, const char* title, int width, int height);
void baba_window_destroy(BabaWindow* window);
void baba_window_set_title(BabaWindow* window, const char* title);
void baba_window_show(BabaWindow* window);
void baba_window_close(BabaWindow* window);
void baba_window_get_size(BabaWindow* window, int* w, int* h);

BabaButton* baba_button_create(BabaWindow* window, const char* text, float x, float y, float w, float h);
void baba_button_destroy(BabaButton* button);
void baba_button_set_text(BabaButton* button, const char* text);
void baba_button_set_callback(BabaButton* button, BabaButtonCallback callback, void* userdata);

BabaLabel* baba_label_create(BabaWindow* window, const char* text, float x, float y, float w, float h);
void baba_label_destroy(BabaLabel* label);
void baba_label_set_text(BabaLabel* label, const char* text);

BabaTextField* baba_textfield_create(BabaWindow* window, const char* text, float x, float y, float w, float h, bool editable);
void baba_textfield_destroy(BabaTextField* field);
void baba_textfield_set_text(BabaTextField* field, const char* text);
const char* baba_textfield_get_text(BabaTextField* field);

BabaTableView* baba_table_create(BabaWindow* window, float x, float y, float w, float h);
void baba_table_destroy(BabaTableView* table);
void baba_table_set_headers(BabaTableView* table, const char** headers, int count);
void baba_table_set_row(BabaTableView* table, int row, const char** values, int count);
void baba_table_add_row(BabaTableView* table, const char** values, int count);
void baba_table_clear(BabaTableView* table);
int baba_table_get_row_count(BabaTableView* table);
void baba_table_get_cell(BabaTableView* table, int row, int col, char* buffer, int bufsize);
void baba_table_set_select_callback(BabaTableView* table, BabaTableSelectCallback callback, void* userdata);

BabaImageView* baba_image_create(BabaWindow* window, float x, float y, float w, float h);
void baba_image_destroy(BabaImageView* image);
bool baba_image_load_file(BabaImageView* image, const char* path);
bool baba_image_save_file(BabaImageView* image, const char* path);
void baba_image_get_size(BabaImageView* image, int* w, int* h);

char* baba_file_open_dialog(BabaWindow* window, const char* title, const char* filter);
char* baba_file_save_dialog(BabaWindow* window, const char* title, const char* filter);
char* baba_file_open_folder_dialog(BabaWindow* window, const char* title);

const char* baba_get_version(void);

#ifdef BABA_IMPLEMENTATION

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#import <Cocoa/Cocoa.h>

struct BabaApp {
    int running;
};

struct BabaWindow {
    BabaApp* app;
    NSWindow* ns_window;
    NSView* ns_view;
    int width;
    int height;
};

struct BabaButton {
    BabaWindow* window;
    NSButton* ns_button;
    BabaButtonCallback callback;
    void* userdata;
};

struct BabaLabel {
    BabaWindow* window;
    NSTextField* ns_label;
};

struct BabaTextField {
    BabaWindow* window;
    NSTextField* ns_field;
    char buffer[1024];
};

struct BabaTableView {
    BabaWindow* window;
    NSScrollView* scroll;
    NSTableView* table;
    NSMutableArray* data;
    BabaTableSelectCallback callback;
    void* userdata;
};

struct BabaImageView {
    BabaWindow* window;
    NSImageView* ns_image;
    NSImage* image;
    char path[512];
};

@interface BabaTarget : NSObject
@property (assign) BabaButton* button;
@property (assign) BabaTableView* table;
@end

@implementation BabaTarget
- (void)onClick:(id)sender {
    if (self.button && self.button->callback) {
        self.button->callback(self.button, self.button->userdata);
    }
}
- (void)tableViewSelectionDidChange:(NSNotification*)notification {
    if (self.table && self.table->callback) {
        NSTableView* tv = self.table->table;
        int row = (int)[tv selectedRow];
        int col = (int)[tv selectedColumn];
        if (row >= 0 && col >= 0) {
            self.table->callback(self.table, row, col, self.table->userdata);
        }
    }
}
@end

@interface BabaDataSource : NSObject <NSTableViewDataSource, NSTableViewDelegate>
@property (assign) BabaTableView* table;
@end

@implementation BabaDataSource
- (NSInteger)numberOfRowsInTableView:(NSTableView*)tv {
    return self.table->data ? [self.table->data count] : 0;
}
- (id)tableView:(NSTableView*)tv objectValueForTableColumn:(NSTableColumn*)col row:(NSInteger)row {
    if (!self.table->data || row >= [self.table->data count]) return @"";
    NSArray* rowData = self.table->data[row];
    NSInteger colIdx = [[col identifier] integerValue];
    if (colIdx < [rowData count]) return rowData[colIdx];
    return @"";
}
@end

BabaApp* baba_app_create(void) {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp finishLaunching];
    BabaApp* app = calloc(1, sizeof(BabaApp));
    app->running = 1;
    return app;
}

void baba_app_destroy(BabaApp* app) { if (app) free(app); }

int baba_app_run(BabaApp* app) {
    while (app->running) {
        @autoreleasepool {
            NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                untilDate:[NSDate dateWithTimeIntervalSinceNow:0.01]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];
            if (event) [NSApp sendEvent:event];
        }
    }
    return 0;
}

void baba_app_quit(BabaApp* app) { if (app) app->running = 0; }

BabaWindow* baba_window_create(BabaApp* app, const char* title, int width, int height) {
    BabaWindow* window = calloc(1, sizeof(BabaWindow));
    window->app = app;
    window->width = width;
    window->height = height;
    
    NSRect frame = NSMakeRect(0, 0, width, height);
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    
    window->ns_window = [[NSWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:NO];
    window->ns_view = [[NSView alloc] initWithFrame:frame];
    [window->ns_window setContentView:window->ns_view];
    [window->ns_window setTitle:[NSString stringWithUTF8String:title ? title : "Baba"]];
    [window->ns_window center];
    return window;
}

void baba_window_destroy(BabaWindow* window) {
    if (!window) return;
    if (window->ns_window) [window->ns_window close];
    free(window);
}

void baba_window_set_title(BabaWindow* window, const char* title) {
    if (window && window->ns_window && title) [window->ns_window setTitle:[NSString stringWithUTF8String:title]];
}

void baba_window_show(BabaWindow* window) {
    if (window && window->ns_window) {
        [window->ns_window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];
    }
}

void baba_window_close(BabaWindow* window) {
    if (window) { window->app->running = 0; if (window->ns_window) [window->ns_window close]; }
}

void baba_window_get_size(BabaWindow* window, int* w, int* h) {
    if (!window) return;
    NSRect frame = [window->ns_view frame];
    if (w) *w = (int)frame.size.width;
    if (h) *h = (int)frame.size.height;
}

BabaButton* baba_button_create(BabaWindow* window, const char* text, float x, float y, float w, float h) {
    BabaButton* button = calloc(1, sizeof(BabaButton));
    button->window = window;
    NSRect frame = NSMakeRect(x, window->height - y - h, w, h);
    button->ns_button = [[NSButton alloc] initWithFrame:frame];
    [button->ns_button setTitle:[NSString stringWithUTF8String:text ? text : ""]];
    [button->ns_button setBezelStyle:NSBezelStyleRounded];
    BabaTarget* target = [[BabaTarget alloc] init];
    target.button = button;
    [button->ns_button setTarget:target];
    [button->ns_button setAction:@selector(onClick:)];
    [window->ns_view addSubview:button->ns_button];
    return button;
}

void baba_button_destroy(BabaButton* button) {
    if (!button) return;
    if (button->ns_button) [button->ns_button removeFromSuperview];
    free(button);
}

void baba_button_set_text(BabaButton* button, const char* text) {
    if (button && button->ns_button) [button->ns_button setTitle:[NSString stringWithUTF8String:text ? text : ""]];
}

void baba_button_set_callback(BabaButton* button, BabaButtonCallback callback, void* userdata) {
    if (button) { button->callback = callback; button->userdata = userdata; }
}

BabaLabel* baba_label_create(BabaWindow* window, const char* text, float x, float y, float w, float h) {
    BabaLabel* label = calloc(1, sizeof(BabaLabel));
    label->window = window;
    NSRect frame = NSMakeRect(x, window->height - y - h, w, h);
    label->ns_label = [[NSTextField alloc] initWithFrame:frame];
    [label->ns_label setStringValue:[NSString stringWithUTF8String:text ? text : ""]];
    [label->ns_label setBezeled:NO];
    [label->ns_label setEditable:NO];
    [label->ns_label setSelectable:NO];
    [window->ns_view addSubview:label->ns_label];
    return label;
}

void baba_label_destroy(BabaLabel* label) {
    if (!label) return;
    if (label->ns_label) [label->ns_label removeFromSuperview];
    free(label);
}

void baba_label_set_text(BabaLabel* label, const char* text) {
    if (label && label->ns_label) [label->ns_label setStringValue:[NSString stringWithUTF8String:text ? text : ""]];
}

BabaTextField* baba_textfield_create(BabaWindow* window, const char* text, float x, float y, float w, float h, bool editable) {
    BabaTextField* field = calloc(1, sizeof(BabaTextField));
    field->window = window;
    NSRect frame = NSMakeRect(x, window->height - y - h, w, h);
    field->ns_field = [[NSTextField alloc] initWithFrame:frame];
    [field->ns_field setStringValue:[NSString stringWithUTF8String:text ? text : ""]];
    [field->ns_field setBezeled:YES];
    [field->ns_field setEditable:editable ? YES : NO];
    [window->ns_view addSubview:field->ns_field];
    return field;
}

void baba_textfield_destroy(BabaTextField* field) {
    if (!field) return;
    if (field->ns_field) [field->ns_field removeFromSuperview];
    free(field);
}

void baba_textfield_set_text(BabaTextField* field, const char* text) {
    if (field && field->ns_field) {
        [field->ns_field setStringValue:[NSString stringWithUTF8String:text ? text : ""]];
        if (text) strncpy(field->buffer, text, 1023);
    }
}

const char* baba_textfield_get_text(BabaTextField* field) {
    if (!field) return NULL;
    NSString* str = [field->ns_field stringValue];
    strncpy(field->buffer, [str UTF8String], 1023);
    return field->buffer;
}

BabaTableView* baba_table_create(BabaWindow* window, float x, float y, float w, float h) {
    BabaTableView* table = calloc(1, sizeof(BabaTableView));
    table->window = window;
    table->data = [[NSMutableArray alloc] init];
    
    NSRect frame = NSMakeRect(x, window->height - y - h, w, h);
    table->scroll = [[NSScrollView alloc] initWithFrame:frame];
    [table->scroll setHasVerticalScroller:YES];
    [table->scroll setHasHorizontalScroller:YES];
    [table->scroll setBorderType:NSBezelBorder];
    
    NSTableView* tv = [[NSTableView alloc] initWithFrame:NSMakeRect(0, 0, w - 20, h - 20)];
    [tv setAllowsColumnReordering:NO];
    [tv setAllowsColumnResizing:YES];
    [tv setAllowsMultipleSelection:NO];
    table->table = tv;
    
    BabaDataSource* ds = [[BabaDataSource alloc] init];
    ds.table = table;
    [tv setDataSource:ds];
    [tv setDelegate:ds];
    
    BabaTarget* target = [[BabaTarget alloc] init];
    target.table = table;
    [tv setTarget:target];
    
    [table->scroll setDocumentView:tv];
    [window->ns_view addSubview:table->scroll];
    return table;
}

void baba_table_destroy(BabaTableView* table) {
    if (!table) return;
    if (table->scroll) [table->scroll removeFromSuperview];
    free(table);
}

void baba_table_set_headers(BabaTableView* table, const char** headers, int count) {
    if (!table || !headers) return;
    while ([table->table numberOfColumns] > 0) [table->table removeTableColumn:[table->table tableColumns][0]];
    for (int i = 0; i < count; i++) {
        NSString* identifier = [NSString stringWithFormat:@"%d", i];
        NSTableColumn* col = [[NSTableColumn alloc] initWithIdentifier:identifier];
        [[col headerCell] setStringValue:[NSString stringWithUTF8String:headers[i]]];
        [col setWidth:100];
        [table->table addTableColumn:col];
    }
}

void baba_table_set_row(BabaTableView* table, int row, const char** values, int count) {
    if (!table || !values || row < 0) return;
    NSMutableArray* rowData = [NSMutableArray arrayWithCapacity:count];
    for (int i = 0; i < count; i++) [rowData addObject:[NSString stringWithUTF8String:values[i] ? values[i] : ""]];
    if (row < [table->data count]) table->data[row] = rowData;
    else [table->data addObject:rowData];
    [table->table reloadData];
}

void baba_table_add_row(BabaTableView* table, const char** values, int count) {
    if (!table || !values) return;
    NSMutableArray* rowData = [NSMutableArray arrayWithCapacity:count];
    for (int i = 0; i < count; i++) [rowData addObject:[NSString stringWithUTF8String:values[i] ? values[i] : ""]];
    [table->data addObject:rowData];
    [table->table reloadData];
}

void baba_table_clear(BabaTableView* table) {
    if (!table) return;
    [table->data removeAllObjects];
    [table->table reloadData];
}

int baba_table_get_row_count(BabaTableView* table) {
    return table ? (int)[table->data count] : 0;
}

void baba_table_get_cell(BabaTableView* table, int row, int col, char* buffer, int bufsize) {
    if (!table || !buffer || row < 0 || col < 0) return;
    if (row < [table->data count]) {
        NSArray* rowData = table->data[row];
        if (col < [rowData count]) strncpy(buffer, [rowData[col] UTF8String], bufsize - 1);
    }
}

void baba_table_set_select_callback(BabaTableView* table, BabaTableSelectCallback callback, void* userdata) {
    if (table) { table->callback = callback; table->userdata = userdata; }
}

BabaImageView* baba_image_create(BabaWindow* window, float x, float y, float w, float h) {
    BabaImageView* image = calloc(1, sizeof(BabaImageView));
    image->window = window;
    NSRect frame = NSMakeRect(x, window->height - y - h, w, h);
    image->ns_image = [[NSImageView alloc] initWithFrame:frame];
    [image->ns_image setImageScaling:NSImageScaleProportionallyUpOrDown];
    [image->ns_image setImageAlignment:NSImageAlignCenter];
    [window->ns_view addSubview:image->ns_image];
    return image;
}

void baba_image_destroy(BabaImageView* image) {
    if (!image) return;
    if (image->ns_image) [image->ns_image removeFromSuperview];
    free(image);
}

bool baba_image_load_file(BabaImageView* image, const char* path) {
    if (!image || !path) return false;
    NSString* nsPath = [NSString stringWithUTF8String:path];
    NSImage* img = [[NSImage alloc] initWithContentsOfFile:nsPath];
    if (!img) return false;
    image->image = img;
    [image->ns_image setImage:img];
    strncpy(image->path, path, 511);
    return true;
}

bool baba_image_save_file(BabaImageView* image, const char* path) {
    if (!image || !image->image || !path) return false;
    NSString* nsPath = [NSString stringWithUTF8String:path];
    NSData* data = [image->image TIFFRepresentation];
    NSBitmapImageRep* rep = [NSBitmapImageRep imageRepWithData:data];
    NSData* png = [rep representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
    return [png writeToFile:nsPath atomically:YES];
}

void baba_image_get_size(BabaImageView* image, int* w, int* h) {
    if (!image || !image->image) return;
    NSSize size = [image->image size];
    if (w) *w = (int)size.width;
    if (h) *h = (int)size.height;
}

char* baba_file_open_dialog(BabaWindow* window, const char* title, const char* filter) {
    @autoreleasepool {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setTitle:[NSString stringWithUTF8String:title ? title : "Open"]];
        [panel setAllowsMultipleSelection:NO];
        [panel setCanChooseDirectories:NO];
        [panel setCanChooseFiles:YES];
        if (filter) {
            NSString* f = [NSString stringWithUTF8String:filter];
            NSArray* exts = [f componentsSeparatedByString:@","];
            [panel setAllowedFileTypes:exts];
        }
        if ([panel runModal] == NSModalResponseOK) {
            NSURL* url = [panel URL];
            const char* path = [[url path] UTF8String];
            char* result = malloc(strlen(path) + 1);
            strcpy(result, path);
            return result;
        }
        return NULL;
    }
}

char* baba_file_save_dialog(BabaWindow* window, const char* title, const char* filter) {
    @autoreleasepool {
        NSSavePanel* panel = [NSSavePanel savePanel];
        [panel setTitle:[NSString stringWithUTF8String:title ? title : "Save"]];
        if (filter) {
            NSString* f = [NSString stringWithUTF8String:filter];
            NSArray* exts = [f componentsSeparatedByString:@","];
            [panel setAllowedFileTypes:exts];
        }
        if ([panel runModal] == NSModalResponseOK) {
            NSURL* url = [panel URL];
            const char* path = [[url path] UTF8String];
            char* result = malloc(strlen(path) + 1);
            strcpy(result, path);
            return result;
        }
        return NULL;
    }
}

char* baba_file_open_folder_dialog(BabaWindow* window, const char* title) {
    @autoreleasepool {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setTitle:[NSString stringWithUTF8String:title ? title : "Select Folder"]];
        [panel setCanChooseDirectories:YES];
        [panel setCanChooseFiles:NO];
        if ([panel runModal] == NSModalResponseOK) {
            NSURL* url = [panel URL];
            const char* path = [[url path] UTF8String];
            char* result = malloc(strlen(path) + 1);
            strcpy(result, path);
            return result;
        }
        return NULL;
    }
}

const char* baba_get_version(void) { return BABA_VERSION; }

#elif defined(_WIN32)

#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>

struct BabaApp { int running; HINSTANCE instance; };
struct BabaWindow { BabaApp* app; HWND hwnd; int width; int height; };
struct BabaButton { BabaWindow* window; HWND hwnd; BabaButtonCallback callback; void* userdata; int id; };
struct BabaLabel { BabaWindow* window; HWND hwnd; };
struct BabaTextField { BabaWindow* window; HWND hwnd; char buffer[1024]; };
struct BabaTableView { BabaWindow* window; HWND hwnd; char*** data; int rows; int cols; };
struct BabaImageView { BabaWindow* window; HWND hwnd; char path[512]; int img_w; int img_h; };

static int g_id = 1000;
static BabaButton* g_buttons[200];
static HWND g_table_edit = NULL;
static BabaTableView* g_current_table = NULL;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_COMMAND) {
        int id = LOWORD(wParam);
        if (HIWORD(wParam) == BN_CLICKED && id >= 1000 && id < 1200) {
            if (g_buttons[id-1000] && g_buttons[id-1000]->callback)
                g_buttons[id-1000]->callback(g_buttons[id-1000], g_buttons[id-1000]->userdata);
        }
    }
    if (msg == WM_CLOSE) { PostQuitMessage(0); return 0; }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

BabaApp* baba_app_create(void) {
    BabaApp* app = calloc(1, sizeof(BabaApp));
    app->running = 1;
    app->instance = GetModuleHandleW(NULL);
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = app->instance;
    wc.lpszClassName = L"BabaWindow";
    RegisterClassExW(&wc);
    return app;
}

void baba_app_destroy(BabaApp* app) { if (app) free(app); }

int baba_app_run(BabaApp* app) {
    MSG msg;
    while (app->running && GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

void baba_app_quit(BabaApp* app) { if (app) app->running = 0; PostQuitMessage(0); }

BabaWindow* baba_window_create(BabaApp* app, const char* title, int width, int height) {
    BabaWindow* window = calloc(1, sizeof(BabaWindow));
    window->app = app;
    window->width = width;
    window->height = height;
    wchar_t wtitle[256];
    MultiByteToWideChar(CP_UTF8, 0, title ? title : "Baba", -1, wtitle, 256);
    window->hwnd = CreateWindowExW(0, L"BabaWindow", wtitle,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, app->instance, NULL);
    return window;
}

void baba_window_destroy(BabaWindow* window) { if (!window) return; if (window->hwnd) DestroyWindow(window->hwnd); free(window); }
void baba_window_set_title(BabaWindow* window, const char* title) {
    if (window && window->hwnd && title) {
        wchar_t w[256]; MultiByteToWideChar(CP_UTF8, 0, title, -1, w, 256); SetWindowTextW(window->hwnd, w);
    }
}
void baba_window_show(BabaWindow* window) { if (window && window->hwnd) ShowWindow(window->hwnd, SW_SHOW); UpdateWindow(window->hwnd); }
void baba_window_close(BabaWindow* window) { if (window) { window->app->running = 0; PostMessageW(window->hwnd, WM_CLOSE, 0, 0); } }
void baba_window_get_size(BabaWindow* window, int* w, int* h) {
    if (!window) return; RECT r; GetClientRect(window->hwnd, &r);
    if (w) *w = r.right; if (h) *h = r.bottom;
}

BabaButton* baba_button_create(BabaWindow* window, const char* text, float x, float y, float w, float h) {
    BabaButton* button = calloc(1, sizeof(BabaButton));
    button->window = window;
    button->id = g_id++;
    wchar_t wtext[256]; MultiByteToWideChar(CP_UTF8, 0, text ? text : "", -1, wtext, 256);
    button->hwnd = CreateWindowExW(0, L"BUTTON", wtext, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        (int)x, (int)y, (int)w, (int)h, window->hwnd, (HMENU)button->id, window->app->instance, NULL);
    g_buttons[button->id-1000] = button;
    return button;
}
void baba_button_destroy(BabaButton* button) { if (!button) return; if (button->hwnd) DestroyWindow(button->hwnd); free(button); }
void baba_button_set_text(BabaButton* button, const char* text) {
    if (button && button->hwnd) { wchar_t w[256]; MultiByteToWideChar(CP_UTF8, 0, text ? text : "", -1, w, 256); SetWindowTextW(button->hwnd, w); }
}
void baba_button_set_callback(BabaButton* button, BabaButtonCallback callback, void* userdata) {
    if (button) { button->callback = callback; button->userdata = userdata; }
}

BabaLabel* baba_label_create(BabaWindow* window, const char* text, float x, float y, float w, float h) {
    BabaLabel* label = calloc(1, sizeof(BabaLabel));
    label->window = window;
    wchar_t wtext[256]; MultiByteToWideChar(CP_UTF8, 0, text ? text : "", -1, wtext, 256);
    label->hwnd = CreateWindowExW(0, L"STATIC", wtext, WS_CHILD | WS_VISIBLE, (int)x, (int)y, (int)w, (int)h, window->hwnd, NULL, window->app->instance, NULL);
    return label;
}
void baba_label_destroy(BabaLabel* label) { if (!label) return; if (label->hwnd) DestroyWindow(label->hwnd); free(label); }
void baba_label_set_text(BabaLabel* label, const char* text) {
    if (label && label->hwnd) { wchar_t w[256]; MultiByteToWideChar(CP_UTF8, 0, text ? text : "", -1, w, 256); SetWindowTextW(label->hwnd, w); }
}

BabaTextField* baba_textfield_create(BabaWindow* window, const char* text, float x, float y, float w, float h, bool editable) {
    BabaTextField* field = calloc(1, sizeof(BabaTextField));
    field->window = window;
    wchar_t wtext[1024]; MultiByteToWideChar(CP_UTF8, 0, text ? text : "", -1, wtext, 1024);
    field->hwnd = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", wtext, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | (editable ? 0 : ES_READONLY),
        (int)x, (int)y, (int)w, (int)h, window->hwnd, NULL, window->app->instance, NULL);
    return field;
}
void baba_textfield_destroy(BabaTextField* field) { if (!field) return; if (field->hwnd) DestroyWindow(field->hwnd); free(field); }
void baba_textfield_set_text(BabaTextField* field, const char* text) {
    if (field && field->hwnd) { wchar_t w[1024]; MultiByteToWideChar(CP_UTF8, 0, text ? text : "", -1, w, 1024); SetWindowTextW(field->hwnd, w); }
}
const char* baba_textfield_get_text(BabaTextField* field) {
    if (!field) return NULL;
    wchar_t w[1024]; GetWindowTextW(field->hwnd, w, 1024); WideCharToMultiByte(CP_UTF8, 0, w, -1, field->buffer, 1024, NULL, NULL);
    return field->buffer;
}

BabaTableView* baba_table_create(BabaWindow* window, float x, float y, float w, float h) {
    BabaTableView* table = calloc(1, sizeof(BabaTableView));
    table->window = window;
    table->hwnd = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | LBS_NOINTEGRALHEIGHT | WS_VSCROLL,
        (int)x, (int)y, (int)w, (int)h, window->hwnd, NULL, window->app->instance, NULL);
    return table;
}
void baba_table_destroy(BabaTableView* table) { if (!table) return; if (table->hwnd) DestroyWindow(table->hwnd); free(table); }
void baba_table_set_headers(BabaTableView* table, const char** headers, int count) { }
void baba_table_set_row(BabaTableView* table, int row, const char** values, int count) { }
void baba_table_add_row(BabaTableView* table, const char** values, int count) {
    if (!table || !values) return;
    char line[1024] = ""; for (int i = 0; i < count && i < 10; i++) { strcat(line, values[i] ? values[i] : ""); if (i < count-1) strcat(line, "\t"); }
    wchar_t wline[1024]; MultiByteToWideChar(CP_UTF8, 0, line, -1, wline, 1024);
    SendMessageW(table->hwnd, LB_ADDSTRING, 0, (LPARAM)wline);
}
void baba_table_clear(BabaTableView* table) { if (table && table->hwnd) SendMessageW(table->hwnd, LB_RESETCONTENT, 0, 0); }
int baba_table_get_row_count(BabaTableView* table) { return table ? (int)SendMessageW(table->hwnd, LB_GETCOUNT, 0, 0) : 0; }
void baba_table_get_cell(BabaTableView* table, int row, int col, char* buffer, int bufsize) { }
void baba_table_set_select_callback(BabaTableView* table, BabaTableSelectCallback callback, void* userdata) { }

BabaImageView* baba_image_create(BabaWindow* window, float x, float y, float w, float h) {
    BabaImageView* image = calloc(1, sizeof(BabaImageView));
    image->window = window;
    image->hwnd = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_BITMAP, (int)x, (int)y, (int)w, (int)h, window->hwnd, NULL, window->app->instance, NULL);
    return image;
}
void baba_image_destroy(BabaImageView* image) { if (!image) return; if (image->hwnd) DestroyWindow(image->hwnd); free(image); }
bool baba_image_load_file(BabaImageView* image, const char* path) {
    if (!image || !path) return false;
    wchar_t wpath[512]; MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 512);
    HBITMAP bmp = (HBITMAP)LoadImageW(NULL, wpath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (!bmp) return false;
    SendMessageW(image->hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);
    strncpy(image->path, path, 511);
    return true;
}
bool baba_image_save_file(BabaImageView* image, const char* path) { return false; }
void baba_image_get_size(BabaImageView* image, int* w, int* h) { if (w) *w = image->img_w; if (h) *h = image->img_h; }

char* baba_file_open_dialog(BabaWindow* window, const char* title, const char* filter) {
    wchar_t path[MAX_PATH] = {0};
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = window ? window->hwnd : NULL;
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    if (filter) {
        wchar_t wfilter[256]; wchar_t wfilter2[256];
        MultiByteToWideChar(CP_UTF8, 0, filter, -1, wfilter2, 256);
        wcscpy(wfilter, wfilter2); wcscat(wfilter, L"\0"); wcscat(wfilter, wfilter2); wcscat(wfilter, L"\0");
        ofn.lpstrFilter = wfilter;
    }
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameW(&ofn)) {
        int len = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
        char* result = malloc(len); WideCharToMultiByte(CP_UTF8, 0, path, -1, result, len, NULL, NULL);
        return result;
    }
    return NULL;
}

char* baba_file_save_dialog(BabaWindow* window, const char* title, const char* filter) {
    wchar_t path[MAX_PATH] = {0};
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = window ? window->hwnd : NULL;
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    if (filter) {
        wchar_t wfilter[256]; wchar_t wfilter2[256];
        MultiByteToWideChar(CP_UTF8, 0, filter, -1, wfilter2, 256);
        wcscpy(wfilter, wfilter2); wcscat(wfilter, L"\0"); wcscat(wfilter, wfilter2); wcscat(wfilter, L"\0");
        ofn.lpstrFilter = wfilter;
    }
    ofn.Flags = OFN_OVERWRITEPROMPT;
    if (GetSaveFileNameW(&ofn)) {
        int len = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
        char* result = malloc(len); WideCharToMultiByte(CP_UTF8, 0, path, -1, result, len, NULL, NULL);
        return result;
    }
    return NULL;
}

char* baba_file_open_folder_dialog(BabaWindow* window, const char* title) {
    BROWSEINFOW bi = {0};
    bi.hwndOwner = window ? window->hwnd : NULL;
    bi.lpszTitle = L"Select Folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);
    if (!pidl) return NULL;
    wchar_t path[MAX_PATH];
    SHGetPathFromIDListW(pidl, path);
    CoTaskMemFree(pidl);
    int len = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
    char* result = malloc(len); WideCharToMultiByte(CP_UTF8, 0, path, -1, result, len, NULL, NULL);
    return result;
}

const char* baba_get_version(void) { return BABA_VERSION; }

#elif defined(__linux__)

#include <X11/Xlib.h>
#include <unistd.h>
#include <sys/stat.h>

struct BabaApp { int running; Display* display; };
struct BabaWindow { BabaApp* app; Window window; int width; int height; GC gc; };
struct BabaButton { BabaWindow* window; int x, y, w, h; char text[64]; BabaButtonCallback callback; void* userdata; };
struct BabaLabel { BabaWindow* window; int x, y, w, h; char text[256]; };
struct BabaTextField { BabaWindow* window; int x, y, w, h; char buffer[1024]; };
struct BabaTableView { BabaWindow* window; int x, y, w, h; char** rows; int row_count; };
struct BabaImageView { BabaWindow* window; int x, y, w, h; char path[512]; };

static BabaButton* g_buttons[200];
static int g_btn_count = 0;
static BabaLabel* g_labels[100];
static int g_label_count = 0;
static BabaTextField* g_fields[100];
static int g_field_count = 0;
static BabaTableView* g_tables[50];
static int g_table_count = 0;
static BabaImageView* g_images[50];
static int g_image_count = 0;
static XFontStruct* g_font = NULL;

static void draw_btn(BabaButton* btn, Display* d, Window w, GC gc) {
    XClearArea(d, w, btn->x, btn->y, btn->w, btn->h, False);
    XDrawRectangle(d, w, gc, btn->x, btn->y, btn->w-1, btn->h-1);
    if (!g_font) g_font = XLoadQueryFont(d, "fixed");
    XSetFont(d, gc, g_font->fid);
    XDrawString(d, w, gc, btn->x+10, btn->y+btn->h/2+5, btn->text, strlen(btn->text));
}

static void draw_lbl(BabaLabel* lbl, Display* d, Window w, GC gc) {
    XClearArea(d, w, lbl->x, lbl->y, lbl->w, lbl->h, False);
    if (!g_font) g_font = XLoadQueryFont(d, "fixed");
    XSetFont(d, gc, g_font->fid);
    XDrawString(d, w, gc, lbl->x+5, lbl->y+lbl->h/2+5, lbl->text, strlen(lbl->text));
}

static void draw_field(BabaTextField* f, Display* d, Window w, GC gc) {
    XClearArea(d, w, f->x, f->y, f->w, f->h, False);
    XDrawRectangle(d, w, gc, f->x, f->y, f->w-1, f->h-1);
    if (!g_font) g_font = XLoadQueryFont(d, "fixed");
    XSetFont(d, gc, g_font->fid);
    XDrawString(d, w, gc, f->x+5, f->y+f->h/2+5, f->buffer, strlen(f->buffer));
}

static void draw_table(BabaTableView* t, Display* d, Window w, GC gc) {
    XClearArea(d, w, t->x, t->y, t->w, t->h, False);
    XDrawRectangle(d, w, gc, t->x, t->y, t->w-1, t->h-1);
    if (!g_font) g_font = XLoadQueryFont(d, "fixed");
    XSetFont(d, gc, g_font->fid);
    int row_h = 20;
    for (int i = 0; i < t->row_count && i < 20; i++) {
        int yy = t->y + 5 + i * row_h;
        XDrawString(d, w, gc, t->x+5, yy+10, t->rows[i], strlen(t->rows[i]));
    }
}

BabaApp* baba_app_create(void) {
    BabaApp* app = calloc(1, sizeof(BabaApp));
    app->running = 1;
    app->display = XOpenDisplay(NULL);
    return app;
}
void baba_app_destroy(BabaApp* app) { if (app) { if (app->display) XCloseDisplay(app->display); free(app); } }

int baba_app_run(BabaApp* app) {
    if (!app || !app->display) return -1;
    while (app->running) {
        XEvent e;
        while (XPending(app->display)) {
            XNextEvent(app->display, &e);
            if (e.type == Expose) {
                for (int i = 0; i < g_btn_count; i++) draw_btn(g_buttons[i], app->display, e.xexpose.window, app->gc);
                for (int i = 0; i < g_label_count; i++) draw_lbl(g_labels[i], app->display, e.xexpose.window, app->gc);
                for (int i = 0; i < g_field_count; i++) draw_field(g_fields[i], app->display, e.xexpose.window, app->gc);
                for (int i = 0; i < g_table_count; i++) draw_table(g_tables[i], app->display, e.xexpose.window, app->gc);
            }
            if (e.type == ButtonPress) {
                int x = e.xbutton.x, y = e.xbutton.y;
                for (int i = 0; i < g_btn_count; i++) {
                    BabaButton* btn = g_buttons[i];
                    if (x >= btn->x && x < btn->x+btn->w && y >= btn->y && y < btn->y+btn->h && btn->callback)
                        btn->callback(btn, btn->userdata);
                }
            }
            if (e.type == ClientMessage && (Atom)e.xclient.data.l[0] == XInternAtom(app->display, "WM_DELETE_WINDOW", False))
                app->running = 0;
        }
        usleep(10000);
    }
    return 0;
}
void baba_app_quit(BabaApp* app) { if (app) app->running = 0; }

BabaWindow* baba_window_create(BabaApp* app, const char* title, int width, int height) {
    BabaWindow* window = calloc(1, sizeof(BabaWindow));
    window->app = app;
    window->width = width;
    window->height = height;
    int screen = DefaultScreen(app->display);
    window->window = XCreateSimpleWindow(app->display, RootWindow(app->display, screen), 0, 0, width, height, 1,
        BlackPixel(app->display, screen), WhitePixel(app->display, screen));
    XStoreName(app->display, window->window, title ? title : "Baba");
    XSelectInput(app->display, window->window, ExposureMask | ButtonPressMask | StructureNotifyMask);
    Atom wm_delete = XInternAtom(app->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(app->display, window->window, &wm_delete, 1);
    window->gc = XCreateGC(app->display, window->window, 0, NULL);
    return window;
}
void baba_window_destroy(BabaWindow* window) { if (!window) return; if (window->window) XDestroyWindow(window->app->display, window->window); free(window); }
void baba_window_set_title(BabaWindow* window, const char* title) { if (window && window->window && title) XStoreName(window->app->display, window->window, title); }
void baba_window_show(BabaWindow* window) { if (window && window->window) XMapWindow(window->app->display, window->window); }
void baba_window_close(BabaWindow* window) { if (window) window->app->running = 0; }
void baba_window_get_size(BabaWindow* window, int* w, int* h) { if (w) *w = window->width; if (h) *h = window->height; }

BabaButton* baba_button_create(BabaWindow* window, const char* text, float x, float y, float w, float h) {
    BabaButton* btn = calloc(1, sizeof(BabaButton));
    btn->window = window; btn->x = (int)x; btn->y = (int)y; btn->w = (int)w; btn->h = (int)h;
    strncpy(btn->text, text ? text : "", 63);
    g_buttons[g_btn_count++] = btn;
    return btn;
}
void baba_button_destroy(BabaButton* btn) { if (!btn) return; for (int i=0; i<g_btn_count; i++) if (g_buttons[i]==btn) { g_buttons[i]=g_buttons[--g_btn_count]; break; } free(btn); }
void baba_button_set_text(BabaButton* btn, const char* text) { if (btn) { strncpy(btn->text, text ? text : "", 63); draw_btn(btn, btn->window->app->display, btn->window->window, btn->window->gc); } }
void baba_button_set_callback(BabaButton* btn, BabaButtonCallback cb, void* ud) { if (btn) { btn->callback = cb; btn->userdata = ud; } }

BabaLabel* baba_label_create(BabaWindow* window, const char* text, float x, float y, float w, float h) {
    BabaLabel* lbl = calloc(1, sizeof(BabaLabel));
    lbl->window = window; lbl->x = (int)x; lbl->y = (int)y; lbl->w = (int)w; lbl->h = (int)h;
    strncpy(lbl->text, text ? text : "", 255);
    g_labels[g_label_count++] = lbl;
    return lbl;
}
void baba_label_destroy(BabaLabel* lbl) { if (!lbl) return; for (int i=0; i<g_label_count; i++) if (g_labels[i]==lbl) { g_labels[i]=g_labels[--g_label_count]; break; } free(lbl); }
void baba_label_set_text(BabaLabel* lbl, const char* text) { if (lbl) { strncpy(lbl->text, text ? text : "", 255); draw_lbl(lbl, lbl->window->app->display, lbl->window->window, lbl->window->gc); } }

BabaTextField* baba_textfield_create(BabaWindow* window, const char* text, float x, float y, float w, float h, bool editable) {
    BabaTextField* f = calloc(1, sizeof(BabaTextField));
    f->window = window; f->x = (int)x; f->y = (int)y; f->w = (int)w; f->h = (int)h;
    strncpy(f->buffer, text ? text : "", 1023);
    g_fields[g_field_count++] = f;
    return f;
}
void baba_textfield_destroy(BabaTextField* f) { if (!f) return; for (int i=0; i<g_field_count; i++) if (g_fields[i]==f) { g_fields[i]=g_fields[--g_field_count]; break; } free(f); }
void baba_textfield_set_text(BabaTextField* f, const char* text) { if (f) { strncpy(f->buffer, text ? text : "", 1023); draw_field(f, f->window->app->display, f->window->window, f->window->gc); } }
const char* baba_textfield_get_text(BabaTextField* f) { return f ? f->buffer : NULL; }

BabaTableView* baba_table_create(BabaWindow* window, float x, float y, float w, float h) {
    BabaTableView* t = calloc(1, sizeof(BabaTableView));
    t->window = window; t->x = (int)x; t->y = (int)y; t->w = (int)w; t->h = (int)h; t->rows = NULL; t->row_count = 0;
    g_tables[g_table_count++] = t;
    return t;
}
void baba_table_destroy(BabaTableView* t) { if (!t) return; for (int i=0; i<t->row_count; i++) free(t->rows[i]); free(t->rows); free(t); }
void baba_table_set_headers(BabaTableView* t, const char** headers, int count) { }
void baba_table_set_row(BabaTableView* t, int row, const char** values, int count) { }
void baba_table_add_row(BabaTableView* t, const char** values, int count) {
    if (!t || !values) return;
    char line[1024] = ""; for (int i = 0; i < count && i < 20; i++) { strcat(line, values[i] ? values[i] : ""); if (i < count-1) strcat(line, " | "); }
    t->rows = realloc(t->rows, sizeof(char*) * (t->row_count + 1));
    t->rows[t->row_count] = malloc(strlen(line)+1); strcpy(t->rows[t->row_count], line);
    t->row_count++;
    draw_table(t, t->window->app->display, t->window->window, t->window->gc);
}
void baba_table_clear(BabaTableView* t) { if (!t) return; for (int i=0; i<t->row_count; i++) free(t->rows[i]); free(t->rows); t->rows=NULL; t->row_count=0; draw_table(t, t->window->app->display, t->window->window, t->window->gc); }
int baba_table_get_row_count(BabaTableView* t) { return t ? t->row_count : 0; }
void baba_table_get_cell(BabaTableView* t, int row, int col, char* buf, int size) { }
void baba_table_set_select_callback(BabaTableView* t, BabaTableSelectCallback cb, void* ud) { }

BabaImageView* baba_image_create(BabaWindow* window, float x, float y, float w, float h) {
    BabaImageView* img = calloc(1, sizeof(BabaImageView));
    img->window = window; img->x = (int)x; img->y = (int)y; img->w = (int)w; img->h = (int)h;
    g_images[g_image_count++] = img;
    return img;
}
void baba_image_destroy(BabaImageView* img) { if (!img) return; free(img); }
bool baba_image_load_file(BabaImageView* img, const char* path) { if (!img || !path) return false; strncpy(img->path, path, 511); return true; }
bool baba_image_save_file(BabaImageView* img, const char* path) { return false; }
void baba_image_get_size(BabaImageView* img, int* w, int* h) { if (w) *w = img->w; if (h) *h = img->h; }

char* baba_file_open_dialog(BabaWindow* window, const char* title, const char* filter) { return NULL; }
char* baba_file_save_dialog(BabaWindow* window, const char* title, const char* filter) { return NULL; }
char* baba_file_open_folder_dialog(BabaWindow* window, const char* title) { return NULL; }

const char* baba_get_version(void) { return BABA_VERSION; }

#endif
#endif
#endif