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

    std::vector<VkSemaphore> present_complete_semaphores {VK_NULL_HANDLE};
    std::vector<VkSemaphore> render_finished_semaphores {VK_NULL_HANDLE};
    std::vector<VkFence> in_flight_fences {VK_NULL_HANDLE};
} RenderingContext;
