// fundemental
#include "util/image.h"
#include "util/log.h"
// application
#include "platform/window.h"
// graphics
#include "renderer/command.h"
#include "renderer/device.h"
#include "renderer/draw.h"
#include "renderer/pipeline.h"
#include "renderer/swapchain.h"
#include "renderer/sync.h"
#include "renderer/vulkan_instance.h"
// graphics (resources)
#include "renderer/resources/buffer.h"
#include "renderer/resources/vulkan_allocator.h"
// contexts (engine)
#include "engine_config.h"
#include "engine_state.h"
// contexts (vulkan)
#include "buffer_context.h"
#include "device_context.h"
#include "rendering_context.h"
#include "vulkan_context.h"
// misc.
#include "types/vertex.h"

#include <SDL3/SDL.h>

EngineConfig engine_config;
EngineState engine_state;

BufferContext buffer_context;
DeviceContext device_context;
RenderingContext rendering_context;
VulkanContext vulkan_context;

class Renderer {
  public:
    void run() {
        windowCreate(EngineConfig::window_width, EngineConfig::window_height);
        init_vulkan();
        main_loop();
        cleanup();
    }

  private:
    void init_vulkan() {
        // instance
        volkInitialize(); // add a check here later to ensure VK_SUCCESS is returned
        vulkanCreateInstance(&vulkan_context.instance);
        volkLoadInstance(vulkan_context.instance);
        vulkanCreateDebugMessenger(vulkan_context.instance, &vulkan_context.debug_messenger);

        // window
        windowCreateSurface(vulkan_context.instance, &rendering_context.surface);

        // device
        devicePickPhysicalDevice(vulkan_context.instance, &device_context.physical_device);
        deviceCreateLogicalDevice(device_context.physical_device, rendering_context.surface, &device_context.queue_index, &device_context.logical_device);
        volkLoadDevice(device_context.logical_device);
        deviceGetQueue(device_context.logical_device, device_context.queue_index, &device_context.graphics_queue);

        // allocator
        vulkanAllocatorCreate(vulkan_context.instance, device_context.physical_device, device_context.logical_device, &vulkan_context.allocator);

        // swapchain
        swapchainCreate(
            device_context.physical_device, device_context.logical_device, rendering_context.surface, &rendering_context.swapchain,
            &rendering_context.swapchain_extent, &rendering_context.swapchain_surface_format
        );

        uint32_t swapchain_images_count = 0;
        swapchainGetImages(device_context.logical_device, rendering_context.swapchain, nullptr, &swapchain_images_count);
        rendering_context.swapchain_images.resize(swapchain_images_count);
        swapchainGetImages(device_context.logical_device, rendering_context.swapchain, rendering_context.swapchain_images.data(), &swapchain_images_count);

        rendering_context.swapchain_image_views.resize(swapchain_images_count);
        swapchainCreateImageViews(
            device_context.logical_device, rendering_context.swapchain_surface_format.format, rendering_context.swapchain_images.data(),
            rendering_context.swapchain_images.size(), rendering_context.swapchain_image_views.data()
        );

        pipelineCreateGraphics(
            device_context.logical_device, rendering_context.swapchain_surface_format, &vulkan_context.graphics_pipeline, &vulkan_context.pipeline_layout
        );

        vulkan_context.command_buffers.resize(swapchain_images_count);
        // command
        commandCreatePool(device_context.logical_device, device_context.queue_index, &vulkan_context.command_pool);
        commandAllocateBuffers(device_context.logical_device, vulkan_context.command_pool, vulkan_context.command_buffers.data());

        // buffers
        bufferCreateVertex(
            device_context.graphics_queue, vulkan_context.command_pool, device_context.physical_device, device_context.logical_device,
            &buffer_context.vertex_buffer, &buffer_context.vertex_buffer_memory
        );
        bufferCreateIndex(
            device_context.graphics_queue, vulkan_context.command_pool, device_context.physical_device, device_context.logical_device,
            &buffer_context.index_buffer, &buffer_context.index_buffer_memory
        );

        // sync
        syncCreateObjects(
            device_context.logical_device, static_cast<uint32_t>(rendering_context.swapchain_images.size()), &rendering_context.in_flight_fences,
            &rendering_context.render_finished_semaphores, &rendering_context.present_complete_semaphores
        );

        engine_state.is_running = true;
    }

    void main_loop() {
        Image image;
        if (!imageLoad("small.png", &image)) return;
        evoLog(PrintSeverity::Debug, "Size: {}", image.size);

        while (engine_state.is_running) {
            SDL_Event event;

            drawFrame(
                &engine_state.frame_index, device_context.logical_device, rendering_context.in_flight_fences.data(),
                static_cast<uint32_t>(rendering_context.in_flight_fences.size()), rendering_context.present_complete_semaphores.data(),
                rendering_context.render_finished_semaphores.data(), rendering_context.swapchain, device_context.graphics_queue,
                vulkan_context.command_buffers.data(), rendering_context.swapchain_images.data(), rendering_context.swapchain_image_views.data(),
                vulkan_context.graphics_pipeline, rendering_context.swapchain_extent, &buffer_context.vertex_buffer, &buffer_context.index_buffer
            );

            // evoLog(PrintSeverity::Debug, "drew frame");

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EventType::SDL_EVENT_QUIT) {
                    engine_state.is_running = false;
                } else if (event.type == SDL_EventType::SDL_EVENT_WINDOW_RESIZED) {
                    engine_state.framebuffer_resized = true;
                }
            }
        }
    }

    void cleanup() {
        windowDestroySurface(vulkan_context.instance, &rendering_context.surface);
        vulkanDestroyDebugMessenger(vulkan_context.instance, &vulkan_context.debug_messenger);
        vulkanDestroyInstance(&vulkan_context.instance);

        windowDestroy();
    }
};

int main() {
    try {
        Renderer program;
        program.run();
    } catch (const std::exception& e) {
        evoLog(PrintSeverity::Fatal, "{}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
