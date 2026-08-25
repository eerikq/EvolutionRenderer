#pragma once

#include <volk.h>

VkResult vulkanCreateInstance(VkInstance* instance);
void vulkanDestroyInstance(VkInstance* instance);

VkResult vulkanCreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debug_messenger);
void vulkanDestroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debug_messenger);
