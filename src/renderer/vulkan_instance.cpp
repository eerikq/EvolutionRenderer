#include "vulkan_instance.h"

#include "../platform/window.h"
#include "util/log.h"

#include <array>
#include <cstring>
#include <vector>

#ifdef NDEBUG
constexpr bool enable_validation_layers = false;
#else
constexpr bool enable_validation_layers = true;
#endif

constexpr std::array<const char*, 1> required_validation_layers = {
    "VK_LAYER_KHRONOS_validation",
};

static bool validateInstanceLayers(const uint32_t layers_count, const char* const* layers_names) {
    uint32_t layer_property_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_property_count, nullptr);

    std::vector<VkLayerProperties> layer_properties(layer_property_count);
    vkEnumerateInstanceLayerProperties(&layer_property_count, layer_properties.data());

    for (uint32_t i = 0; i < layers_count; i++) {
        bool found_layer = false;

        for (uint32_t j = 0; j < layer_property_count; j++) {
            found_layer = strcmp(layers_names[i], layer_properties[j].layerName) == 0;
            if (found_layer) break;
        }

        if (!found_layer) {
            debugLog(PrintSeverity::Error, "Required layer not supported: {}", layers_names[i]);
            return false;
        }
    }

    return true;
}

static bool validateInstanceExtensions(const uint32_t extensions_count, const char* const* extensions_names) {
    uint32_t extension_property_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_property_count, nullptr);

    std::vector<VkExtensionProperties> extension_properties(extension_property_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_property_count, extension_properties.data());

    for (uint32_t i = 0; i < extensions_count; i++) {
        bool found_extension = false;

        for (uint32_t j = 0; j < extension_property_count; j++) {
            found_extension = strcmp(extensions_names[i], extension_properties[j].extensionName) == 0;
            if (found_extension) break;
        }

        if (!found_extension) {
            debugLog(PrintSeverity::Error, "Required extension not supported: {}", extensions_names[i]);
            return false;
        }
    }

    return true;
}

VkResult vulkanCreateInstance(VkInstance* instance) {
    constexpr VkApplicationInfo app_info {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Evolution Renderer",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_4,
    };

    // instance (validation) layers
    uint32_t validation_layers_count = enable_validation_layers ? static_cast<uint32_t>(required_validation_layers.size()) : 0;
    const char* const* validation_layers_names = enable_validation_layers ? required_validation_layers.data() : nullptr;

    if (!validateInstanceLayers(validation_layers_count, validation_layers_names)) return VK_ERROR_LAYER_NOT_PRESENT;

    // instance extensions
    uint32_t extensions_count = 0;
    const char* const* extension_names = windowGetInstanceExtensions(&extensions_count);

    std::vector extensions(extension_names, extension_names + extensions_count);
    if (enable_validation_layers) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    // append "VK_EXT_DEBUG_UTILS_EXTENSION_NAME" to the end of extensions to enable validation layers

    if (!validateInstanceExtensions(static_cast<uint32_t>(extensions.size()), extensions.data())) return VK_ERROR_EXTENSION_NOT_PRESENT;

    VkInstanceCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = validation_layers_count,
        .ppEnabledLayerNames = validation_layers_names,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    debugLog(PrintSeverity::Info, "Created Vulkan instance. Validation layers: {}", enable_validation_layers ? "Enabled" : "Disabled");
    return vkCreateInstance(&create_info, nullptr, instance);
}

void vulkanDestroyInstance(VkInstance* instance) {
    vkDestroyInstance(*instance, nullptr);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* p_user_data
) {
    debugLog(PrintSeverity::Warn, "[{}] {}", type, p_callback_data->pMessage);
    return VK_FALSE;
}

VkResult vulkanCreateDebugMessenger(const VkInstance instance, VkDebugUtilsMessengerEXT* debug_messenger) {
    if (!enable_validation_layers) {
        debugLog(PrintSeverity::Warn, "Validation layers are disabled! Debug messenger will not be created");
        return VK_SUCCESS;
    }

    VkDebugUtilsMessageSeverityFlagsEXT severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    VkDebugUtilsMessageTypeFlagsEXT message_type_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    VkDebugUtilsMessengerCreateInfoEXT create_info {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = severity_flags,
        .messageType = message_type_flags,
        .pfnUserCallback = &debugCallback,
    };

    debugLog(PrintSeverity::Info, "Created debug messenger");
    return vkCreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, debug_messenger);
}

void vulkanDestroyDebugMessenger(const VkInstance instance, VkDebugUtilsMessengerEXT* debug_messenger) {
    vkDestroyDebugUtilsMessengerEXT(instance, *debug_messenger, nullptr);
}
