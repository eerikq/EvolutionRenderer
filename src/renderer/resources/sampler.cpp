#include "sampler.h"

namespace sampler {
    VkResult Create(const VkDevice logical_device, VkSampler* sampler) {
        VkSamplerCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            // unfinished
        };

        return vkCreateSampler(logical_device, &create_info, nullptr, sampler);
    }

    void Destroy(const VkDevice logical_device, VkSampler* sampler) {
        vkDestroySampler(logical_device, *sampler, nullptr);
    }
} // namespace sampler
