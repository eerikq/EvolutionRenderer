#pragma once

#include <vector>
#include <volk.h>

namespace depth {
    VkFormat FindSupportedFormat(
        const VkPhysicalDevice physical_device,
        const std::vector<VkFormat>& candidates,
        const VkImageTiling tiling,
        const VkFormatFeatureFlags features
    );

    VkFormat FindDepthFormat(const VkPhysicalDevice physical_device);
} // namespace Depth
