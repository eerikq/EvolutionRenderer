#include "sync.h"

constexpr uint32_t max_frames_in_flights = 2;

void syncCreateObjects(
    const VkDevice logical_device,
    const uint32_t swapchain_images_count,
    std::vector<VkFence>* in_flight_fences,
    std::vector<VkSemaphore>* render_finished_semaphores,
    std::vector<VkSemaphore>* present_complete_semaphores
) {
    VkSemaphoreCreateInfo semaphore_create_info {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fence_create_info {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (size_t i = 0; i < swapchain_images_count; i++) {
        VkSemaphore semaphore;
        vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &semaphore);

        render_finished_semaphores->emplace_back(semaphore);
    }

    for (size_t i = 0; i < max_frames_in_flights; i++) {
        VkSemaphore semaphore;
        vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &semaphore);

        VkFence fence;
        vkCreateFence(logical_device, &fence_create_info, nullptr, &fence);

        present_complete_semaphores->emplace_back(semaphore);
        in_flight_fences->emplace_back(fence);
    }
}
