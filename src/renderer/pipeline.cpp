#include "pipeline.h"

#include "types/vertex.h"
#include "util/file.h"
#include "vulkan/vulkan_core.h"

#include <array>

[[nodiscard]] static VkShaderModule CreateShaderModule(const VkDevice logical_device, const std::vector<char> code) {
    VkShaderModuleCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    VkShaderModule shader_module {};
    if (vkCreateShaderModule(logical_device, &create_info, nullptr, &shader_module) != VK_SUCCESS) return VK_NULL_HANDLE;

    return shader_module;
}

namespace pipeline {
    VkResult CreateGraphics(
        const VkDevice logical_device,
        const VkSurfaceFormatKHR swapchain_surface_format,
        const VkDescriptorSetLayout descriptor_set_layout,
        VkPipeline* graphics_pipeline,
        VkPipelineLayout* pipeline_layout
    ) {
        VkShaderModule shaderModule = CreateShaderModule(logical_device, evoReadFile("shaders/slang.spv"));

        VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = shaderModule,
            .pName = "vertMain",
        };
        VkPipelineShaderStageCreateInfo frag_shader_stage_create_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = shaderModule,
            .pName = "fragMain",
        };
        VkPipelineShaderStageCreateInfo shader_stages[] = {
            vertex_shader_stage_create_info,
            frag_shader_stage_create_info,
        };

        VkVertexInputBindingDescription binding = VertexDescription::binding_description;

        VkPipelineVertexInputStateCreateInfo vertex_input_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &VertexDescription::binding_description,
            .vertexAttributeDescriptionCount = 3,
            .pVertexAttributeDescriptions = VertexDescription::attribute_descriptions,
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        };
        VkPipelineViewportStateCreateInfo viewport_state {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        VkPipelineRasterizationStateCreateInfo rasterizer_state {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f
        };

        VkPipelineMultisampleStateCreateInfo multisampling_state {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
        };

        VkPipelineColorBlendAttachmentState color_blend_attachment {
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineColorBlendStateCreateInfo color_blend_state {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment,
        };

        std::array<VkDynamicState, 2> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };
        VkPipelineDynamicStateCreateInfo dynamic_state {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data(),
        };

        VkPipelineLayoutCreateInfo pipeline_layout_create_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &descriptor_set_layout,
        };

        vkCreatePipelineLayout(logical_device, &pipeline_layout_create_info, nullptr, pipeline_layout);

        VkPipelineRenderingCreateInfo rendering_create_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &swapchain_surface_format.format,
        };
        VkGraphicsPipelineCreateInfo pipeline_create_info {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &rendering_create_info,
            .stageCount = 2,
            .pStages = shader_stages,
            .pVertexInputState = &vertex_input_info,
            .pInputAssemblyState = &input_assembly,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterizer_state,
            .pMultisampleState = &multisampling_state,
            .pColorBlendState = &color_blend_state,
            .pDynamicState = &dynamic_state,
            .layout = *pipeline_layout,
            .renderPass = nullptr,
        };

        return vkCreateGraphicsPipelines(logical_device, nullptr, 1, &pipeline_create_info, nullptr, graphics_pipeline);
    }

    void DestroyGraphics(const VkDevice logical_device, VkPipeline* graphics_pipeline) {
        vkDestroyPipeline(logical_device, *graphics_pipeline, nullptr);
        *graphics_pipeline = VK_NULL_HANDLE;
    }

    void DestroyLayout(const VkDevice logical_device, VkPipelineLayout* pipeline_layout) {
        vkDestroyPipelineLayout(logical_device, *pipeline_layout, nullptr);
        *pipeline_layout = VK_NULL_HANDLE;
    }
} // namespace pipeline
