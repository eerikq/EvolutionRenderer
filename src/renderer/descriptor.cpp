#include "descriptor.h"

namespace descriptor {
    VkResult CreateSetLayout(const VkDevice logical_device, VkDescriptorSetLayout* layout) {
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

        return vkCreateDescriptorSetLayout(logical_device, &create_info, nullptr, layout);
    }

    void DestroySetLayout(const VkDevice logical_device, VkDescriptorSetLayout* layout) {
        vkDestroyDescriptorSetLayout(logical_device, *layout, nullptr)
    }
} // namespace descriptor
