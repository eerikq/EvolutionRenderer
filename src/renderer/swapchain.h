#pragma once

#include <volk.h>

VkResult swapchainCreate(
    const VkPhysicalDevice physical_device,
    const VkDevice logical_device,
    const VkSurfaceKHR surface,
    VkSwapchainKHR* swapchain,
    VkExtent2D* swapchain_extent,
    VkSurfaceFormatKHR* swapchain_surface_format
);
void swapchainDestroy(const VkDevice logical_device, VkSwapchainKHR* swapchain, VkImageView* swapchain_image_views, const uint32_t swapchain_image_views_count);
void swapchainGetImages(const VkDevice logical_device, const VkSwapchainKHR swapchain, VkImage* swapchain_images, uint32_t* swapchain_images_count);

void swapchainCreateImageViews(
    const VkDevice logical_device,
    const VkFormat swapchain_surface_format,
    const VkImage* swapchain_images,
    const uint32_t swapchain_images_count,
    VkImageView* swapchain_image_views
);
void swapchainDestroyImageViews(const VkDevice logical_device, VkImageView* swapchain_image_views, const uint32_t swapchain_image_views_count);
