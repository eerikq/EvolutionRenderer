#include "device.h"

#include "util/log.h"
#include "vulkan/vulkan_core.h"

#include <array>
#include <cstring>
#include <vector>

constexpr std::array<const char*, 1> required_device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static bool IsDeviceSuitable(const VkPhysicalDevice physical_device, char* device_name) {
    VkPhysicalDeviceProperties2 device_properties {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
    };
    vkGetPhysicalDeviceProperties2(physical_device, &device_properties);

    evoLog(PrintSeverity::Debug, "Evaluating {} for suitability", device_properties.properties.deviceName);

    // check if device supports vulkan 1.4
    if (device_properties.properties.apiVersion < VK_API_VERSION_1_4) {
        evoLog(PrintSeverity::Error, "{} doesn't support Vulkan 1.4", device_properties.properties.deviceName);
        return false;
    }

    // check if any of the queue families support graphics operations
    uint32_t queue_property_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_property_count, nullptr);
    std::vector<VkQueueFamilyProperties2> queue_properties(queue_property_count);
    for (uint32_t i = 0; i < queue_property_count; i++) {
        queue_properties[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    }
    vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_property_count, queue_properties.data());

    bool supports_graphics = false;
    for (uint32_t i = 0; i < queue_property_count; i++) {
        if (queue_properties[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            supports_graphics = true;
            break;
        }
    }
    if (!supports_graphics) {
        evoLog(PrintSeverity::Error, "{} doesn't support graphics operations", device_properties.properties.deviceName);
        return false;
    }

    // check if all required device extensions are available
    uint32_t extension_property_count = 0;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_property_count, nullptr);
    std::vector<VkExtensionProperties> extension_properties(extension_property_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_property_count, extension_properties.data());

    for (uint32_t i = 0; i < static_cast<uint32_t>(required_device_extensions.size()); i++) {
        bool found_extension = false;

        for (uint32_t j = 0; j < extension_property_count; j++) {
            found_extension = strcmp(required_device_extensions[i], extension_properties[j].extensionName) == 0;
            if (found_extension) break;
        }

        if (!found_extension) {
            evoLog(
                PrintSeverity::Warn, "{} doesn't support the required extension: {}", device_properties.properties.deviceName, required_device_extensions[i]
            );
            return false;
        }
    }

    // check if the physical device supports the required features
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT features_dynamic_state {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
    };
    VkPhysicalDeviceVulkan13Features features_vulkan_13 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features_dynamic_state,
    };
    VkPhysicalDeviceVulkan11Features features_vulkan_11 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &features_vulkan_13,
    };
    VkPhysicalDeviceFeatures2 device_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features_vulkan_11,
    };

    vkGetPhysicalDeviceFeatures2(physical_device, &device_features);

    bool supports_required_features = features_vulkan_11.shaderDrawParameters && features_vulkan_13.dynamicRendering && features_vulkan_13.synchronization2 &&
                                      features_dynamic_state.extendedDynamicState && device_features.features.samplerAnisotropy;

    if (!supports_required_features) {
        evoLog(PrintSeverity::Error, "{} doesn't support required features", device_properties.properties.deviceName);
        return false;
    }

    // copy the deviceName into device_name to make sure the name is printable, even after deviceName goes out of scope
    strncpy(device_name, device_properties.properties.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);

    return true;
}

namespace device {
    VkResult PickPhysical(const VkInstance instance, VkPhysicalDevice* physical_device) {
        uint32_t physical_device_count = 0;
        vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
        evoLog(PrintSeverity::Debug, "Found {} physical devices", physical_device_count);
        std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
        vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

        char device_name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE] {};
        for (uint32_t i = 0; i < physical_device_count; i++) {
            if (IsDeviceSuitable(physical_devices[i], device_name)) {
                *physical_device = physical_devices[i];
                evoLog(PrintSeverity::Info, "Picked physical device: {}", device_name);
                return VK_SUCCESS;
            }
        }

        evoLog(PrintSeverity::Error, "Failed to find a suitable GPU!");
        return VK_ERROR_UNKNOWN;
    }

    VkResult CreateLogical(const VkPhysicalDevice physical_device, const VkSurfaceKHR surface, uint32_t* graphics_queue_index, VkDevice* logical_device) {
        uint32_t queue_property_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_property_count, nullptr);
        std::vector<VkQueueFamilyProperties2> queue_properties(queue_property_count);
        for (uint32_t i = 0; i < queue_property_count; i++) {
            queue_properties[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
        }
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_property_count, queue_properties.data());

        // get the first index into queueFamilyProperties which supports both graphics and present
        for (uint32_t i = 0; i < queue_property_count; i++) {
            VkBool32 supports_surface = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_surface);
            bool supports_graphics = queue_properties[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT;

            if (supports_graphics && supports_surface) {
                *graphics_queue_index = i;
                break;
            }
        }

        // need to revisit this later, not critically important for now.
        if (*graphics_queue_index == VK_QUEUE_FAMILY_IGNORED) throw "Could not find a queue for graphics and present. Terminating";

        float queuePriority = 0.5f;

        VkDeviceQueueCreateInfo device_queue_create_info {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = *graphics_queue_index,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };

        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT features_dynamic_state {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
            .extendedDynamicState = VK_TRUE,
        };
        VkPhysicalDeviceVulkan13Features features_vulkan_13 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &features_dynamic_state,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
        };
        VkPhysicalDeviceVulkan11Features features_vulkan_11 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &features_vulkan_13,
            .shaderDrawParameters = VK_TRUE,
        };
        VkPhysicalDeviceFeatures2 device_features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &features_vulkan_11,
            .features = {.samplerAnisotropy = VK_TRUE},
        };

        VkDeviceCreateInfo device_create_info {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &device_features,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &device_queue_create_info,
            .enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size()),
            .ppEnabledExtensionNames = required_device_extensions.data()
        };

        evoLog(PrintSeverity::Info, "Created logical device");
        return vkCreateDevice(physical_device, &device_create_info, nullptr, logical_device);
    }

    void DestroyLogical(VkDevice* logical_device, VkQueue* graphics_queue) {
        vkDestroyDevice(*logical_device, nullptr);
        *logical_device = VK_NULL_HANDLE;
        *graphics_queue = VK_NULL_HANDLE;
    }

    void GetPhysicalProperties(const VkPhysicalDevice physical_device, VkPhysicalDeviceProperties2* properties) {
        vkGetPhysicalDeviceProperties2(physical_device, properties);
    }

    void GetPhysicalFeatures(const VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures2* features) {
        vkGetPhysicalDeviceFeatures2(physical_device, features);
    }

    void GetLogicalQueue(const VkDevice logical_device, const uint32_t graphics_queue_index, VkQueue* graphics_queue) {
        vkGetDeviceQueue(logical_device, graphics_queue_index, 0, graphics_queue);
        // hardcoded the "queueIndex" input parameter to 0 for now because i only ever create 1 queue in device_queue_create_info.
        // in the future need to get better track of queue families and their queue indices, but not that important at the moment.
        //
        // that also extends to the VkQueue parameters. feels like the "devices" and "queues" should have different contexts,
        // or at the very least some level of separation between the two so its easier to manage multiple queues and their indices
    }
} // namespace device
