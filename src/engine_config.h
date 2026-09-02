#pragma once

#include <array>
#include <volk.h>

typedef struct EngineConfig {
    // validation layers
#ifdef NDEBUG
    static constexpr bool enable_validation_layers = false;
#else
    static constexpr bool enable_validation_layers = true;
#endif

    static constexpr std::array<const char*, 1> required_validation_layers = {
        "VK_LAYER_KHRONOS_validation",
    };

    // metadata
    static constexpr const char* application_name = "Evolution Renderer";
    static constexpr const char* application_identifier = "eerikq.evolution-renderer";
    static constexpr const char* version_string = "1.0.0";
    static constexpr uint32_t version_vulkan = VK_MAKE_VERSION(1, 0, 0);
    static constexpr uint32_t vulkan_api_version = VK_API_VERSION_1_4;

    // settings
    static constexpr int window_width = 1200;
    static constexpr int window_height = 900;

    static constexpr int max_frames_in_flight = 2;
} EngineConfig;
