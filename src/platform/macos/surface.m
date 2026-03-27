#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_metal.h>
#include "../../platform/window.h"

struct BabaPlatformWindow {
    NSWindow* window;
    NSView* view;
};

VkSurfaceKHR baba_platform_window_create_surface(
    BabaPlatformWindow* window,
    VkInstance instance
) {
    if (!window || !instance) return VK_NULL_HANDLE;
    
    VkMetalSurfaceCreateInfoEXT create_info = {
        .sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT,
        .pNext = NULL,
        .flags = 0,
        .pLayer = (CAMetalLayer*)window->view.layer
    };
    
    VkSurfaceKHR surface;
    PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT = 
        (PFN_vkCreateMetalSurfaceEXT)vkGetInstanceProcAddr(instance, "vkCreateMetalSurfaceEXT");
    
    if (!vkCreateMetalSurfaceEXT) {
        return VK_NULL_HANDLE;
    }
    
    VkResult result = vkCreateMetalSurfaceEXT(instance, &create_info, NULL, &surface);
    
    if (result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    
    return surface;
}