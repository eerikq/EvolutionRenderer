#pragma once

#include <volk.h>

namespace sampler {
    VkResult Create(const VkDevice logical_device, VkSampler* sampler);
    void Destroy(const VkDevice logical_device, VkSampler* sampler);
} // namespace sampler
