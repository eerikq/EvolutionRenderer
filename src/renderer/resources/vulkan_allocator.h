#pragma once

#include <volk.h>

#define VMA_IMPLEMENTATION
#define VMA_VULKAN_VERSION 1004000
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#include <vk_mem_alloc.h>

VkResult vulkanAllocatorCreate(const VkInstance instance, const VkPhysicalDevice physical_device, const VkDevice logical_device, VmaAllocator* allocator);
void vulkanAllocatorDestroy(VmaAllocator* allocator);
