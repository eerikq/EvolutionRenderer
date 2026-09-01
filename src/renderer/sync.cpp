#include "sync.h"

#include "engine_config.h"

void syncCreateObjects(
    const VkDevice logical_device,
    std::vector<VkFence>* in_flight_fences,
    std::vector<VkSemaphore>* render_finished_semaphores,
    std::vector<VkSemaphore>* present_complete_semaphores
) {
    VkSemaphoreCreateInfo semaphore_create_info {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fence_create_info {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    in_flight_fences->clear();
    render_finished_semaphores->clear();
    present_complete_semaphores->clear();

    for (size_t i = 0; i < EngineConfig::max_frames_in_flights; i++) {
        VkSemaphore render_semaphore;
        vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &render_semaphore);
        render_finished_semaphores->emplace_back(render_semaphore);

        VkSemaphore present_semaphore;
        vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &present_semaphore);
        present_complete_semaphores->emplace_back(present_semaphore);

        VkFence fence;
        vkCreateFence(logical_device, &fence_create_info, nullptr, &fence);
        in_flight_fences->emplace_back(fence);
    }
}
