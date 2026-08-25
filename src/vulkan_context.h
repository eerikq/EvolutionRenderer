#pragma once

#include <vector>
#include <volk.h>

typedef struct VulkanContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;

    // everything below this line should probably be moved elsewhere but im not sure where yet
    VkPipelineLayout pipeline_layout = nullptr;
    VkPipeline graphics_pipeline = nullptr;

    VkCommandPool command_pool = nullptr;
    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkSemaphore> present_complete_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence> in_flight_fences;

    uint32_t frame_index = 0;
    const uint32_t max_frames_in_flights = 2;
} VulkanContext;
