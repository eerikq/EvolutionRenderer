#include "buffer.h"

#include "types/vertex.h"

#include <cstring>
#include <vector>

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

static uint32_t findMemoryType(const VkPhysicalDevice physical_device, const uint32_t type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties {};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw "failed to find suitable memory type!";
}

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

static std::pair<VkBuffer, VkDeviceMemory> createBuffer(
    const VkPhysicalDevice physical_device,
    const VkDevice logical_device,
    const VkDeviceSize size,
    const VkBufferUsageFlags usage,
    const VkMemoryPropertyFlags properties
) {
    VkBufferCreateInfo buffer_info {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VkBuffer buffer;
    vkCreateBuffer(logical_device, &buffer_info, nullptr, &buffer);

    VkMemoryRequirements memory_requirements {};
    vkGetBufferMemoryRequirements(logical_device, buffer, &memory_requirements);

    VkPhysicalDeviceMemoryProperties memory_properties {};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    VkMemoryAllocateInfo alloc_info {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = findMemoryType(physical_device, memory_requirements.memoryTypeBits, properties),
    };

    VkDeviceMemory bufferMemory;
    vkAllocateMemory(logical_device, &alloc_info, nullptr, &bufferMemory);
    vkBindBufferMemory(logical_device, buffer, bufferMemory, 0);

    return {std::move(buffer), std::move(bufferMemory)};
}

void bufferCreateVertex(
    const VkQueue graphics_queue,
    const VkCommandPool command_pool,
    const VkPhysicalDevice physical_device,
    const VkDevice logical_device,
    VkBuffer* vertex_buffer,
    VkDeviceMemory* vertex_buffer_memory
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
    VkBuffer* index_buffer,
    VkDeviceMemory* index_buffer_memory
) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    auto [stagingBuffer, stagingBufferMemory] = createBuffer(
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
