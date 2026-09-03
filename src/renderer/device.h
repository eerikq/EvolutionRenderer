#pragma once

#include <volk.h>

namespace device {
    VkResult PickPhysical(const VkInstance instance, VkPhysicalDevice* physical_device);
    VkResult CreateLogical(const VkPhysicalDevice physical_device, const VkSurfaceKHR surface, uint32_t* graphics_queue_index, VkDevice* logical_device);
    void DestroyLogical(VkDevice* logical_device, VkQueue* graphics_queue);

    void GetPhysicalProperties(const VkPhysicalDevice physical_device, VkPhysicalDeviceProperties2* properties);
    void GetPhysicalFeatures(const VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures2* properties);

    void GetLogicalQueue(const VkDevice logical_device, const uint32_t graphics_queue_index, VkQueue* graphics_queue);
} // namespace device
