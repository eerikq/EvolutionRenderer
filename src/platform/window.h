#pragma once

#include <volk.h>

bool windowCreate(int window_width, int window_height);
void windowDestroy();

bool windowCreateSurface(const VkInstance instance, VkSurfaceKHR* surface);
void windowDestroySurface(const VkInstance instance, VkSurfaceKHR* surface);

const char* const* windowGetInstanceExtensions(uint32_t* count);
bool windowGetSize(int* width, int* height);
