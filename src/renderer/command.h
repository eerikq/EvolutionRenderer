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

void commandRecordBuffer(
    const uint32_t frame_index,
    const uint32_t image_index,
    const VkCommandBuffer* command_buffers,
    const VkImage* swapchain_images,
    const VkImageView* swapchain_image_views,
    const VkPipeline graphics_pipeline,
    const VkExtent2D swapchain_extent,
    const VkBuffer* vertex_buffer,
    const VkBuffer* index_buffer
);
