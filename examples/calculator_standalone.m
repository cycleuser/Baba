#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>

static NSWindow* g_window = nil;
static NSTextField* g_display = nil;
static NSString* g_display_text = @"0";
static double g_stored_value = 0;
static double g_current_value = 0;
static char g_current_op = 0;
static bool g_new_input = true;

static void update_display() {
    if (g_display) g_display.stringValue = g_display_text;
}

static void digit_pressed(const char* digit) {
    if (g_new_input) {
        g_display_text = [NSString stringWithUTF8String:digit];
        g_new_input = false;
    } else {
        NSString* current = g_display_text;
        if ([current isEqualToString:@"0"]) {
            g_display_text = [NSString stringWithUTF8String:digit];
        } else if ([current length] < 12) {
            g_display_text = [current stringByAppendingString:[NSString stringWithUTF8String:digit]];
        }
    }
    update_display();
}

static void clear_pressed() {
    g_display_text = @"0";
    g_stored_value = 0;
    g_current_value = 0;
    g_current_op = 0;
    g_new_input = true;
    update_display();
}

static void operator_pressed(char op) {
    g_current_value = [g_display_text doubleValue];
    g_stored_value = g_current_value;
    g_current_op = op;
    g_new_input = true;
}

static void equals_pressed() {
    double second = [g_display_text doubleValue];
    double result = 0;
    switch (g_current_op) {
        case '+': result = g_stored_value + second; break;
        case '-': result = g_stored_value - second; break;
        case '*': result = g_stored_value * second; break;
        case '/': 
            if (second != 0) result = g_stored_value / second;
            else { g_display_text = @"错误"; update_display(); g_new_input = true; return; }
            break;
        default: result = second; break;
    }
    if (result == (long long)result && fabs(result) < 1e12) {
        g_display_text = [NSString stringWithFormat:@"%lld", (long long)result];
    } else {
        g_display_text = [NSString stringWithFormat:@"%.8g", result];
    }
    g_current_value = result;
    g_current_op = 0;
    g_new_input = true;
    update_display();
}

static void percent_pressed() {
    g_display_text = [NSString stringWithFormat:@"%.8g", [g_display_text doubleValue] / 100.0];
    update_display();
}

static void negate_pressed() {
    g_display_text = [NSString stringWithFormat:@"%.8g", -[g_display_text doubleValue]];
    update_display();
}

static void decimal_pressed() {
    NSString* current = g_display_text;
    if (g_new_input) {
        g_display_text = @"0.";
        g_new_input = false;
    } else if ([current rangeOfString:@"."].location == NSNotFound) {
        g_display_text = [current stringByAppendingString:@"."];
    }
    update_display();
}

@interface CalcButton : NSButton
@property (nonatomic, assign) int buttonType; // 0=digit, 1=op, 2=func
@end

@implementation CalcButton
- (void)drawRect:(NSRect)rect {
    NSColor* bgColor;
    NSColor* textColor;
    
    if (self.buttonType == 1) { // 运算符 - 橙色
        bgColor = [NSColor colorWithRed:1.0 green:0.58 blue:0.0 alpha:1.0];
        textColor = [NSColor whiteColor];
    } else if (self.buttonType == 2) { // 功能键 - 浅灰
        bgColor = [NSColor colorWithRed:0.75 green:0.75 blue:0.75 alpha:1.0];
        textColor = [NSColor blackColor];
    } else { // 数字 - 深灰
        bgColor = [NSColor colorWithRed:0.33 green:0.33 blue:0.35 alpha:1.0];
        textColor = [NSColor whiteColor];
    }
    
    // 按下效果
    if (self.state == NSControlStateValueOn) {
        bgColor = [bgColor colorWithAlphaComponent:0.6];
    }
    
    // 圆角背景
    NSBezierPath* path = [NSBezierPath bezierPathWithRoundedRect:self.bounds xRadius:12 yRadius:12];
    [bgColor setFill];
    [path fill];
    
    // 文字
    NSDictionary* attrs = @{
        NSFontAttributeName: [NSFont systemFontOfSize:26 weight:NSFontWeightMedium],
        NSForegroundColorAttributeName: textColor
    };
    NSSize size = [self.title sizeWithAttributes:attrs];
    NSPoint point = NSMakePoint(
        (self.bounds.size.width - size.width) / 2,
        (self.bounds.size.height - size.height) / 2
    );
    [self.title drawAtPoint:point withAttributes:attrs];
}
@end

@interface CalculatorDelegate : NSObject <NSApplicationDelegate>
@end

