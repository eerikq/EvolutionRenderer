#pragma once

#include <volk.h>

typedef struct DeviceContext {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice logical_device = VK_NULL_HANDLE;
    VkQueue graphics_queue = VK_NULL_HANDLE;

    uint32_t queue_index = VK_QUEUE_FAMILY_IGNORED;

    // properties
    VkPhysicalDeviceProperties2 device_properties {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
    };

    // features
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT features_dynamic_state {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
    };
    VkPhysicalDeviceVulkan13Features features_vulkan_13 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features_dynamic_state,
    };
    VkPhysicalDeviceVulkan11Features features_vulkan_11 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &features_vulkan_13,
    };
    VkPhysicalDeviceFeatures2 device_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features_vulkan_11,
    };
} DeviceContext;
