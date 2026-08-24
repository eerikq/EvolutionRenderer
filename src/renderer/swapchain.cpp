#include "swapchain.h"

#include "platform/window.h"
#include "util/log.h"

#include <algorithm>
#include <vector>

static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

    int width, height;
    windowGetSize(&width, &height);

    return {
        std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

static uint32_t chooseSwapMinImageCount(const VkSurfaceCapabilitiesKHR surfaceCapabilities) {
    uint32_t minImageCount = std::max<uint32_t>(3, surfaceCapabilities.minImageCount);

    if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount)) minImageCount = surfaceCapabilities.maxImageCount;

    return minImageCount;
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormat2KHR* available_formats, const uint32_t available_formats_count) {
    for (uint32_t i = 0; i < available_formats_count; i++) {
        if (available_formats[i].surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_formats[i].surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_formats[i].surfaceFormat;
        }
    }

    return available_formats[0].surfaceFormat;
}

static VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* available_present_modes, const uint32_t available_present_modes_count) {
    for (uint32_t i = 0; i < available_present_modes_count; i++) {
        if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkResult swapchainCreate(const VkPhysicalDevice physical_device, const VkDevice logical_device, const VkSurfaceKHR surface, VkSwapchainKHR* swapchain) {
    // swapchain min image count & extent
    VkSurfaceCapabilitiesKHR surface_capabilities {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

    uint32_t min_image_count = chooseSwapMinImageCount(surface_capabilities);
    VkExtent2D swapchain_extent = chooseSwapExtent(surface_capabilities);

    // swapchain surface format
    uint32_t surface_formats_count = 0;
    VkPhysicalDeviceSurfaceInfo2KHR surface_info {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
        .surface = surface,
    };
    vkGetPhysicalDeviceSurfaceFormats2KHR(physical_device, &surface_info, &surface_formats_count, nullptr);
    std::vector<VkSurfaceFormat2KHR> surface_formats(surface_formats_count);
    for (uint32_t i = 0; i < surface_formats_count; i++) {
        surface_formats[i].sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
    }
    vkGetPhysicalDeviceSurfaceFormats2KHR(physical_device, &surface_info, &surface_formats_count, surface_formats.data());

    VkSurfaceFormatKHR swapchain_surface_format = chooseSwapSurfaceFormat(surface_formats.data(), surface_formats.size());

    // swapchain present mode
    uint32_t present_modes_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, present_modes.data());

    VkPresentModeKHR present_mode = chooseSwapPresentMode(present_modes.data(), present_modes.size());

    VkSwapchainCreateInfoKHR swap_chain_create_info {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = min_image_count,
        .imageFormat = swapchain_surface_format.format,
        .imageColorSpace = swapchain_surface_format.colorSpace,
        .imageExtent = swapchain_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE
    };

    return vkCreateSwapchainKHR(logical_device, &swap_chain_create_info, nullptr, swapchain);
}

void swapchainDestroy(
    const VkDevice logical_device,
    VkSwapchainKHR* swapchain,
    VkImageView* swapchain_image_views,
    const uint32_t swapchain_image_views_count
) {
    swapchainDestroyImageViews(logical_device, swapchain_image_views, swapchain_image_views_count);

    vkDestroySwapchainKHR(logical_device, *swapchain, nullptr);
    *swapchain = VK_NULL_HANDLE;
}

void swapchainGetImages(const VkDevice logical_device, const VkSwapchainKHR swapchain, VkImage* swapchain_images, uint32_t* swapchain_images_count) {
    vkGetSwapchainImagesKHR(logical_device, swapchain, swapchain_images_count, swapchain_images);
}

void swapchainCreateImageViews(
    const VkDevice logical_device,
    const VkFormat swapchain_surface_format,
    const VkImage* swapchain_images,
    const uint32_t swapchain_images_count,
    VkImageView* swapchain_image_views
) {
    VkImageViewCreateInfo image_view_create_info {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchain_surface_format,
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    for (uint32_t i = 0; i < swapchain_images_count; i++) {
        image_view_create_info.image = swapchain_images[i];
        vkCreateImageView(logical_device, &image_view_create_info, nullptr, &(swapchain_image_views[i]));
    }

    evoLog(PrintSeverity::Info, "Created swapchain image views");
}

void swapchainDestroyImageViews(const VkDevice logical_device, VkImageView* swapchain_image_views, const uint32_t swapchain_image_views_count) {
    for (uint32_t i = 0; i < swapchain_image_views_count; i++) {
        vkDestroyImageView(logical_device, swapchain_image_views[i], nullptr);
        swapchain_image_views[i] = VK_NULL_HANDLE;
    }
}
