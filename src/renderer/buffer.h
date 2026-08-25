#pragma once

#include <volk.h>

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
