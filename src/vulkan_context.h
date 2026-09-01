#pragma once

#include "vulkan/vulkan_core.h"

#include <volk.h>

//
#include <vector>
#include <vk_mem_alloc.h>

typedef struct VulkanContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;

    VmaAllocator allocator;

    // everything below this line should probably be
    // moved elsewhere but im not sure where yet
    VkDescriptorSetLayout descriptor_layout = nullptr;

    VkPipelineLayout pipeline_layout = nullptr;
    VkPipeline graphics_pipeline = nullptr;

    VkCommandPool command_pool = nullptr;
    std::vector<VkCommandBuffer> command_buffers;
} VulkanContext;
