#include "descriptor.h"

#include "engine_config.h"
#include "types/ubo.h"

#include <vector>

namespace descriptor {
    VkResult CreateSetLayout(const VkDevice logical_device, VkDescriptorSetLayout* descriptor_layout) {
        VkDescriptorSetLayoutBinding binding {
            .binding = 0,
            .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr
        };

        VkDescriptorSetLayoutCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &binding,
        };

        return vkCreateDescriptorSetLayout(logical_device, &create_info, nullptr, descriptor_layout);
    }

    void DestroySetLayout(const VkDevice logical_device, VkDescriptorSetLayout* descriptor_layout) {
        vkDestroyDescriptorSetLayout(logical_device, *descriptor_layout, nullptr);
    }

    VkResult CreatePool(const VkDevice logical_device, VkDescriptorPool* descriptor_pool) {
        VkDescriptorPoolSize pool_size {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = EngineConfig::max_frames_in_flight,
        };

        VkDescriptorPoolCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = EngineConfig::max_frames_in_flight,
            .poolSizeCount = 1,
            .pPoolSizes = &pool_size,
        };

        return vkCreateDescriptorPool(logical_device, &create_info, nullptr, descriptor_pool);
    }

    void DestroyPool(const VkDevice logical_device, VkDescriptorPool* descriptor_pool) {
        vkDestroyDescriptorPool(logical_device, *descriptor_pool, nullptr);
    }

    VkResult CreateSets(
        const VkDevice logical_device,
        const VkDescriptorSetLayout descriptor_set_layout,
        const VkDescriptorPool descriptor_pool,
        VkDescriptorSet* descriptor_sets
    ) {
        std::vector<VkDescriptorSetLayout> layouts(EngineConfig::max_frames_in_flight, descriptor_set_layout);

        VkDescriptorSetAllocateInfo alloc_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptor_pool,
            .descriptorSetCount = EngineConfig::max_frames_in_flight,
            .pSetLayouts = layouts.data(),
        };

        return vkAllocateDescriptorSets(logical_device, &alloc_info, descriptor_sets);
    }

    VkResult ConfigureSets(const VkDevice logical_device, const Buffer* uniform_buffers, const VkDescriptorSet* descriptor_sets) {
        for (size_t i = 0; i < EngineConfig::max_frames_in_flight; i++) {
            VkDescriptorBufferInfo buffer_info {
                .buffer = uniform_buffers[i].data,
                .offset = 0,
                .range = sizeof(UniformBufferObject),
            };

            VkWriteDescriptorSet descriptor_write {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptor_sets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &buffer_info,
            };

            vkUpdateDescriptorSets(logical_device, 1, &descriptor_write, 0, nullptr);
        }

        return VK_SUCCESS;
    }

    void DestroySets(
        const VkDevice logical_device,
        const VkDescriptorPool descriptor_pool,
        const uint32_t descriptor_set_count,
        VkDescriptorSet* descriptor_sets
    ) {
        vkFreeDescriptorSets(logical_device, descriptor_pool, descriptor_set_count, descriptor_sets);
    }
} // namespace descriptor
