#pragma once

#include <volk.h>

namespace window {
    bool Create();
    void Destroy();

    bool CreateSurface(const VkInstance instance, VkSurfaceKHR* surface);
    void DestroySurface(const VkInstance instance, VkSurfaceKHR* surface);

    const char* const* GetInstanceExtensions(uint32_t* count);
    bool GetSizeInPixels(int* width, int* height);
} // namespace window
