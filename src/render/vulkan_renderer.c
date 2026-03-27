#include "vulkan_renderer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define VK_CHECK(call) \
    do { \
        VkResult result = call; \
        if (result != VK_SUCCESS) { \
            fprintf(stderr, "Vulkan error at %s:%d: %d\n", __FILE__, __LINE__, result); \
            return NULL; \
        } \
    } while(0)

struct BabaVulkanRenderer {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    
    VkSwapchainKHR swapchain;
    VkImage* swapchainImages;
    VkImageView* swapchainImageViews;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;
    uint32_t imageCount;
    
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    
    VkCommandPool commandPool;
    VkCommandBuffer* commandBuffers;
    
    VkBuffer* vertexBuffers;
    VkDeviceMemory* vertexBufferMemories;
    VkBuffer* indexBuffers;
    VkDeviceMemory* indexBufferMemories;
    
    VkBuffer* uniformBuffers;
    VkDeviceMemory* uniformBufferMemories;
    void** uniformBuffersMapped;
    
    VkDescriptorPool descriptorPool;
    VkDescriptorSet* descriptorSets;
    
    VkImage* depthImages;
    VkDeviceMemory* depthImageMemories;
    VkImageView* depthImageViews;
    
    VkFramebuffer* framebuffers;
    
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;
    VkFence* imagesInFlight;
    
    uint32_t currentFrame;
    uint32_t frameCount;
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool vsync;
    
    size_t vertexCount;
    size_t indexCount;
};

typedef struct {
    float pos[2];
    float color[4];
    float texCoord[2];
} Vertex;

static const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
static const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return 0;
}

static VkShaderModule createShaderModule(VkDevice device, const uint32_t* code, size_t codeSize) {
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = codeSize,
        .pCode = code,
    };
    
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}

static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    return bindingDescription;
}

static void getAttributeDescriptions(VkVertexInputAttributeDescription* attributeDescriptions) {
    attributeDescriptions[0] = (VkVertexInputAttributeDescription){
        .binding = 0, .location = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, pos)};
    attributeDescriptions[1] = (VkVertexInputAttributeDescription){
        .binding = 0, .location = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex, color)};
    attributeDescriptions[2] = (VkVertexInputAttributeDescription){
        .binding = 0, .location = 2, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, texCoord)};
}

static bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    
    VkExtensionProperties* availableExtensions = malloc(extensionCount * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);
    
    for (size_t i = 0; i < sizeof(deviceExtensions) / sizeof(deviceExtensions[0]); i++) {
        bool found = false;
        for (uint32_t j = 0; j < extensionCount; j++) {
            if (strcmp(deviceExtensions[i], availableExtensions[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            free(availableExtensions);
            return false;
        }
    }
    
    free(availableExtensions);
    return true;
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR* formats, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && 
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return formats[i];
        }
    }
    return formats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR* modes, uint32_t count, bool vsync) {
    if (!vsync) {
        for (uint32_t i = 0; i < count; i++) {
            if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                return modes[i];
            }
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR* capabilities, uint32_t width, uint32_t height) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    }
    
    VkExtent2D extent = {
        .width = width > capabilities->maxImageExtent.width ? capabilities->maxImageExtent.width : width,
        .height = height > capabilities->maxImageExtent.height ? capabilities->maxImageExtent.height : height,
    };
    
    extent.width = extent.width < capabilities->minImageExtent.width ? capabilities->minImageExtent.width : extent.width;
    extent.height = extent.height < capabilities->minImageExtent.height ? capabilities->minImageExtent.height : extent.height;
    
    return extent;
}

static VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange.aspectMask = aspectFlags,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };
    
    VkImageView imageView;
    vkCreateImageView(device, &viewInfo, NULL, &imageView);
    return imageView;
}

static void createImage(VkDevice device, VkPhysicalDevice physicalDevice, 
                       uint32_t width, uint32_t height, VkFormat format,
                       VkImageTiling tiling, VkImageUsageFlags usage,
                       VkMemoryPropertyFlags properties,
                       VkImage* image, VkDeviceMemory* imageMemory) {
    VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = width,
        .extent.height = height,
        .extent.depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = format,
        .tiling = tiling,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = VK_SAMPLE_COUNT_1_BIT,
    };
    
    vkCreateImage(device, &imageInfo, NULL, image);
    
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *image, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties),
    };
    
    vkAllocateMemory(device, &allocInfo, NULL, imageMemory);
    vkBindImageMemory(device, *image, *imageMemory, 0);
}

