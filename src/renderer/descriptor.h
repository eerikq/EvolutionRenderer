#pragma once

#include "renderer/resources/buffer.h"

#include <volk.h>

namespace descriptor {
    VkResult CreateSetLayout(const VkDevice logical_device, VkDescriptorSetLayout* descriptor_layout);
    void DestroySetLayout(const VkDevice logical_device, VkDescriptorSetLayout* descriptor_layout);

    VkResult CreatePool(const VkDevice logical_device, VkDescriptorPool* descriptor_pool);
    void DestroyPool(const VkDevice logical_device, VkDescriptorPool* descriptor_pool);

    VkResult CreateSets(
        const VkDevice logical_device,
        const VkDescriptorSetLayout descriptor_set_layout,
        const VkDescriptorPool descriptor_pool,
        VkDescriptorSet* descriptor_set
    );
    VkResult ConfigureSets(const VkDevice logical_device, const Buffer* uniform_buffers, const VkDescriptorSet* descriptor_sets);
    void DestroySets(
        const VkDevice logical_device,
        const VkDescriptorPool descriptor_pool,
        const uint32_t descriptor_set_count,
        VkDescriptorSet* descriptor_sets
    );
} // namespace descriptor
