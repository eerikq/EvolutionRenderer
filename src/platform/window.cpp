#include "window.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_vulkan.h>

static SDL_Window* window;

bool windowCreate(int window_width, int window_height) {
    SDL_SetAppMetadata("Evolution Renderer", "1.0.0", "eerikq.evolution-renderer");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Evolution Renderer", window_width, window_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (!window) {
        SDL_Log("Couldn't create window or renderer: %s", SDL_GetError());
        return false;
    }

    return true;
}

void windowDestroy() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool windowCreateSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, surface)) {
        SDL_Log("Couldn't create a Vulkan Surface: %s", SDL_GetError());
        return false;
    }

    return true;
}

void windowDestroySurface(VkInstance instance, VkSurfaceKHR* surface) {
    SDL_Vulkan_DestroySurface(instance, *surface, nullptr);
    *surface = VK_NULL_HANDLE;
}

const char* const* windowGetInstanceExtensions(uint32_t* instance_extensions_count) {
    return SDL_Vulkan_GetInstanceExtensions(instance_extensions_count);
}

// returns size in pixels to account for screen scaling (common on laptops)
bool windowGetSize(int* width, int* height) {
    if (!SDL_GetWindowSizeInPixels(window, width, height)) {
        SDL_Log("Couldn't Get Window Size, %s", SDL_GetError());
        return false;
    }

    return true;
}