static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
    VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    
    for (size_t i = 0; i < 3; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &props);
        
        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return candidates[i];
        }
    }
    return VK_FORMAT_D32_SFLOAT;
}

static void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                        VkDeviceSize size, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties,
                        VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    
    vkCreateBuffer(device, &bufferInfo, NULL, buffer);
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties),
    };
    
    vkAllocateMemory(device, &allocInfo, NULL, bufferMemory);
    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

static void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue,
                      VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = commandPool,
        .commandBufferCount = 1,
    };
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkBufferCopy copyRegion = {.size = size};
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };
    
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

static uint32_t* compileShaders(size_t* vertSize, size_t* fragSize) {
    static const uint32_t vertSpv[] = {
        0x07230203, 0x00010000, 0x00080006, 0x00000000,
        0x00000011, 0x00000000, 0x00020011, 0x00000001,
        0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e,
        0x3035342e, 0x00000000, 0x0003000e, 0x00000000,
        0x00000001, 0x0007000f, 0x00000004, 0x00000004,
        0x6e69616d, 0x00000000, 0x00000009, 0x0000000b,
        0x00030010, 0x00000004, 0x00000007, 0x00040047,
        0x0000000b, 0x0000001e, 0x00000000, 0x00040047,
        0x0000000f, 0x00000022, 0x00000000, 0x00000001,
        0x00040047, 0x0000000f, 0x00000021, 0x00000000,
        0x00040047, 0x00000011, 0x0000000b, 0x00000019,
        0x00020013, 0x00000002, 0x00030021, 0x00000003,
        0x00000002, 0x00030016, 0x00000006, 0x00000020,
        0x00040017, 0x00000007, 0x00000006, 0x00000002,
        0x00040017, 0x00000008, 0x00000006, 0x00000004,
        0x00040020, 0x00000009, 0x00000003, 0x00000007,
        0x0004003b, 0x00000009, 0x0000000a, 0x00000003,
        0x00040015, 0x0000000b, 0x00000020, 0x00000001,
        0x0004002b, 0x0000000b, 0x0000000c, 0x00000000,
        0x00040020, 0x0000000d, 0x00000001, 0x00000007,
        0x0004003b, 0x0000000d, 0x0000000e, 0x00000001,
        0x00040020, 0x0000000f, 0x00000003, 0x00000008,
        0x00040017, 0x00000010, 0x00000006, 0x00000002,
        0x00040020, 0x00000011, 0x00000001, 0x00000010,
        0x0004003b, 0x00000011, 0x00000012, 0x00000001,
        0x00050036, 0x00000002, 0x00000004, 0x00000000,
        0x00000003, 0x000200f8, 0x00000005, 0x0004003d,
        0x00000007, 0x00000013, 0x0000000a, 0x00050041,
        0x00000010, 0x00000014, 0x00000012, 0x0000000c,
        0x0003003e, 0x00000014, 0x00000013, 0x0004003d,
        0x00000008, 0x00000015, 0x0000000a, 0x00050041,
        0x00000007, 0x00000016, 0x0000000e, 0x0000000c,
        0x0003003e, 0x00000016, 0x00000015, 0x000100fd,
        0x00010038
    };
    
    static const uint32_t fragSpv[] = {
        0x07230203, 0x00010000, 0x00080006, 0x00000000,
        0x00000011, 0x00000000, 0x00020011, 0x00000001,
        0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e,
        0x3035342e, 0x00000000, 0x0003000e, 0x00000000,
        0x00000001, 0x0007000f, 0x00000004, 0x00000004,
        0x6e69616d, 0x00000000, 0x0000000e, 0x00000012,
        0x00030010, 0x00000004, 0x00000007, 0x00040047,
        0x00000012, 0x0000001e, 0x00000000, 0x00040047,
        0x00000016, 0x00000022, 0x00000000, 0x00000001,
        0x00040047, 0x00000016, 0x00000021, 0x00000000,
        0x00040047, 0x0000001e, 0x0000000b, 0x00000019,
        0x00020013, 0x00000002, 0x00030021, 0x00000003,
        0x00000002, 0x00030016, 0x00000006, 0x00000020,
        0x00040017, 0x00000007, 0x00000006, 0x00000004,
        0x00040020, 0x00000008, 0x00000003, 0x00000007,
        0x0004003b, 0x00000008, 0x00000009, 0x00000003,
        0x00040017, 0x0000000a, 0x00000006, 0x00000002,
        0x00040020, 0x0000000b, 0x00000001, 0x0000000a,
        0x0004003b, 0x0000000b, 0x0000000c, 0x00000001,
        0x00040015, 0x0000000e, 0x00000020, 0x00000001,
        0x0004002b, 0x0000000e, 0x0000000f, 0x00000000,
        0x00040020, 0x00000010, 0x00000003, 0x00000007,
        0x00040020, 0x00000012, 0x00000003, 0x00000007,
        0x0004003b, 0x00000012, 0x00000013, 0x00000003,
        0x00040017, 0x00000014, 0x00000006, 0x00000002,
        0x00040020, 0x00000015, 0x00000001, 0x00000014,
        0x0004003b, 0x00000015, 0x00000016, 0x00000001,
        0x00050036, 0x00000002, 0x00000004, 0x00000000,
        0x00000003, 0x000200f8, 0x00000005, 0x00050041,
        0x0000000a, 0x00000017, 0x0000000c, 0x0000000f,
        0x00050041, 0x00000007, 0x00000018, 0x00000013,
        0x0000000f, 0x0003003e, 0x00000018, 0x00000017,
        0x000100fd, 0x00010038
    };
    
    *vertSize = sizeof(vertSpv);
    *fragSize = sizeof(fragSpv);
    
    return NULL;
}

