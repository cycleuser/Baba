#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>
#include "../../platform/window.h"

VkSurfaceKHR baba_platform_window_create_surface(
    BabaPlatformWindow* window,
    VkInstance instance
);

const char** baba_platform_get_required_vulkan_extensions(uint32_t* count);