#include "depth.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace depth {
    VkFormat FindSupportedFormat(
        const VkPhysicalDevice physical_device,
        const std::vector<VkFormat>& candidates,
        const VkImageTiling tiling,
        const VkFormatFeatureFlags features
    ) {
        for (int i = 0; i < candidates.size(); i++) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physical_device, candidates[i], &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return candidates[i];
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return candidates[i];
            }
        }

        return VkFormat::VK_FORMAT_UNDEFINED;
    }

    VkFormat FindDepthFormat(const VkPhysicalDevice physical_device) {
        return depth::FindSupportedFormat(
            physical_device, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool HasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }
} // namespace Depth