BabaVulkanRenderer* baba_vulkan_renderer_create(
    BabaPlatformWindow* window,
    const BabaVulkanConfig* config
) {
    BabaVulkanRenderer* renderer = calloc(1, sizeof(BabaVulkanRenderer));
    if (!renderer) return NULL;
    
    renderer->frameCount = config && config->max_frames_in_flight ? config->max_frames_in_flight : 2;
    renderer->vsync = config ? config->vsync : true;
    renderer->currentFrame = 0;
    
    uint32_t extensionCount = 0;
    const char** extensions = baba_platform_get_required_vulkan_extensions(&extensionCount);
    
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Baba App",
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = "Baba Engine",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };
    
    VkInstanceCreateInfo instanceInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions,
    };
    
    VK_CHECK(vkCreateInstance(&instanceInfo, NULL, &renderer->instance));
    
    renderer->surface = baba_platform_window_create_surface(window, renderer->instance);
    if (!renderer->surface) {
        vkDestroyInstance(renderer->instance, NULL);
        free(renderer);
        return NULL;
    }
    
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(renderer->instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
        vkDestroyInstance(renderer->instance, NULL);
        free(renderer);
        return NULL;
    }
    
    VkPhysicalDevice* devices = malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(renderer->instance, &deviceCount, devices);
    
    renderer->physicalDevice = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < deviceCount; i++) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, NULL);
        VkQueueFamilyProperties* queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, queueFamilies);
        
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily = UINT32_MAX;
        
        for (uint32_t j = 0; j < queueFamilyCount; j++) {
            if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsFamily = j;
            }
            
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, renderer->surface, &presentSupport);
            if (presentSupport) {
                presentFamily = j;
            }
            
            if (graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX) {
                renderer->physicalDevice = devices[i];
                renderer->graphicsFamily = graphicsFamily;
                renderer->presentFamily = presentFamily;
                break;
            }
        }
        
        free(queueFamilies);
        
        if (renderer->physicalDevice && checkDeviceExtensionSupport(renderer->physicalDevice)) {
            break;
        }
    }
    
    free(devices);
    
    if (!renderer->physicalDevice) {
        vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
        vkDestroyInstance(renderer->instance, NULL);
        free(renderer);
        return NULL;
    }
    
    float queuePriority = 1.0f;
    uint32_t queueCreateInfosCount = (renderer->graphicsFamily == renderer->presentFamily) ? 1 : 2;
    VkDeviceQueueCreateInfo* queueCreateInfos = malloc(queueCreateInfosCount * sizeof(VkDeviceQueueCreateInfo));
    
    queueCreateInfos[0] = (VkDeviceQueueCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = renderer->graphicsFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };
    
    if (queueCreateInfosCount == 2) {
        queueCreateInfos[1] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = renderer->presentFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };
    }
    
    VkPhysicalDeviceFeatures deviceFeatures = {0};
    
    VkDeviceCreateInfo deviceInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = queueCreateInfosCount,
        .pQueueCreateInfos = queueCreateInfos,
        .pEnabledFeatures = &deviceFeatures,
        .enabledExtensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]),
        .ppEnabledExtensionNames = deviceExtensions,
    };
    
    VK_CHECK(vkCreateDevice(renderer->physicalDevice, &deviceInfo, NULL, &renderer->device));
    free(queueCreateInfos);
    
    vkGetDeviceQueue(renderer->device, renderer->graphicsFamily, 0, &renderer->graphicsQueue);
    vkGetDeviceQueue(renderer->device, renderer->presentFamily, 0, &renderer->presentQueue);
    
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer->physicalDevice, renderer->surface, &capabilities);
    
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->physicalDevice, renderer->surface, &formatCount, NULL);
    VkSurfaceFormatKHR* formats = malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->physicalDevice, renderer->surface, &formatCount, formats);
    
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats, formatCount);
    free(formats);
    
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->physicalDevice, renderer->surface, &presentModeCount, NULL);
    VkPresentModeKHR* presentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->physicalDevice, renderer->surface, &presentModeCount, presentModes);
    
    VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes, presentModeCount, renderer->vsync);
    free(presentModes);
    
    int width, height;
    baba_platform_window_get_size(window, &width, &height);
    VkExtent2D extent = chooseSwapExtent(&capabilities, width, height);
    
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR swapchainInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = renderer->surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    
    if (renderer->graphicsFamily != renderer->presentFamily) {
        uint32_t queueFamilyIndices[] = {renderer->graphicsFamily, renderer->presentFamily};
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2;
        swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    
    VK_CHECK(vkCreateSwapchainKHR(renderer->device, &swapchainInfo, NULL, &renderer->swapchain));
    
    renderer->swapchainFormat = surfaceFormat.format;
    renderer->swapchainExtent = extent;
    
    vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &renderer->imageCount, NULL);
    renderer->swapchainImages = malloc(renderer->imageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &renderer->imageCount, renderer->swapchainImages);
    
    renderer->swapchainImageViews = malloc(renderer->imageCount * sizeof(VkImageView));
    for (uint32_t i = 0; i < renderer->imageCount; i++) {
        renderer->swapchainImageViews[i] = createImageView(renderer->device, renderer->swapchainImages[i], 
                                                          renderer->swapchainFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    
    VkAttachmentDescription colorAttachment = {
        .format = renderer->swapchainFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    
    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
    };
    
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };
    
    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };
    
    VK_CHECK(vkCreateRenderPass(renderer->device, &renderPassInfo, NULL, &renderer->renderPass));
    
    size_t vertSize, fragSize;
    compileShaders(&vertSize, &fragSize);
    
    static const uint32_t vertSpv[] = {
        0x07230203, 0x00010000, 0x00080006, 0x00000000, 0x00000011, 0x00000000,
        0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e,
        0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x00030010,
        0x00000004, 0x00000007, 0x00040047, 0x0000000b, 0x0000001e, 0x00000000,
        0x00040047, 0x0000000f, 0x00000022, 0x00000000, 0x00000001, 0x00040047,
        0x0000000f, 0x00000021, 0x00000000, 0x0000000b, 0x00060017, 0x00000001,
        0x00000006, 0x00000004, 0x00000000, 0x00000000, 0x00060017, 0x00000002,
        0x00000006, 0x00000002, 0x00000000, 0x00000000, 0x00060017, 0x00000003,
        0x00000006, 0x00000002, 0x00000000, 0x00000001, 0x00040020, 0x00000004,
        0x00000001, 0x00000003, 0x00080020, 0x00000005, 0x00000003, 0x00000006,
        0x00000001, 0x00000001, 0x00000000, 0x00000002, 0x00000000, 0x00000005,
        0x00030005, 0x00000006, 0x006d6178, 0x00040020, 0x00000007, 0x00000003,
        0x00000006, 0x0004003b, 0x00000007, 0x00000008, 0x00000003, 0x00040015,
        0x00000009, 0x00000020, 0x00000001, 0x0004002b, 0x00000009, 0x0000000a,
        0x00000000, 0x0004002b, 0x00000009, 0x0000000b, 0x00000001, 0x00040017,
        0x0000000c, 0x00000006, 0x00000004, 0x00000020, 0x00000000, 0x00090019,
        0x0000000d, 0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000001, 0x00000000, 0x0003001d, 0x0000000e, 0x0000000d,
        0x0006001e, 0x0000000f, 0x0000000d, 0x0000000e, 0x0000000e, 0x0000000e,
        0x00040020, 0x00000010, 0x00000000, 0x0000000f, 0x0004003b, 0x00000010,
        0x00000011, 0x00000000, 0x00090019, 0x00000012, 0x00000006, 0x00000001,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x0003001d, 0x00000013, 0x00000012, 0x0006001e, 0x00000014, 0x00000012,
        0x00000013, 0x00000013, 0x00000013, 0x00040020, 0x00000015, 0x00000000,
        0x00000014, 0x0004003b, 0x00000015, 0x00000016, 0x00000000, 0x00040015,
        0x00000017, 0x00000020, 0x00000000, 0x0004002b, 0x00000017, 0x00000018,
        0x00000000, 0x0004002b, 0x00000017, 0x00000019, 0x00000001, 0x0004002b,
        0x00000017, 0x0000001a, 0x00000002, 0x0004002b, 0x00000017, 0x0000001b,
        0x00000003, 0x0005002c, 0x0000000c, 0x0000001c, 0x00000018, 0x0000001b,
        0x00050041, 0x00000001, 0x0000001d, 0x00000008, 0x0000000a, 0x0004003d,
        0x00000002, 0x0000001e, 0x0000001d, 0x00050041, 0x0000000c, 0x0000001f,
        0x00000011, 0x0000001c, 0x00050051, 0x00000006, 0x00000020, 0x0000001f,
        0x0000001e, 0x00050041, 0x00000001, 0x00000021, 0x00000008, 0x0000000b,
        0x0004003d, 0x00000003, 0x00000022, 0x00000021, 0x00050041, 0x0000000c,
        0x00000023, 0x00000011, 0x0000001c, 0x00050051, 0x00000006, 0x00000024,
        0x00000023, 0x00000022, 0x0004003d, 0x0000000c, 0x00000025, 0x00000011,
        0x00070050, 0x00000006, 0x00000026, 0x00000025, 0x00000020, 0x00000024,
        0x00000019, 0x00050041, 0x00000006, 0x00000027, 0x00000016, 0x0000001a,
        0x0003003e, 0x00000027, 0x00000026, 0x000100fd, 0x00010038
    };
    
    static const uint32_t fragSpv[] = {
        0x07230203, 0x00010000, 0x00080006, 0x00000000, 0x00000011, 0x00000000,
        0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e,
        0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x00030010,
        0x00000004, 0x00000007, 0x00040047, 0x0000000b, 0x0000001e, 0x00000000,
        0x00040047, 0x0000000f, 0x00000022, 0x00000000, 0x00000001, 0x00040047,
        0x0000000f, 0x00000021, 0x00000000, 0x0000000b, 0x00060017, 0x00000001,
        0x00000006, 0x00000004, 0x00000000, 0x00000000, 0x00060017, 0x00000002,
        0x00000006, 0x00000002, 0x00000000, 0x00000000, 0x00060017, 0x00000003,
        0x00000006, 0x00000002, 0x00000000, 0x00000001, 0x00040020, 0x00000004,
        0x00000001, 0x00000003, 0x00080020, 0x00000005, 0x00000003, 0x00000006,
        0x00000001, 0x00000001, 0x00000000, 0x00000002, 0x00000000, 0x00000005,
        0x0004003b, 0x00000004, 0x00000006, 0x00000001, 0x00040015, 0x00000007,
        0x00000020, 0x00000001, 0x0004002b, 0x00000007, 0x00000008, 0x00000000,
        0x0004002b, 0x00000007, 0x00000009, 0x00000001, 0x00040020, 0x0000000a,
        0x00000003, 0x00000001, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000003,
        0x00040017, 0x0000000c, 0x00000006, 0x00000004, 0x00000020, 0x00000000,
        0x00040020, 0x0000000d, 0x00000003, 0x0000000c, 0x0004003b, 0x0000000d,
        0x0000000e, 0x00000003, 0x00040020, 0x0000000f, 0x00000003, 0x00000002,
        0x0004003b, 0x0000000f, 0x00000010, 0x00000003, 0x0004002b, 0x00000007,
        0x00000011, 0x00000002, 0x0005002c, 0x00000001, 0x00000012, 0x00000008,
        0x00000011, 0x00050041, 0x00000001, 0x00000013, 0x0000000b, 0x00000008,
        0x0004003d, 0x00000001, 0x00000014, 0x00000013, 0x00050041, 0x0000000c,
        0x00000015, 0x0000000e, 0x00000012, 0x0003003e, 0x00000015, 0x00000014,
        0x00050041, 0x00000002, 0x00000016, 0x00000010, 0x00000008, 0x0004003d,
        0x00000002, 0x00000017, 0x00000016, 0x00050041, 0x00000002, 0x00000018,
        0x00000010, 0x00000009, 0x0004003d, 0x00000002, 0x00000019, 0x00000018,
        0x00050051, 0x00000006, 0x0000001a, 0x00000017, 0x00000019, 0x00050051,
        0x00000006, 0x0000001b, 0x0000001a, 0x00000014, 0x00050051, 0x00000006,
        0x0000001c, 0x0000001b, 0x00000014, 0x00070050, 0x00000006, 0x0000001d,
        0x0000001c, 0x00000015, 0x00000015, 0x00000015, 0x0003003e, 0x00000006,
        0x0000001d, 0x000100fd, 0x00010038
    };
    
    VkShaderModule vertShaderModule = createShaderModule(renderer->device, vertSpv, sizeof(vertSpv));
    VkShaderModule fragShaderModule = createShaderModule(renderer->device, fragSpv, sizeof(fragSpv));
    
    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertShaderModule,
            .pName = "main",
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragShaderModule,
            .pName = "main",
        }
    };
    
    VkVertexInputBindingDescription bindingDescription = getBindingDescription();
    VkVertexInputAttributeDescription attributeDescriptions[3];
    getAttributeDescriptions(attributeDescriptions);
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = 3,
        .pVertexAttributeDescriptions = attributeDescriptions,
    };
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
    
    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)renderer->swapchainExtent.width,
        .height = (float)renderer->swapchainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    
    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = renderer->swapchainExtent,
    };
    
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };
    
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };
    
    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };
    
    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };
    
    VK_CHECK(vkCreatePipelineLayout(renderer->device, &pipelineLayoutInfo, NULL, &renderer->pipelineLayout));
    
    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .layout = renderer->pipelineLayout,
        .renderPass = renderer->renderPass,
        .subpass = 0,
    };
    
    VK_CHECK(vkCreateGraphicsPipelines(renderer->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &renderer->graphicsPipeline));
    
    vkDestroyShaderModule(renderer->device, fragShaderModule, NULL);
    vkDestroyShaderModule(renderer->device, vertShaderModule, NULL);
    
    renderer->framebuffers = malloc(renderer->imageCount * sizeof(VkFramebuffer));
    for (uint32_t i = 0; i < renderer->imageCount; i++) {
        VkImageView attachments[] = {renderer->swapchainImageViews[i]};
        
        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderer->renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = renderer->swapchainExtent.width,
            .height = renderer->swapchainExtent.height,
            .layers = 1,
        };
        
        VK_CHECK(vkCreateFramebuffer(renderer->device, &framebufferInfo, NULL, &renderer->framebuffers[i]));
    }
    
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = renderer->graphicsFamily,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    };
    
    VK_CHECK(vkCreateCommandPool(renderer->device, &poolInfo, NULL, &renderer->commandPool));
    
    renderer->commandBuffers = malloc(renderer->frameCount * sizeof(VkCommandBuffer));
    
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = renderer->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = renderer->frameCount,
    };
    
    VK_CHECK(vkAllocateCommandBuffers(renderer->device, &allocInfo, renderer->commandBuffers));
    
    renderer->imageAvailableSemaphores = malloc(renderer->frameCount * sizeof(VkSemaphore));
    renderer->renderFinishedSemaphores = malloc(renderer->frameCount * sizeof(VkSemaphore));
    renderer->inFlightFences = malloc(renderer->frameCount * sizeof(VkFence));
    renderer->imagesInFlight = malloc(renderer->imageCount * sizeof(VkFence));
    memset(renderer->imagesInFlight, 0, renderer->imageCount * sizeof(VkFence));
    
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    
    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    
    for (uint32_t i = 0; i < renderer->frameCount; i++) {
        vkCreateSemaphore(renderer->device, &semaphoreInfo, NULL, &renderer->imageAvailableSemaphores[i]);
        vkCreateSemaphore(renderer->device, &semaphoreInfo, NULL, &renderer->renderFinishedSemaphores[i]);
        vkCreateFence(renderer->device, &fenceInfo, NULL, &renderer->inFlightFences[i]);
    }
    
    return renderer;
}void baba_vulkan_renderer_destroy(BabaVulkanRenderer* renderer) {
    if (!renderer) return;
    
    vkDeviceWaitIdle(renderer->device);
    
    for (uint32_t i = 0; i < renderer->frameCount; i++) {
        vkDestroySemaphore(renderer->device, renderer->renderFinishedSemaphores[i], NULL);
        vkDestroySemaphore(renderer->device, renderer->imageAvailableSemaphores[i], NULL);
        vkDestroyFence(renderer->device, renderer->inFlightFences[i], NULL);
    }
    free(renderer->renderFinishedSemaphores);
    free(renderer->imageAvailableSemaphores);
    free(renderer->inFlightFences);
    free(renderer->imagesInFlight);
    
    vkFreeCommandBuffers(renderer->device, renderer->commandPool, renderer->frameCount, renderer->commandBuffers);
    free(renderer->commandBuffers);
    
    vkDestroyCommandPool(renderer->device, renderer->commandPool, NULL);
    
    for (uint32_t i = 0; i < renderer->imageCount; i++) {
        vkDestroyFramebuffer(renderer->device, renderer->framebuffers[i], NULL);
    }
    free(renderer->framebuffers);
    
    vkDestroyPipeline(renderer->device, renderer->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(renderer->device, renderer->pipelineLayout, NULL);
    vkDestroyRenderPass(renderer->device, renderer->renderPass, NULL);
    
    for (uint32_t i = 0; i < renderer->imageCount; i++) {
        vkDestroyImageView(renderer->device, renderer->swapchainImageViews[i], NULL);
    }
    free(renderer->swapchainImageViews);
    free(renderer->swapchainImages);
    
    vkDestroySwapchainKHR(renderer->device, renderer->swapchain, NULL);
    vkDestroyDevice(renderer->device, NULL);
    vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
    vkDestroyInstance(renderer->instance, NULL);
    
    free(renderer);
}

int baba_vulkan_renderer_begin_frame(BabaVulkanRenderer* renderer) {
    if (!renderer) return BABA_ERROR_INVALID_PARAM;
    
    vkWaitForFences(renderer->device, 1, &renderer->inFlightFences[renderer->currentFrame], 
                    VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(renderer->device, renderer->swapchain, 
                                            UINT64_MAX, renderer->imageAvailableSemaphores[renderer->currentFrame],
                                            VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return BABA_ERROR_VULKAN;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        return BABA_ERROR_VULKAN;
    }
    
    if (renderer->imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(renderer->device, 1, &renderer->imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    renderer->imagesInFlight[imageIndex] = renderer->inFlightFences[renderer->currentFrame];
    
    vkResetCommandBuffer(renderer->commandBuffers[renderer->currentFrame], 0);
    
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = NULL,
    };
    
    if (vkBeginCommandBuffer(renderer->commandBuffers[renderer->currentFrame], &beginInfo) != VK_SUCCESS) {
        return BABA_ERROR_VULKAN;
    }
    
    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderer->renderPass,
        .framebuffer = renderer->framebuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = renderer->swapchainExtent,
        .clearValueCount = 1,
    };
    
    VkClearValue clearValue = {{{0.1f, 0.1f, 0.15f, 1.0f}}};
    renderPassInfo.pClearValues = &clearValue;
    
    vkCmdBeginRenderPass(renderer->commandBuffers[renderer->currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    vkCmdBindPipeline(renderer->commandBuffers[renderer->currentFrame], 
                      VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->graphicsPipeline);
    
    renderer->currentFrame = (renderer->currentFrame + 1) % renderer->frameCount;
    
    return BABA_SUCCESS;
}

int baba_vulkan_renderer_end_frame(BabaVulkanRenderer* renderer) {
    if (!renderer) return BABA_ERROR_INVALID_PARAM;
    
    vkCmdEndRenderPass(renderer->commandBuffers[renderer->currentFrame]);
    
    if (vkEndCommandBuffer(renderer->commandBuffers[renderer->currentFrame]) != VK_SUCCESS) {
        return BABA_ERROR_VULKAN;
    }
    
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer->imageAvailableSemaphores[renderer->currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderer->renderFinishedSemaphores[renderer->currentFrame],
        .commandBufferCount = 1,
        .pCommandBuffers = &renderer->commandBuffers[renderer->currentFrame],
    };
    
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = waitStages;
    
    vkResetFences(renderer->device, 1, &renderer->inFlightFences[renderer->currentFrame]);
    
    if (vkQueueSubmit(renderer->graphicsQueue, 1, &submitInfo, renderer->inFlightFences[renderer->currentFrame]) != VK_SUCCESS) {
        return BABA_ERROR_VULKAN;
    }
    
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer->renderFinishedSemaphores[renderer->currentFrame],
        .swapchainCount = 1,
        .pSwapchains = &renderer->swapchain,
    };
    
    vkQueuePresentKHR(renderer->presentQueue, &presentInfo);
    
    return BABA_SUCCESS;
}

int baba_vulkan_renderer_resize(BabaVulkanRenderer* renderer, int width, int height) {
    if (!renderer) return BABA_ERROR_INVALID_PARAM;
    
    vkDeviceWaitIdle(renderer->device);
    
    for (uint32_t i = 0; i < renderer->imageCount; i++) {
        vkDestroyFramebuffer(renderer->device, renderer->framebuffers[i], NULL);
        vkDestroyImageView(renderer->device, renderer->swapchainImageViews[i], NULL);
    }
    free(renderer->framebuffers);
    free(renderer->swapchainImageViews);
    free(renderer->swapchainImages);
    
    vkDestroySwapchainKHR(renderer->device, renderer->swapchain, NULL);
    
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer->physicalDevice, renderer->surface, &capabilities);
    
    VkExtent2D extent = chooseSwapExtent(&capabilities, width, height);
    
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR swapchainInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = renderer->surface,
        .minImageCount = imageCount,
        .imageFormat = renderer->swapchainFormat,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    
    vkCreateSwapchainKHR(renderer->device, &swapchainInfo, NULL, &renderer->swapchain);
    
    renderer->swapchainExtent = extent;
    
    vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &renderer->imageCount, NULL);
    renderer->swapchainImages = malloc(renderer->imageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &renderer->imageCount, renderer->swapchainImages);
    
    renderer->swapchainImageViews = malloc(renderer->imageCount * sizeof(VkImageView));
    for (uint32_t i = 0; i < renderer->imageCount; i++) {
        renderer->swapchainImageViews[i] = createImageView(renderer->device, renderer->swapchainImages[i],
                                                          renderer->swapchainFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    
    renderer->framebuffers = malloc(renderer->imageCount * sizeof(VkFramebuffer));
    for (uint32_t i = 0; i < renderer->imageCount; i++) {
        VkImageView attachments[] = {renderer->swapchainImageViews[i]};
        
        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderer->renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = renderer->swapchainExtent.width,
            .height = renderer->swapchainExtent.height,
            .layers = 1,
        };
        
        vkCreateFramebuffer(renderer->device, &framebufferInfo, NULL, &renderer->framebuffers[i]);
    }
    
    return BABA_SUCCESS;
}

VkDevice baba_vulkan_renderer_get_device(BabaVulkanRenderer* renderer) {
    return renderer ? renderer->device : VK_NULL_HANDLE;
}

VkPhysicalDevice baba_vulkan_renderer_get_physical_device(BabaVulkanRenderer* renderer) {
    return renderer ? renderer->physicalDevice : VK_NULL_HANDLE;
}

VkRenderPass baba_vulkan_renderer_get_render_pass(BabaVulkanRenderer* renderer) {
    return renderer ? renderer->renderPass : VK_NULL_HANDLE;
}

VkCommandBuffer baba_vulkan_renderer_get_command_buffer(BabaVulkanRenderer* renderer) {
    return renderer ? renderer->commandBuffers[renderer->currentFrame] : VK_NULL_HANDLE;
}

uint32_t baba_vulkan_renderer_get_current_frame(BabaVulkanRenderer* renderer) {
    return renderer ? renderer->currentFrame : 0;
}

VkFormat baba_vulkan_renderer_get_swapchain_format(BabaVulkanRenderer* renderer) {
    return renderer ? renderer->swapchainFormat : VK_FORMAT_UNDEFINED;
}

VkExtent2D baba_vulkan_renderer_get_swapchain_extent(BabaVulkanRenderer* renderer) {
    VkExtent2D empty = {0, 0};
    return renderer ? renderer->swapchainExtent : empty;
}