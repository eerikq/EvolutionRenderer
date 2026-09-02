#pragma once

#include <volk.h>

namespace pipeline {
    VkResult CreateGraphics(
        const VkDevice logical_device,
        const VkSurfaceFormatKHR swapchain_surface_format,
        const VkDescriptorSetLayout descriptor_set_layout,
        VkPipeline* graphics_pipeline,
        VkPipelineLayout* pipeline_layout
    );
    void DestroyGraphics(const VkDevice logical_device, VkPipeline* graphics_pipeline);
    void DestroyLayout(const VkDevice logical_device, VkPipelineLayout* pipeline_layout);
} // namespace pipeline
