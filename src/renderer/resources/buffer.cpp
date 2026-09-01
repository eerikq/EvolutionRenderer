#include "buffer.h"

#include "types/vertex.h"
#include "util/log.h"

namespace buffer {
    VkResult Create(
        const VkDeviceSize size,
        const VkBufferUsageFlags buffer_flags,
        const VmaAllocationCreateFlags allocation_flags,
        const VmaAllocator allocator,
        Buffer* buffer
    ) {
        VkBufferCreateInfo buffer_info {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = buffer_flags,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        VmaAllocationCreateInfo allocation_info = {
            .flags = allocation_flags,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        return vmaCreateBuffer(allocator, &buffer_info, &allocation_info, &buffer->data, &buffer->allocation, nullptr);
    }

    void Destroy(const VmaAllocator allocator, Buffer* buffer) {
        vmaDestroyBuffer(allocator, buffer->data, buffer->allocation);
        buffer->data = nullptr;
        buffer->allocation = nullptr;
    }

    VkResult CopyFromBuffer(
        const VkQueue graphics_queue,
        const VkCommandPool command_pool,
        const VkDevice logical_device,
        const Buffer source,
        const Buffer destination,
        const VkDeviceSize size
    ) {
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

        VkBufferCopy copy = VkBufferCopy(0, 0, size);
        vkCmdCopyBuffer(temp_command_buffer, source.data, destination.data, 1, &copy);

        vkEndCommandBuffer(temp_command_buffer);

        VkSubmitInfo submit_info {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &temp_command_buffer,
        };
        vkQueueSubmit(graphics_queue, 1, &submit_info, nullptr);
        vkQueueWaitIdle(graphics_queue);

        vkFreeCommandBuffers(logical_device, command_pool, 1, &temp_command_buffer);
        return VkResult::VK_SUCCESS;
    }

    VkResult Write(
        const VmaAllocator allocator,
        const VkQueue graphics_queue,
        const VkCommandPool command_pool,
        const VkDevice logical_device,
        const VkDeviceSize data_size,
        const void* data,
        Buffer* buffer
    ) {
        Buffer staging_buffer;
        VkResult result = Create(
            data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, allocator, &staging_buffer
        );

        if (result != VK_SUCCESS) {
            evoLog(PrintSeverity::Error, "{Couldn't create staging buffer}");
            return result;
        }

        void* mapped_data;
        vmaMapMemory(allocator, staging_buffer.allocation, &mapped_data);
        memcpy(mapped_data, data, data_size);

        vmaFlushAllocation(allocator, staging_buffer.allocation, 0, data_size);
        vmaUnmapMemory(allocator, staging_buffer.allocation);

        buffer::CopyFromBuffer(graphics_queue, command_pool, logical_device, staging_buffer, *buffer, data_size);
        vkQueueWaitIdle(graphics_queue);
        buffer::Destroy(allocator, &staging_buffer);

        return VkResult::VK_SUCCESS;
    }
} // namespace buffer

//
//
//
//
//
// move this shit outta here later
//
//
//
//
//

/*

static void copyBuffer(
    const VkQueue graphics_queue,
    const VkCommandPool command_pool,
    const VkDevice logical_device,
    const VkBuffer source,
    const VkBuffer destination,
    const VkDeviceSize size
) {
    VkCommandBufferAllocateInfo alloc_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer command_buffer_copy;
    vkAllocateCommandBuffers(logical_device, &alloc_info, &command_buffer_copy);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(command_buffer_copy, &begin_info);

    VkBufferCopy copy = VkBufferCopy(0, 0, size);
    vkCmdCopyBuffer(command_buffer_copy, source, destination, 1, &copy);

    vkEndCommandBuffer(command_buffer_copy);

    VkSubmitInfo queue_subtmit_info {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer_copy,
    };
    vkQueueSubmit(graphics_queue, 1, &queue_subtmit_info, nullptr);
}

void bufferCreateVertex(
    const VkQueue graphics_queue,
    const VkCommandPool command_pool,
    const VkPhysicalDevice physical_device,
    const VkDevice logical_device,
    Buffer* vertex_buffer,
) {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    auto [stagingBuffer, stagingBufferMemory] = createBuffer(
        physical_device, logical_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    void* data;
    vkMapMemory(logical_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), bufferSize);
    vkUnmapMemory(logical_device, stagingBufferMemory);

    std::tie(*vertex_buffer, *vertex_buffer_memory) = createBuffer(
        physical_device, logical_device, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    copyBuffer(graphics_queue, command_pool, logical_device, stagingBuffer, *vertex_buffer, bufferSize);
}

void bufferCreateIndex(
    const VkQueue graphics_queue,
    const VkCommandPool command_pool,
    const VkPhysicalDevice physical_device,
    const VkDevice logical_device,
    Buffer* index_buffer
) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    bufferCreate(const VkDeviceSize size, const VkBufferUsageFlags usage, const VmaAllocator allocator, index_buffer) auto
        [stagingBuffer, stagingBufferMemory] = createBuffer(
            physical_device, logical_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

    void* data;
    vkMapMemory(logical_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(logical_device, stagingBufferMemory);

    std::tie(*index_buffer, *index_buffer_memory) = createBuffer(
        physical_device, logical_device, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    copyBuffer(graphics_queue, command_pool, logical_device, stagingBuffer, *index_buffer, bufferSize);
}
*/
