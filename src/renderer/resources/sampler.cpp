#include "sampler.h"

namespace sampler {
    VkResult Create(const VkDevice logical_device, const VkPhysicalDeviceProperties2* physical_properties, VkSampler* sampler) {
        VkSamplerCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VkFilter::VK_FILTER_LINEAR,
            .minFilter = VkFilter::VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = physical_properties->properties.limits.maxSamplerAnisotropy,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        return vkCreateSampler(logical_device, &create_info, nullptr, sampler);
    }

    void Destroy(const VkDevice logical_device, VkSampler* sampler) {
        vkDestroySampler(logical_device, *sampler, nullptr);
    }
} // namespace sampler
