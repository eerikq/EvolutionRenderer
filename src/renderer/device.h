#pragma once

#include <volk.h>

bool devicePickPhysicalDevice(const VkInstance instance, VkPhysicalDevice* physical_device);
VkResult deviceCreateLogicalDevice(
    const VkPhysicalDevice physical_device,
    const VkSurfaceKHR surface,
    uint32_t* graphics_queue_index,
    VkDevice* logical_device
);
void deviceDestroyLogicalDevice(VkDevice* logical_device, VkQueue* graphics_queue);

void deviceGetQueue(const VkDevice logical_device, const uint32_t graphics_queue_index, VkQueue* graphics_queue);
