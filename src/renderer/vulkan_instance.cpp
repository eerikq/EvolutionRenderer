#include "vulkan_instance.h"

#include "../platform/window.h"

#include <SDL3/SDL_log.h>
#include <array>
#include <vector>

#if NDEBUG
constexpr bool enable_validation_layers = false;
#else
constexpr bool enable_validation_layers = true;
#endif

// forward declaration. kinda goofy but what can you do
std::vector<const char*> get_required_intance_extensions();

constexpr std::array<char const*, 1> validation_layers = {
    "VK_LAYER_KHRONOS_validation",
};

constexpr std::array<char const*, 1> required_device_extension = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

void vulkanInitialize(VkInstance* instance) {
    SDL_Log("Initiating Vulkan. Validation Layers Enabled: %b", enable_validation_layers);

    constexpr VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Evolution Renderer",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_4,
    };

    // this one's kinda confusing, but the gist is that you can change what required_layers_names points towards (although it has to be
    // another, "inner" pointer), but you cant change what that inner pointer is pointing at, nor the underlying char values of that pointer.
    char const* const* required_layers_names = enable_validation_layers ? validation_layers.data() : nullptr;
    uint32_t required_layers_count = enable_validation_layers ? static_cast<uint32_t>(validation_layers.size()) : 0;

    uint32_t property_count;
    vkEnumerateInstanceLayerProperties(&property_count, nullptr);
    // unsure how i feel about the vector use here. ideally i wouldn't, but doing this with arrays is way uglier.
    // also the vector is just in the function's local scope, so its just a one time thing that is very controlled so it gets a pass.
    std::vector<VkLayerProperties> layer_properties(property_count);
    vkEnumerateInstanceLayerProperties(&property_count, layer_properties.data());

    if (enable_validation_layers) {
        for (int i = 0; i < required_layers_count; i++) {
            bool found_layers = strcmp(layer_properties.data()->layerName, required_layers_names) == 0;

            if (!found_layers) {
                SDL_Log("Required layer not supported: %s", "get the layer here later");
                break;
            }
        }

        /*
        auto unsupportedLayerIt = std::ranges::find_if(required_layers_names, [&layer_properties](auto const& requiredLayer) {
            return std::ranges::none_of(layer_properties, [requiredLayer](auto const& layerProperty) {
                return strcmp(layerProperty.layerName, requiredLayer) == 0;
            });
        });

        if (unsupportedLayerIt != required_layers_names.end()) {
            throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
        }
        */
    }

    //
    // the following 15 lines are basically the same as whatever happens above
    //

    // get_required_intance_extensions() is defined lower down
    std::vector<const char*> required_extensions = get_required_intance_extensions();

    // check if the required extensions are supported by the Vulkan implementation
    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    auto unsupportedPropertyIt = std::ranges::find_if(required_extensions, [&extensionProperties](auto const& requiredExtension) {
        return std::ranges::none_of(extensionProperties, [requiredExtension](auto const& extensionProperty) {
            return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
        });
    });

    if (unsupportedPropertyIt != required_extensions.end()) {
        throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
    }

    VkInstanceCreateInfo createInfo = {
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = required_layers_count,
        .ppEnabledLayerNames = required_layers_names,
        .enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
        .ppEnabledExtensionNames = required_extensions.data(),
    };

    vkCreateInstance(&createInfo, nullptr, &instance);
    // would be nice to have this return a VkResult just in case, but i dont know enough about that yet.
}

void vulkanCleanup(VkInstance instance) {
    vkDestroyInstance(instance, nullptr);
}

std::vector<const char*> get_required_intance_extensions() {
    uint32_t instance_extensions_count = 0;
    char const* const* instance_extensions = windowGetInstanceExtensions(&instance_extensions_count);

    std::vector extensions(instance_extensions, instance_extensions + instance_extensions_count);
    if (enable_validation_layers) {
        // append "vk::EXTDebugUtilsExtensionName" to the end of extensions to enable validation layers
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void setup_debug_messenger() {
    if (!enable_validation_layers) return;

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
    );
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
    );
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
        .messageSeverity = severityFlags, .messageType = messageTypeFlags, .pfnUserCallback = &debug_callback
    };

    debug_messenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData
) {
    std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

    return vk::False;
}
