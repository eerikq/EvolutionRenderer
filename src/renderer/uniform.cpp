#include "uniform.h"

#include "engine_config.h"
#include "types/ubo.h"

#include <cstring>

#define GLM_FORCE_RADIANS
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace uniform {
    VkResult CreateBuffers(const VmaAllocator allocator, Buffer* uniform_buffers, void** uniform_buffers_mapped) {
        for (uint32_t i = 0; i < EngineConfig::max_frames_in_flight; i++) {
            buffer::Create(
                sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, allocator, &uniform_buffers[i]
            );

            if (vmaMapMemory(allocator, uniform_buffers[i].allocation, &uniform_buffers_mapped[i]) != VK_SUCCESS) return VK_ERROR_MEMORY_MAP_FAILED;
        }

        return VK_SUCCESS;
    }

    void UpdateBuffers(const int frame_index, const VkExtent2D swapchain_extent, void** uniform_buffers_mapped) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo {
            .model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            .proj = glm::perspective(glm::radians(45.0f), swapchain_extent.width / (float)swapchain_extent.height, 0.1f, 10.0f),
        };
        ubo.proj[1][1] *= -1;

        memcpy(uniform_buffers_mapped[frame_index], &ubo, sizeof(ubo));
    }
} // namespace uniform
