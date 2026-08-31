#include "vulkan_allocator.h"

VkResult vulkanAllocatorCreate(const VkInstance instance, const VkPhysicalDevice physical_device, const VkDevice logical_device, VmaAllocator* allocator) {
    VmaVulkanFunctions vulkan_functions = {
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
    };

    VmaAllocatorCreateInfo create_info {
        .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
        .physicalDevice = physical_device,
        .device = logical_device,
        .pVulkanFunctions = &vulkan_functions,
        .instance = instance,
        .vulkanApiVersion = VK_API_VERSION_1_4,
    };

    return vmaCreateAllocator(&create_info, allocator);
}

void vulkanAllocatorDestroy(VmaAllocator* allocator) {
    vmaDestroyAllocator(*allocator);
}
