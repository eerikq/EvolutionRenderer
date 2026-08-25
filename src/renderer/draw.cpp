#include "draw.h"

void drawFrame() {
    auto fenceResult = device.waitForFences(*in_flight_fences[frame_index], vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) throw std::runtime_error("failed to wait for fence!");

    auto [result, imageIndex] = swap_chain.acquireNextImage(UINT64_MAX, *present_complete_semaphores[frame_index], nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR || framebuffer_resized) {
        framebuffer_resized = false;
        recreate_swap_chain();
        return;
    }

    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    device.resetFences(*in_flight_fences[frame_index]);

    command_buffers[frame_index].reset();
    record_command_buffer(imageIndex);

    graphics_queue.waitIdle();

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*present_complete_semaphores[frame_index],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*command_buffers[frame_index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*render_finished_semaphores[frame_index]
    };

    graphics_queue.submit(submitInfo, *in_flight_fences[frame_index]);

    const vk::PresentInfoKHR presentInfoKHR {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*render_finished_semaphores[frame_index],
        .swapchainCount = 1,
        .pSwapchains = &*swap_chain,
        .pImageIndices = &imageIndex
    };

    result = graphics_queue.presentKHR(presentInfoKHR);
    switch (result) {
        case VkResult::::eSuccess: break;
        case VkResult::eSuboptimalKHR: std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n"; break;
        default: break; // an unexpected result is returned!
    }

    frame_index = (frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}
