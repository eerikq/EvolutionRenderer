#pragma once

#include <volk.h>

namespace descriptor {
    VkResult CreateSetLayout(const VkDevice logical_device, VkDescriptorSetLayout* layout);
}