@implementation CalculatorDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
    CGFloat windowWidth = 320;
    CGFloat windowHeight = 480;
    
    NSRect frame = NSMakeRect(200, 200, windowWidth, windowHeight);
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
    
    g_window = [[NSWindow alloc] initWithContentRect:frame
                                          styleMask:style
                                            backing:NSBackingStoreBuffered
                                              defer:NO];
    g_window.title = @"把拔计算器";
    g_window.titlebarAppearsTransparent = NO;
    
    NSView* content = g_window.contentView;
    content.wantsLayer = YES;
    content.layer.backgroundColor = [NSColor blackColor].CGColor;
    
    // 显示屏
    CGFloat displayY = windowHeight - 90;
    g_display = [[NSTextField alloc] initWithFrame:NSMakeRect(20, displayY, windowWidth - 40, 70)];
    g_display.stringValue = @"0";
    g_display.font = [NSFont systemFontOfSize:52 weight:NSFontWeightLight];
    g_display.alignment = NSTextAlignmentRight;
    g_display.bezeled = NO;
    g_display.drawsBackground = NO;
    g_display.editable = NO;
    g_display.selectable = NO;
    g_display.textColor = [NSColor whiteColor];
    [content addSubview:g_display];
    
    // 按钮配置
    CGFloat btnSize = 65;
    CGFloat gap = 8;
    CGFloat marginX = 20;
    CGFloat marginY = 20;
    
    // 按钮数据: title, action, type
    struct BtnData {
        const char* title;
        SEL action;
        int type;
        CGFloat width; // 0 = 默认
    };
    
    struct BtnData buttons[5][4] = {
        {{"C", @selector(clearPressed:), 2, 0}, {"±", @selector(negatePressed:), 2, 0}, {"%", @selector(percentPressed:), 2, 0}, {"÷", @selector(operatorPressed:), 1, 0}},
        {{"7", @selector(digitPressed:), 0, 0}, {"8", @selector(digitPressed:), 0, 0}, {"9", @selector(digitPressed:), 0, 0}, {"×", @selector(operatorPressed:), 1, 0}},
        {{"4", @selector(digitPressed:), 0, 0}, {"5", @selector(digitPressed:), 0, 0}, {"6", @selector(digitPressed:), 0, 0}, {"−", @selector(operatorPressed:), 1, 0}},
        {{"1", @selector(digitPressed:), 0, 0}, {"2", @selector(digitPressed:), 0, 0}, {"3", @selector(digitPressed:), 0, 0}, {"+", @selector(operatorPressed:), 1, 0}},
        {{"0", @selector(digitPressed:), 0, 0}, {".", @selector(decimalPressed:), 0, 0}, {"=", @selector(equalsPressed:), 1, 0}, {NULL, NULL, 0, 0}},
    };
    
    CGFloat startY = displayY - 10 - btnSize;
    
    for (int row = 0; row < 5; row++) {
        CGFloat y = startY - row * (btnSize + gap);
        CGFloat x = marginX;
        
        for (int col = 0; col < 4; col++) {
            if (buttons[row][col].title == NULL) continue;
            
            CGFloat w = btnSize;
            
            // 特殊处理最后一行: 0占两格，然后是.和=
            if (row == 4) {
                if (col == 0) {
                    // 0按钮：占两格+间距
                    w = btnSize * 2 + gap;
                } else if (col == 1) {
                    // .按钮：从0按钮后面开始
                    x = marginX + btnSize * 2 + gap * 2;
                } else if (col == 2) {
                    // =按钮
                    x = marginX + btnSize * 3 + gap * 3;
                }
            }
            
            CalcButton* btn = [[CalcButton alloc] initWithFrame:NSMakeRect(x, y, w, btnSize)];
            btn.title = [NSString stringWithUTF8String:buttons[row][col].title];
            btn.bezelStyle = NSBezelStyleRegularSquare;
            btn.bordered = NO;
            btn.buttonType = buttons[row][col].type;
            [btn setTarget:self];
            [btn setAction:buttons[row][col].action];
            [content addSubview:btn];
            
            // 正常递增x（最后一行特殊处理）
            if (row != 4) {
                x += btnSize + gap;
            }
        }
    }
    
    [g_window center];
    [g_window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

- (void)digitPressed:(CalcButton*)sender { digit_pressed([sender.title UTF8String]); }
- (void)clearPressed:(CalcButton*)sender { clear_pressed(); }
- (void)operatorPressed:(CalcButton*)sender {
    NSString* op = sender.title;
    char c = [op UTF8String][0];
    if ([op isEqualToString:@"÷"]) c = '/';
    else if ([op isEqualToString:@"×"]) c = '*';
    else if ([op isEqualToString:@"−"]) c = '-';
    operator_pressed(c);
}
- (void)equalsPressed:(CalcButton*)sender { equals_pressed(); }
- (void)percentPressed:(CalcButton*)sender { percent_pressed(); }
- (void)negatePressed:(CalcButton*)sender { negate_pressed(); }
- (void)decimalPressed:(CalcButton*)sender { decimal_pressed(); }

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)app { return YES; }
@end

int main(int argc, const char* argv[]) {
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        CalculatorDelegate* delegate = [[CalculatorDelegate alloc] init];
        [app setDelegate:delegate];
        [app run];
    }
    return 0;
}

#else
int main(int argc, char** argv) { printf("仅支持macOS\n"); return 0; }
#endif