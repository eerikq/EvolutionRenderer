#pragma once

#include <volk.h>
//
#include <vk_mem_alloc.h>

typedef struct Texture {
    VkImage data = nullptr;
    VkImageView view = nullptr;
    VmaAllocation allocation = nullptr;
} Texture;
