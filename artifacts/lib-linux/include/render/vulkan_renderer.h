#ifndef BABA_RENDER_VULKAN_RENDERER_H
#define BABA_RENDER_VULKAN_RENDERER_H

#include "../core/types.h"
#include <vulkan/vulkan.h>
#include "../platform/window.h"

typedef struct BabaVulkanRenderer BabaVulkanRenderer;

typedef struct {
    VkFormat color_format;
    VkColorSpaceKHR color_space;
    VkPresentModeKHR present_mode;
    bool vsync;
    uint32_t max_frames_in_flight;
} BabaVulkanConfig;

BabaVulkanRenderer* baba_vulkan_renderer_create(
    BabaPlatformWindow* window,
    const BabaVulkanConfig* config
);

void baba_vulkan_renderer_destroy(BabaVulkanRenderer* renderer);

int baba_vulkan_renderer_begin_frame(BabaVulkanRenderer* renderer);
int baba_vulkan_renderer_end_frame(BabaVulkanRenderer* renderer);
int baba_vulkan_renderer_resize(BabaVulkanRenderer* renderer, int width, int height);

VkDevice baba_vulkan_renderer_get_device(BabaVulkanRenderer* renderer);
VkPhysicalDevice baba_vulkan_renderer_get_physical_device(BabaVulkanRenderer* renderer);
VkRenderPass baba_vulkan_renderer_get_render_pass(BabaVulkanRenderer* renderer);
VkCommandBuffer baba_vulkan_renderer_get_command_buffer(BabaVulkanRenderer* renderer);
uint32_t baba_vulkan_renderer_get_current_frame(BabaVulkanRenderer* renderer);

VkFormat baba_vulkan_renderer_get_swapchain_format(BabaVulkanRenderer* renderer);
VkExtent2D baba_vulkan_renderer_get_swapchain_extent(BabaVulkanRenderer* renderer);

#endif