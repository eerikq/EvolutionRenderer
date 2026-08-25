#pragma once

#include <vector>
#include <volk.h>

typedef struct RenderingContext {
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkSurfaceFormatKHR swapchain_surface_format {};
    VkExtent2D swapchain_extent {};

    std::vector<VkImage> swapchain_images {VK_NULL_HANDLE};
    std::vector<VkImageView> swapchain_image_views {VK_NULL_HANDLE};
} RenderingContext;
