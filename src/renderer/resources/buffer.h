#pragma once

#include <volk.h>
//
#include <vk_mem_alloc.h>

typedef struct Buffer {
    VkBuffer data = nullptr;
    VkBufferView view = nullptr;
    VmaAllocation allocation = nullptr;
} Buffer;

namespace buffer {
    VkResult Create(
        const VkDeviceSize size,
        const VkBufferUsageFlags buffer_flags,
        const VmaAllocationCreateFlags allocation_flags,
        const VmaAllocator allocator,
        Buffer* buffer
    );
    void Destroy(const VmaAllocator allocator, Buffer* buffer);

    VkResult CopyFromBuffer(
        const VkQueue graphics_queue,
        const VkCommandPool command_pool,
        const VkDevice logical_device,
        const Buffer source,
        const Buffer destination,
        const VkDeviceSize size
    );

    VkResult Write(
        const VmaAllocator allocator,
        const VkQueue graphics_queue,
        const VkCommandPool command_pool,
        const VkDevice logical_device,
        const VkDeviceSize data_size,
        const void* data,
        Buffer* buffer
    );
} // namespace buffer
