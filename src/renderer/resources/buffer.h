#pragma once

#include <volk.h>
//
#include <vk_mem_alloc.h>

typedef struct Buffer {
    VkBuffer data = nullptr;
    VkBufferView view = nullptr;
    VmaAllocation allocation = nullptr;
} Buffer;

VkResult bufferCreate(
    const VkDeviceSize size,
    const VkBufferUsageFlags buffer_flags,
    const VmaAllocationCreateFlags allocation_flags,
    const VmaAllocator allocator,
    Buffer* buffer
);
void bufferDestroy(const VmaAllocator allocator, Buffer* buffer);

VkResult bufferCopy(
    const VkQueue graphics_queue,
    const VkCommandPool command_pool,
    const VkDevice logical_device,
    const Buffer source,
    const Buffer destination,
    const VkDeviceSize size
);

/*
void bufferCreateVertex(
    const VkQueue graphics_queue,
    const VkCommandPool command_pool,
    const VkPhysicalDevice physical_device,
    const VkDevice logical_device,
    VkBuffer* vertex_buffer,
    VkDeviceMemory* vertex_buffer_memory
);
void bufferCreateIndex(
    const VkQueue graphics_queue,
    const VkCommandPool command_pool,
    const VkPhysicalDevice physical_device,
    const VkDevice logical_device,
    VkBuffer* index_buffer,
    VkDeviceMemory* index_buffer_memory
);
*/
