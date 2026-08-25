#pragma once

#include <volk.h>

VkResult pipelineCreateGraphics(
    const VkDevice logical_device,
    const VkSurfaceFormatKHR swapchain_surface_format,
    VkPipeline* graphics_pipeline,
    VkPipelineLayout* pipeline_layout
);
void pipelineDestroyGraphics(const VkDevice logical_device, VkPipeline* graphics_pipeline);
void pipelineDestroyLayout(const VkDevice logical_device, VkPipelineLayout* pipeline_layout);
