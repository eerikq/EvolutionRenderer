#include "command.h"

#include "engine_config.h"
#include "types/vertex.h"

#include <vector>

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

VkResult commandCreatePool(const VkDevice logical_device, const uint32_t queue_index, VkCommandPool* command_pool) {
    VkCommandPoolCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_index,
    };

    return vkCreateCommandPool(logical_device, &create_info, nullptr, command_pool);
}

void commandDestroyPool(const VkDevice logical_device, VkCommandPool* command_pool) {
    vkDestroyCommandPool(logical_device, *command_pool, nullptr);
    command_pool = VK_NULL_HANDLE;
}

VkResult commandAllocateBuffers(const VkDevice logical_device, const VkCommandPool command_pool, VkCommandBuffer* command_buffers) {
    VkCommandBufferAllocateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = EngineConfig::max_frames_in_flights,
    };

    return vkAllocateCommandBuffers(logical_device, &create_info, command_buffers);
}

void commandFreeBuffers(
    const VkDevice logical_device,
    const VkCommandPool command_pool,
    VkCommandBuffer* command_buffers,
    const uint32_t command_buffers_count
) {
    vkFreeCommandBuffers(logical_device, command_pool, command_buffers_count, command_buffers);
    for (uint32_t i = 0; i < command_buffers_count; i++) {
        command_buffers[i] = VK_NULL_HANDLE;
    }
}

static void transition_image_layout(
    const VkCommandBuffer* command_buffers,
    const VkImage* swapchain_images,
    const uint32_t frame_index,
    const uint32_t image_index,
    VkImageLayout old_layout,
    VkImageLayout new_layout,
    VkAccessFlags2 src_access_mask,
    VkAccessFlags2 dst_access_mask,
    VkPipelineStageFlags2 src_stage_mask,
    VkPipelineStageFlags2 dst_stage_mask
) {
    VkImageMemoryBarrier2 barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchain_images[image_index],
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    VkDependencyInfo dependency_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
    };

    vkCmdPipelineBarrier2(command_buffers[frame_index], &dependency_info);
}

void commandRecordBuffer(
    const uint32_t frame_index,
    const uint32_t image_index,
    const VkCommandBuffer* command_buffers,
    const VkImage* swapchain_images,
    const VkImageView* swapchain_image_views,
    const VkPipeline graphics_pipeline,
    const VkExtent2D swapchain_extent,
    const VkBuffer* vertex_buffer,
    const VkBuffer* index_buffer
) {
    VkCommandBuffer command_buffer = command_buffers[frame_index];

    VkCommandBufferBeginInfo begin_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    vkBeginCommandBuffer(command_buffer, &begin_info);

    transition_image_layout(
        command_buffers, swapchain_images, frame_index, image_index, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, {},
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
    );

    VkClearValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
    VkRenderingAttachmentInfo attachment_info {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchain_image_views[image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clear_color
    };

    VkRenderingInfo rendering_info {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea =
            {
                .offset = {0, 0},
                .extent = swapchain_extent,
            },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment_info,
    };

    vkCmdBeginRendering(command_buffer, &rendering_info);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    VkViewport viewport = VkViewport(0.0f, 0.0f, static_cast<float>(swapchain_extent.width), static_cast<float>(swapchain_extent.height), 0.0f, 1.0f);
    VkRect2D offset = VkRect2D(VkOffset2D(0, 0), swapchain_extent);
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &offset);

    VkDeviceSize device_size = 0;
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffer, &device_size);
    vkCmdBindIndexBuffer(command_buffer, *index_buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    vkCmdEndRendering(command_buffer);

    transition_image_layout(
        command_buffers, swapchain_images, frame_index, image_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, {}, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
    );

    vkEndCommandBuffer(command_buffer);
}
