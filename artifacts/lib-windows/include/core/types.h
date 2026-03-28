#ifndef BABA_CORE_TYPES_H
#define BABA_CORE_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define BABA_INLINE static inline

#define BABA_CONTAINER_OF(ptr, type, member) \
    ((type*)((char*)(ptr) - offsetof(type, member)))

#define BABA_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define BABA_MIN(a, b) ((a) < (b) ? (a) : (b))
#define BABA_MAX(a, b) ((a) > (b) ? (a) : (b))
#define BABA_CLAMP(x, lo, hi) BABA_MIN(BABA_MAX(x, lo), hi)

typedef enum {
    BABA_SUCCESS = 0,
    BABA_ERROR_UNKNOWN = -1,
    BABA_ERROR_INVALID_PARAM = -2,
    BABA_ERROR_OUT_OF_MEMORY = -3,
    BABA_ERROR_VULKAN = -4,
    BABA_ERROR_PLATFORM = -5,
    BABA_ERROR_FONT = -6,
} BabaResult;

typedef enum {
    BABA_EVENT_NONE = 0,
    BABA_EVENT_MOUSE_MOVE,
    BABA_EVENT_MOUSE_BUTTON,
    BABA_EVENT_MOUSE_SCROLL,
    BABA_EVENT_KEY,
    BABA_EVENT_CHAR,
    BABA_EVENT_RESIZE,
    BABA_EVENT_CLOSE,
    BABA_EVENT_FOCUS,
    BABA_EVENT_REDRAW,
} BabaEventType;

typedef enum {
    BABA_MOUSE_BUTTON_LEFT = 0,
    BABA_MOUSE_BUTTON_RIGHT = 1,
    BABA_MOUSE_BUTTON_MIDDLE = 2,
} BabaMouseButton;

typedef enum {
    BABA_KEY_STATE_RELEASED = 0,
    BABA_KEY_STATE_PRESSED = 1,
    BABA_KEY_STATE_REPEATED = 2,
} BabaKeyState;

typedef struct {
    BabaEventType type;
    union {
        struct {
            float x, y;
        } mouse_move;
        
        struct {
            BabaMouseButton button;
            bool pressed;
            float x, y;
        } mouse_button;
        
        struct {
            float dx, dy;
            float x, y;
        } mouse_scroll;
        
        struct {
            int keycode;
            int scancode;
            BabaKeyState state;
            bool shift, ctrl, alt, super;
        } key;
        
        struct {
            uint32_t codepoint;
        } character;
        
        struct {
            int width, height;
        } resize;
        
        struct {
            bool gained;
        } focus;
    };
} BabaEvent;

#endif