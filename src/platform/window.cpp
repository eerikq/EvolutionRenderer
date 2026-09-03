#include "window.h"

#include "engine_config.h"
#include "util/log.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>

static SDL_Window* sdl_window;

namespace window {
    bool Create() {
        SDL_SetAppMetadata(EngineConfig::application_name, EngineConfig::version_string, EngineConfig::application_identifier);

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            evoLog(PrintSeverity::Error, "Failed to initialize SDL: {}", SDL_GetError());
            return false;
        }

        sdl_window = SDL_CreateWindow(
            EngineConfig::application_name, EngineConfig::window_width, EngineConfig::window_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
        );
        if (!sdl_window) {
            evoLog(PrintSeverity::Error, "Failed to create window: {}", SDL_GetError());
            return false;
        }

        evoLog(PrintSeverity::Info, "Created window");
        return true;
    }

    void Destroy() {
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
    }

    bool CreateSurface(const VkInstance instance, VkSurfaceKHR* surface) {
        if (!SDL_Vulkan_CreateSurface(sdl_window, instance, nullptr, surface)) {
            evoLog(PrintSeverity::Error, "Failed to create a Vulkan surface: {}", SDL_GetError());
            return false;
        }

        evoLog(PrintSeverity::Info, "Created window surface");
        return true;
    }

    void DestroySurface(const VkInstance instance, VkSurfaceKHR* surface) {
        SDL_Vulkan_DestroySurface(instance, *surface, nullptr);
        *surface = VK_NULL_HANDLE;
    }

    const char* const* GetInstanceExtensions(uint32_t* count) {
        return SDL_Vulkan_GetInstanceExtensions(count);
    }

    // returns size in pixels to account for screen scaling (common on laptops)
    bool GetSizeInPixels(int* width, int* height) {
        if (!SDL_GetWindowSizeInPixels(sdl_window, width, height)) {
            evoLog(PrintSeverity::Error, "Failed to get window size: {}", SDL_GetError());
            return false;
        }

        return true;
    }
} // namespace window
