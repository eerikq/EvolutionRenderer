#include "texture.h"

#include "renderer/resources/buffer.h"
#include "renderer/resources/vulkan_allocator.h"
#include "util/image.h"
#include "util/log.h"

namespace texture {
    void TransitionImageLayout(
        const VkCommandBuffer command_buffer,
        const VkImage image,
        VkPipelineStageFlags2 src_stage_mask,
        VkPipelineStageFlags2 dst_stage_mask,
        VkAccessFlags2 src_access_mask,
        VkAccessFlags2 dst_access_mask,
        VkImageLayout old_layout,
        VkImageLayout new_layout
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
            .image = image,
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
            .dependencyFlags = 0,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        };

        vkCmdPipelineBarrier2(command_buffer, &dependency_info);
    }

    VkResult Create(
        const VkDevice logical_device,
        const VmaAllocator allocator,
        const VmaAllocationCreateFlags allocation_flags,
        const Image* image,
        Texture* texture
    ) {
        VkFormat format;
        if (image->channels == 4) {
            format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
        } else {
            evoLog(PrintSeverity::Error, "Image has {} channels, need 4.", image->channels);
            return VK_ERROR_FORMAT_NOT_SUPPORTED;
        }

        VkExtent3D extent {
            static_cast<uint32_t>(image->width),
            static_cast<uint32_t>(image->height),
            1,
        };

        VkImageCreateInfo create_info {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VkImageType::VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = extent,
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VmaAllocationCreateInfo allocation_info {
            .flags = allocation_flags,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        return vmaCreateImage(allocator, &create_info, &allocation_info, &texture->data, &texture->allocation, nullptr);
    }

    VkResult CreateView(const VkDevice logical_device, const VkFormat texture_format, Texture* texture) {
        VkImageViewCreateInfo view_create_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = texture->data,
            .viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
            .format = texture_format,
            .components =
                {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        return vkCreateImageView(logical_device, &view_create_info, nullptr, &texture->view);
    }

    void Destroy(const VmaAllocator allocator, const VkDevice logical_device, Texture* texture) {
        if (texture->view != nullptr) {
            vkDestroyImageView(logical_device, texture->view, nullptr);
            texture->view = nullptr;
        }

        vmaDestroyImage(allocator, texture->data, texture->allocation);
        texture->data = nullptr;
        texture->allocation = nullptr;
    }

    VkResult LoadImage(
        const VkDevice logical_device,
        const VkQueue graphics_queue,
        const VkCommandPool command_pool,
        const VmaAllocator allocator,
        const Image* image,
        Texture* texture
    ) {
        Buffer staging_buffer {};
        bufferCreate(
            image->size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, allocator, &staging_buffer
        );

        void* data;
        vmaMapMemory(allocator, staging_buffer.allocation, &data);
        memcpy(data, image->data, image->size);
        vmaUnmapMemory(allocator, staging_buffer.allocation);

        VkCommandBufferAllocateInfo command_buffer_create_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = command_pool,
            .level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        VkCommandBuffer temp_command_buffer;
        vkAllocateCommandBuffers(logical_device, &command_buffer_create_info, &temp_command_buffer);

        VkCommandBufferBeginInfo command_buffer_begin_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        vkBeginCommandBuffer(temp_command_buffer, &command_buffer_begin_info);

        TransitionImageLayout(
            temp_command_buffer, texture->data, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT, 0, VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );

        VkBufferImageCopy region {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource =
                {
                    .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .imageOffset = {0, 0, 0},
            .imageExtent = {static_cast<uint32_t>(image->width), static_cast<uint32_t>(image->height), 1}
        };
        vkCmdCopyBufferToImage(temp_command_buffer, staging_buffer.data, texture->data, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        TransitionImageLayout(
            temp_command_buffer, texture->data, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_ACCESS_2_SHADER_READ_BIT, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        vkEndCommandBuffer(temp_command_buffer);

        VkSubmitInfo submit_info {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &temp_command_buffer,
        };
        vkQueueSubmit(graphics_queue, 1, &submit_info, nullptr);
        vkQueueWaitIdle(graphics_queue);

        bufferDestroy(allocator, &staging_buffer);
        vkFreeCommandBuffers(logical_device, command_pool, 1, &temp_command_buffer);

        return VK_SUCCESS; // simplified return, doesn't know where it failed (it also can't lol). not important right now
    }
} // namespace texture
