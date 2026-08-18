#include "platform/window.h"
#include "renderer/vulkan_instance.h"
#include "types/vertex.h"
#include "util/log.h"
#include "vulkan_context.h"

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

constexpr std::array<char const*, 1> required_device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

VulkanContext context;

class Renderer {
  public:
    void run() {
        windowCreate(window_width, window_height);
        evoLog(PrintSeverity::Info, "Created Window. (SDL)");

        init_vulkan();
        main_loop();
        cleanup();
    }

  private:
    // devices
    VkPhysicalDevice physical_device = nullptr;
    VkDevice device = nullptr;
    VkQueue graphics_queue = nullptr;
    // devices

    // swap chain
    VkSwapchainKHR swap_chain = nullptr;
    std::vector<VkImage> swap_chain_images;
    VkSurfaceFormatKHR swap_chain_surface_format;
    VkExtent2D swap_chain_extent;
    std::vector<VkImageView> swap_chain_image_views;
    // swap chain

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
        volkInitialize();
        evoLog(PrintSeverity::Debug, "Initialized Volk.");

        vulkanCreateInstance(&context.instance);
        evoLog(PrintSeverity::Info, "Created Vulkan Instance.");

        volkLoadInstance(context.instance);
        evoLog(PrintSeverity::Warn, "Loading vkInstance into Volk.");

        vulkanCreateDebugMessenger(context.instance, &context.debug_messenger);
        evoLog(PrintSeverity::Error, "Created Debug Messenger.");
        // vulkan initialization

        // windowing
        windowCreateSurface(context.instance, &context.surface);
        evoLog(PrintSeverity::Fatal, "Created Window Surface. (Vulkan)");
        // windowing

        /*
            // device
            pick_physical_device();
            SDL_Log("picked physical device (%s)", physical_device.getProperties().deviceName.data());
            create_logical_device();
            SDL_Log("created logical device");
            volkLoadDevice(device);
            SDL_Log("loading vkDevice into volk");
            // device

            // swap chain
            create_swap_chain();
            SDL_Log("created swap chain");
            create_image_views();
            SDL_Log("created image views");
            create_graphics_pipeline();
            SDL_Log("created graphics pipeline");
            create_command_pool();
            SDL_Log("created command pool");
            // swap chain

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
        windowDestroySurface(context.instance, &context.surface);
        vulkanDestroyDebugMessenger(context.instance, &context.debug_messenger);
        vulkanDestroyInstance(&context.instance);

        windowDestroy();
    }
};

int main() {
    try {
        Renderer program;
        program.run();
    } catch (const std::exception& e) {
        evoLog(PrintSeverity::Info, "Couldn't start the renderer! {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
