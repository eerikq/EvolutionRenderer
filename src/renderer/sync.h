#pragma once

#include <vector>
#include <volk.h>

void syncCreateObjects(
    const VkDevice logical_device,
    std::vector<VkFence>* in_flight_fences,
    std::vector<VkSemaphore>* render_finished_semaphores,
    std::vector<VkSemaphore>* present_complete_semaphores
);
