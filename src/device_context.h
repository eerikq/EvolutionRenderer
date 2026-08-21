#pragma once

#include <volk.h>

typedef struct DeviceContext {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice logical_device = VK_NULL_HANDLE;
    VkQueue graphics_queue = VK_NULL_HANDLE;

    uint32_t graphics_queue_index = VK_QUEUE_FAMILY_IGNORED;
} DeviceContext;
