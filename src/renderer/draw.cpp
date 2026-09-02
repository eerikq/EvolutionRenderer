#include "draw.h"

#include "engine_config.h"
#include "renderer/command.h"
#include "renderer/uniform.h"
#include "util/log.h"

#include <cassert>

void drawFrame(
    int* frame_index,
    const VkDevice logical_device,
    const VkFence* in_flight_fences,
    const uint32_t in_flight_fences_count,
    const VkSemaphore* present_complete_semaphores,
    const VkSemaphore* render_finished_semaphores,
    const VkSwapchainKHR swapchain,
    const VkQueue graphics_queue,
    const VkCommandBuffer* command_buffers,
    const VkImage* swapchain_images,
    const VkImageView* swapchain_image_views,
    const VkPipeline graphics_pipeline,
    const VkPipelineLayout pipeline_layout,
    const VkDescriptorSet* descriptor_sets,
    const VkExtent2D swapchain_extent,
    const VkBuffer* vertex_buffer,
    const VkBuffer* index_buffer,
    void** uniform_buffers_mapped
) {
    if (vkWaitForFences(logical_device, 1, &in_flight_fences[*frame_index], VK_TRUE, UINT64_MAX) != VK_SUCCESS) throw "failed to wait for fence!";

    vkResetFences(logical_device, 1, &in_flight_fences[*frame_index]);

    uint32_t image_index;
    VkAcquireNextImageInfoKHR acquire_info {
        .sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
        .swapchain = swapchain,
        .timeout = UINT64_MAX,
        .semaphore = present_complete_semaphores[*frame_index],
        .fence = nullptr,
        .deviceMask = 1,
    };
    VkResult acquire_image_result = vkAcquireNextImage2KHR(logical_device, &acquire_info, &image_index);

    /*
    VkResult acquire_image_result = vkAcquireNextImageKHR(
        logical_device, swapchain, UINT64_MAX, present_complete_semaphores[*frame_index], nullptr, &image_index
    );
    */

    if (acquire_image_result == VkResult::VK_ERROR_OUT_OF_DATE_KHR /*|| framebuffer_resized*/) {
        // framebuffer_resized = false;
        // recreate_swap_chain();
        return;
    }

    if (acquire_image_result != VK_SUCCESS && acquire_image_result != VK_SUBOPTIMAL_KHR) {
        assert(acquire_image_result == VK_TIMEOUT || acquire_image_result == VK_NOT_READY);
        throw "failed to acquire swap chain image!";
    }

    commandRecordBuffer(
        *frame_index, image_index, command_buffers, swapchain_images, swapchain_image_views, graphics_pipeline, pipeline_layout, descriptor_sets,
        swapchain_extent, vertex_buffer, index_buffer
    );

    vkQueueWaitIdle(graphics_queue);

    uniform::UpdateBuffers(*frame_index, swapchain_extent, uniform_buffers_mapped);

    VkPipelineStageFlags wait_destionation_stage_mask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    const VkSubmitInfo submit_info {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &present_complete_semaphores[*frame_index],
        .pWaitDstStageMask = &wait_destionation_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffers[*frame_index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &render_finished_semaphores[*frame_index]
    };

    vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[*frame_index]);

    const VkPresentInfoKHR present_info {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &render_finished_semaphores[*frame_index],
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &image_index
    };

    switch (vkQueuePresentKHR(graphics_queue, &present_info)) {
        case VK_SUCCESS: break;
        case VK_SUBOPTIMAL_KHR: evoLog(PrintSeverity::Warn, "vkQueuePresentKHR returned VK_SUBOPTIMAL_KHR!"); break;
        default: break; // an unexpected result is returned!
    }

    *frame_index = (*frame_index + 1) % EngineConfig::max_frames_in_flight;
}
