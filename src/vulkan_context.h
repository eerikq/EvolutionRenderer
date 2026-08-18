#pragma once

#include <volk.h>

typedef struct VulkanContext {
    // vulkan_instance.cpp
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;

    // window.cpp
    VkSurfaceKHR surface;
} VulkanContext;
