// fundemental
#include "util/log.h"
// application
#include "platform/window.h"
// graphics
#include "renderer/device.h"
#include "renderer/swapchain.h"
#include "renderer/vulkan_instance.h"
// contexts
#include "device_context.h"
#include "rendering_context.h"
#include "vulkan_context.h"
// misc.
#include "types/vertex.h"

#include <glm/glm.hpp>

constexpr int window_width = 1200;
constexpr int window_height = 900;
constexpr int max_frames_in_flights = 2;

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

VulkanContext vulkan_context;
DeviceContext device_context;
RenderingContext rendering_context;

class Renderer {
  public:
    void run() {
        windowCreate(window_width, window_height);
        init_vulkan();
        main_loop();
        cleanup();
    }

  private:
    // pipeline
    VkPipelineLayout pipeline_layout = nullptr;
    VkPipeline graphics_pipeline = nullptr;
    VkCommandPool command_pool = nullptr;
    std::vector<VkCommandBuffer> command_buffers;
    // pipeline

    // idk where this goes
    std::vector<VkSemaphore> present_complete_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence> in_flight_fences;
    // idk where this goes

    // buffers
    VkBuffer vertex_buffer = nullptr;
    VkDeviceMemory vertex_buffer_memory = nullptr;
    VkBuffer index_buffer = nullptr;
    VkDeviceMemory index_buffer_memory = nullptr;
    // buffers

    bool framebuffer_resized = false;
    bool is_running = true;

    void init_vulkan() {
        // vulkan initialization
        volkInitialize(); // add a check here later to ensure VK_SUCCESS is returned
        vulkanCreateInstance(&vulkan_context.instance);
        volkLoadInstance(vulkan_context.instance);
        vulkanCreateDebugMessenger(vulkan_context.instance, &vulkan_context.debug_messenger);

        // windowing
        windowCreateSurface(vulkan_context.instance, &rendering_context.surface);

        // device
        devicePickPhysicalDevice(vulkan_context.instance, &device_context.physical_device);
        deviceCreateLogicalDevice(
            device_context.physical_device, rendering_context.surface, &device_context.graphics_queue_index, &device_context.logical_device
        );
        volkLoadDevice(device_context.logical_device);
        deviceGetQueue(device_context.logical_device, device_context.graphics_queue_index, &device_context.graphics_queue);

        // swap chain
        swapchainCreate(device_context.physical_device, device_context.logical_device, rendering_context.surface, &rendering_context.swapchain);

        uint32_t swapchain_images_count = 0;
        swapchainGetImages(device_context.logical_device, rendering_context.swapchain, nullptr, &swapchain_images_count);
        rendering_context.swapchain_images.resize(swapchain_images_count);
        swapchainGetImages(device_context.logical_device, rendering_context.swapchain, rendering_context.swapchain_images.data(), &swapchain_images_count);

        rendering_context.swapchain_image_views.resize(swapchain_images_count);
        swapchainCreateImageViews(
            device_context.logical_device, rendering_context.swapchain_surface_format.format, rendering_context.swapchain_images.data(),
            rendering_context.swapchain_images.size(), rendering_context.swapchain_image_views.data()
        );

        /*
            create_graphics_pipeline();
            SDL_Log("created graphics pipeline");
            create_command_pool();
            SDL_Log("created command pool");

            // buffers
            create_vertex_buffer();
            SDL_Log("created vertex buffer");
            create_index_buffer();
            SDL_Log("created index buffer");
            create_command_buffers();
            SDL_Log("created command buffer");
            // buffers

            create_sync_objects();
            SDL_Log("created sync objects");
        */
    }

    void main_loop() {

        while (is_running) {
            /*
                SDL_Event event;
                //draw_frame();

                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_EVENT_QUIT) {
                        is_running = false;
                    } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                        framebuffer_resized = true;
                    }
                }
            */
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
