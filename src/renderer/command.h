#pragma once

#include <volk.h>

VkResult commandCreatePool(const VkDevice logical_device, const uint32_t queue_index, VkCommandPool* command_pool);
void commandDestroyPool(const VkDevice logical_device, VkCommandPool* command_pool);

VkResult commandAllocateBuffers(const VkDevice logical_device, const VkCommandPool command_pool, VkCommandBuffer* command_buffers);
void commandFreeBuffers(
    const VkDevice logical_device,
    const VkCommandPool command_pool,
    VkCommandBuffer* command_buffers,
    const uint32_t command_buffers_count
);
