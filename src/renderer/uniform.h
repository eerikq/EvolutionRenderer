#pragma once

#include "renderer/resources/buffer.h"

#include <volk.h>

namespace uniform {
    VkResult CreateBuffers(const VmaAllocator allocator, Buffer* uniform_buffers, void** uniform_buffers_mapped);
    void UpdateBuffers(const int frame_index, const VkExtent2D swapchain_extent, void** uniform_buffers_mapped);
} // namespace uniform
