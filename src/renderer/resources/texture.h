#pragma once

#include "util/image.h"

#include <volk.h>
//
#include <vk_mem_alloc.h>

typedef struct Texture {
    VkImage data = nullptr;
    VkImageView view = nullptr;
    VmaAllocation allocation = nullptr;
} Texture;

namespace texture {
    void TransitionImageLayout(
        const VkCommandBuffer command_buffer,
        const VkImage image,
        VkPipelineStageFlags2 src_stage_mask,
        VkPipelineStageFlags2 dst_stage_mask,
        VkAccessFlags2 src_access_mask,
        VkAccessFlags2 dst_access_mask,
        VkImageLayout old_layout,
        VkImageLayout new_layout
    );

    VkResult Create(
        const VkDevice logical_device,
        const VmaAllocator allocator,
        const VmaAllocationCreateFlags allocation_flags,
        const Image* image,
        Texture* texture
    );
    VkResult CreateView(const VkDevice logical_device, const VkFormat texture_format, Texture* texture);
    void Destroy(const VmaAllocator allocator, const VkDevice logical_device, Texture* texture);

    VkResult LoadImage(
        const VkDevice logical_device,
        const VkQueue graphics_queue,
        const VkCommandPool command_pool,
        const VmaAllocator allocator,
        const Image* image,
        Texture* texture
    );
} // namespace texture
