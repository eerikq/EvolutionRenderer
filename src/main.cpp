#include "vulkan/vulkan.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <filesystem>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 900

#if NDEBUG
bool enable_validation_layers = false;
#else
bool enable_validation_layers = true;
#endif

class Triangle {
    public:
        void run() {
            init_sdl();
            init_vulkan();
            main_loop();
            cleanup();
        }

    private:
        SDL_Window* window = nullptr;

        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;
        vk::raii::DebugUtilsMessengerEXT debug_messenger = nullptr;

        vk::raii::SurfaceKHR surface = nullptr;
        vk::raii::PhysicalDevice physical_device = nullptr;
        vk::raii::Device device = nullptr;
        vk::raii::Queue graphics_queue = nullptr;
       	vk::raii::SwapchainKHR swap_chain = nullptr;

        std::vector<vk::Image> swapChainImages;
    	vk::SurfaceFormatKHR swapChainSurfaceFormat;
    	vk::Extent2D swapChainExtent;
        std::vector<vk::raii::ImageView> swap_chain_image_views;

        vk::raii::PipelineLayout pipelineLayout = nullptr;
        vk::raii::Pipeline graphics_pipeline = nullptr;

        const std::vector<char const*> validation_layers = {
            "VK_LAYER_KHRONOS_validation"
        };

        std::vector<const char*> required_device_extension = {
            vk::KHRSwapchainExtensionName
        };

        void init_sdl() {
            SDL_SetAppMetadata("Evolution Renderer", "1.0.0", "eerikq.evolution-renderer");

            if (!SDL_Init(SDL_INIT_VIDEO)) {
                SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
            }

            window = SDL_CreateWindow("Evolution Renderer", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
            if (!window) {
                SDL_Log("Couldn't create window or renderer: %s", SDL_GetError());
            }
        }

        std::vector<const char*> get_required_intance_extensions() {
            uint32_t instance_extensions_count = 0;
            char const* const* instance_extensions = SDL_Vulkan_GetInstanceExtensions(&instance_extensions_count);

            std::vector extensions(instance_extensions, instance_extensions + instance_extensions_count);
            if (enable_validation_layers) {
                // append "vk::EXTDebugUtilsExtensionName" to the end of extensions to enable validation layers
                extensions.push_back(vk::EXTDebugUtilsExtensionName);
            }

            return extensions;
        }

        void create_instance() {
            constexpr vk::ApplicationInfo appInfo {
                .pApplicationName = "Evolution Renderer",
                .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                .pEngineName = "No Engine",
                .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                .apiVersion = vk::ApiVersion14,
            };

            std::vector<char const*> required_layers;
            if (enable_validation_layers) {
                required_layers.assign(validation_layers.begin(), validation_layers.end());
            }

            // check if the required layers are supported by the Vulkan implementation
            auto layer_properties = context.enumerateInstanceLayerProperties();
            auto unsupportedLayerIt = std::ranges::find_if(required_layers, [&layer_properties](auto const &requiredLayer) {
                return std::ranges::none_of(layer_properties, [requiredLayer](auto const &layerProperty) {
                    return strcmp(layerProperty.layerName, requiredLayer) == 0;
                });
            });

      		if (unsupportedLayerIt != required_layers.end()) {
     			throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
      		}

    		std::vector<const char*> required_extensions = get_required_intance_extensions();

    		// check if the required extensions are supported by the Vulkan implementation.
    		auto extensionProperties = context.enumerateInstanceExtensionProperties();
    		auto unsupportedPropertyIt = std::ranges::find_if(required_extensions, [&extensionProperties](auto const &requiredExtension) {
    			 return std::ranges::none_of(extensionProperties, [requiredExtension](auto const &extensionProperty) {
                    return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
                });
            });

            if (unsupportedPropertyIt != required_extensions.end()) {
    			throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
    		}

            vk::InstanceCreateInfo createInfo {
                .pApplicationInfo = &appInfo,
                .enabledLayerCount = static_cast<uint32_t>(required_layers.size()),
                .ppEnabledLayerNames = required_layers.data(),
                .enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
                .ppEnabledExtensionNames = required_extensions.data(),
            };

            instance = vk::raii::Instance(context, createInfo);
        }

        void setup_debug_messenger() {
            if(!enable_validation_layers) return;

            vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
            vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
            vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT {
                .messageSeverity = severityFlags,
                .messageType = messageTypeFlags,
                .pfnUserCallback = &debug_callback
            };

            debug_messenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
        }

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(
            vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
            vk::DebugUtilsMessageTypeFlagsEXT type,
            const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        ){
            std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

            return vk::False;
        }

        void create_surface() {
            VkSurfaceKHR _surface;
            if (!SDL_Vulkan_CreateSurface(window, *instance, nullptr, &_surface)) {
                SDL_Log("Couldn't create a Vulkan Surface: %s", SDL_GetError());
            }

            surface = vk::raii::SurfaceKHR(instance, _surface);
        }

        bool isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice) {
    		// Check if the physicalDevice supports the Vulkan 1.4 API version
    		bool supportsVulkan14 = physicalDevice.getProperties().apiVersion >= VK_API_VERSION_1_4;

    		// Check if any of the queue families support graphics operations
    		auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    		bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const &qfp) {
    		    return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
    		});

    		// Check if all required physicalDevice extensions are available
    		auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    		bool supportsAllRequiredExtensions = std::ranges::all_of(required_device_extension, [&availableDeviceExtensions](auto const &requiredDeviceExtension) {
                return std::ranges::any_of(availableDeviceExtensions, [requiredDeviceExtension](auto const &availableDeviceExtension) {
                    return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0;
                });
            });

    		// Check if the physicalDevice supports the required features
    		auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    		bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
    		                                features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
    		                                features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

    		// Return true if the physicalDevice meets all the criteria
    		return supportsVulkan14 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
	    }

        void pick_physical_device() {
            std::vector<vk::raii::PhysicalDevice> physical_devices = instance.enumeratePhysicalDevices();
            auto const devIter = std::ranges::find_if(physical_devices, [&](auto const &physical_device) {
                return isDeviceSuitable(physical_device);
            });

            if (devIter == physical_devices.end()) {
                throw std::runtime_error("failed to find a suitable GPU!");
            }

            physical_device = *devIter;
        }

        void create_logical_device() {
            std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physical_device.getQueueFamilyProperties();

            // get the first index into queueFamilyProperties which supports both graphics and present
            uint32_t queueIndex = ~0;
            for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
                if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) && physical_device.getSurfaceSupportKHR(qfpIndex, *surface)) {
                    // found a queue family that supports both graphics and present
                    queueIndex = qfpIndex;
                    break;
                }
            }

            if (queueIndex == ~0) throw std::runtime_error("Could not find a queue for graphics and present -> terminating");

            float queuePriority = 0.5f;

            vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
                .queueFamilyIndex = queueIndex,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority,
            };

            vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
                {}, // vk::PhysicalDeviceFeatures2 (empty for now)
                {.shaderDrawParameters = true}, // shader draw parameters from Vulkan 1.1
                {.dynamicRendering = true}, // dynamic rendering from Vulkan 1.3
                {.extendedDynamicState = true} // extended dynamic state from the extension
            };


            vk::DeviceCreateInfo deviceCreateInfo {
                .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                .queueCreateInfoCount = 1,
                .pQueueCreateInfos = &deviceQueueCreateInfo,
                .enabledExtensionCount = static_cast<uint32_t>(required_device_extension.size()),
                .ppEnabledExtensionNames = required_device_extension.data()
            };

            device = vk::raii::Device(physical_device, deviceCreateInfo);
            graphics_queue = vk::raii::Queue(device, queueIndex, 0);
        }

        vk::SurfaceFormatKHR choose_swap_surface_format(std::vector<vk::SurfaceFormatKHR> const &availableFormats) {
            const auto formatIt = std::ranges::find_if(availableFormats, [](const auto &format) {
                return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
            });

            return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
        }

        vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes)
        {
            assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) {
                return presentMode == vk::PresentModeKHR::eFifo;
            }));

            return std::ranges::any_of(availablePresentModes, [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
        }

        vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities)
        {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

            int width, height;
            SDL_GetWindowSize(window, &width, &height);

            return {
                std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
            };
        }

       	static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities)
    	{
    		auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    		if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
    		{
    			minImageCount = surfaceCapabilities.maxImageCount;
    		}
    		return minImageCount;
    	}

        void create_swap_chain() {
            vk::SurfaceCapabilitiesKHR surfaceCapabilities = physical_device.getSurfaceCapabilitiesKHR(*surface);
            swapChainExtent = chooseSwapExtent(surfaceCapabilities);
            uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

            std::vector<vk::SurfaceFormatKHR> availableFormats = physical_device.getSurfaceFormatsKHR(*surface);
            swapChainSurfaceFormat = choose_swap_surface_format(availableFormats);

            std::vector<vk::PresentModeKHR> availablePresentModes = physical_device.getSurfacePresentModesKHR(*surface);
    		vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

    		vk::SwapchainCreateInfoKHR swapChainCreateInfo {
                .surface = *surface,
                .minImageCount = minImageCount,
                .imageFormat = swapChainSurfaceFormat.format,
                .imageColorSpace = swapChainSurfaceFormat.colorSpace,
                .imageExtent = swapChainExtent,
                .imageArrayLayers = 1,
                .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
                .imageSharingMode = vk::SharingMode::eExclusive,
                .preTransform = surfaceCapabilities.currentTransform,
                .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                .presentMode = presentMode,
                .clipped = true
            };

    		swap_chain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    		swapChainImages = swap_chain.getImages();
        }

        void create_image_views() {
            assert(swap_chain_image_views.empty());

            vk::ImageViewCreateInfo imageViewCreateInfo {
                .viewType = vk::ImageViewType::e2D,
                .format = swapChainSurfaceFormat.format,
                .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
            };

            imageViewCreateInfo.components = {
                vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity,
            };

            imageViewCreateInfo.subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .levelCount = 1,
                .layerCount = 1
            };

            for (auto &image : swapChainImages) {
                imageViewCreateInfo.image = image;
                swap_chain_image_views.emplace_back(device, imageViewCreateInfo);
            }
        }

        static std::vector<char> readFile(const std::string &filename)
    	{
    		std::ifstream file(filename, std::ios::ate | std::ios::binary);
    		if (!file.is_open())
    		{
                std::cerr << "Current working directory: " << std::filesystem::current_path() << std::endl;
    			throw std::runtime_error("failed to open file!");
    		}
    		std::vector<char> buffer(file.tellg());
    		file.seekg(0, std::ios::beg);
    		file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    		file.close();
    		return buffer;
    	}

        [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const {
            vk::ShaderModuleCreateInfo createInfo {
                .codeSize = code.size() * sizeof(char),
                .pCode = reinterpret_cast<const uint32_t*>(code.data())
            };

            vk::raii::ShaderModule shaderModule {
                device, createInfo
            };

            return shaderModule;
        }

        void create_graphics_pipeline() {
            vk::raii::ShaderModule shaderModule = createShaderModule(readFile("shaders/slang.spv"));

            vk::PipelineShaderStageCreateInfo vertShaderStageInfo {
                .stage = vk::ShaderStageFlagBits::eVertex,
                .module = shaderModule,
                .pName = "vertMain"
            };
            vk::PipelineShaderStageCreateInfo fragShaderStageInfo {
                .stage = vk::ShaderStageFlagBits::eFragment,
                .module = shaderModule,
                .pName = "fragMain"
            };
    		vk::PipelineShaderStageCreateInfo shaderStages[] = {
                vertShaderStageInfo, fragShaderStageInfo
            };

    		vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    		vk::PipelineInputAssemblyStateCreateInfo inputAssembly {
                .topology = vk::PrimitiveTopology::eTriangleList
            };
    		vk::PipelineViewportStateCreateInfo viewportState {
                .viewportCount = 1,
                .scissorCount = 1
            };

    		vk::PipelineRasterizationStateCreateInfo rasterizer {
        		.depthClampEnable = vk::False,
                .rasterizerDiscardEnable = vk::False,
                .polygonMode = vk::PolygonMode::eFill,
                .cullMode = vk::CullModeFlagBits::eBack,
                .frontFace = vk::FrontFace::eClockwise,
                .depthBiasEnable = vk::False,
                .lineWidth = 1.0f
    		};

    		vk::PipelineMultisampleStateCreateInfo multisampling {
    		    .rasterizationSamples = vk::SampleCountFlagBits::e1,
    			.sampleShadingEnable = vk::False
    		};

    		vk::PipelineColorBlendAttachmentState colorBlendAttachment {
    		    .blendEnable = vk::False,
    		    .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    		};

    		vk::PipelineColorBlendStateCreateInfo colorBlending {
    		    .logicOpEnable = vk::False,
    			.logicOp = vk::LogicOp::eCopy,
    			.attachmentCount = 1,
    			.pAttachments = &colorBlendAttachment
    		};

            std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
            vk::PipelineDynamicStateCreateInfo dynamicState {
                .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
                .pDynamicStates = dynamicStates.data(),
            };

           	vk::PipelineLayoutCreateInfo pipelineLayoutInfo {
                .setLayoutCount = 0,
                .pushConstantRangeCount = 0
            };

            pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

            vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo {
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &swapChainSurfaceFormat.format
            };
            vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
                {
                    .stageCount = 2,
                    .pStages = shaderStages,
                    .pVertexInputState = &vertexInputInfo,
                    .pInputAssemblyState = &inputAssembly,
                    .pViewportState = &viewportState,
                    .pRasterizationState = &rasterizer,
                    .pMultisampleState = &multisampling,
                    .pColorBlendState = &colorBlending,
                    .pDynamicState = &dynamicState,
                    .layout = pipelineLayout,
                    .renderPass = nullptr
                },
                {
                    .colorAttachmentCount = 1,
                    .pColorAttachmentFormats = &swapChainSurfaceFormat.format
                }
            };

            graphics_pipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
        }

        void draw_dummy_frame() {
            // 1. Create a semaphore for acquiring the image
            vk::raii::Semaphore image_available_semaphore(device, vk::SemaphoreCreateInfo{});

            // 2. Acquire a swapchain image index
            auto [result, image_index] = swap_chain.acquireNextImage(UINT64_MAX, *image_available_semaphore);

            // 3. Immediately present it (no rendering, no command buffers!)
            vk::PresentInfoKHR present_info {
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &*image_available_semaphore,
                .swapchainCount = 1,
                .pSwapchains = &*swap_chain,
                .pImageIndices = &image_index
            };

            (void)graphics_queue.presentKHR(present_info);
        }

        void init_vulkan() {
            SDL_Log("Initiating Vulkan. Validation Layers Enabled: %b", enable_validation_layers);

            create_instance();
            SDL_Log("created instance");

            setup_debug_messenger();
            SDL_Log("setup debug messenger");

            create_surface();
            SDL_Log("created surface");

            pick_physical_device();
            SDL_Log("picked physical device");

            create_logical_device();
            SDL_Log("created logical device");

            create_swap_chain();
            SDL_Log("created swap chain");

            create_image_views();
            SDL_Log("created image views");

            create_graphics_pipeline();
            SDL_Log("created graphics pipeline");
        }

        void main_loop() {
            SDL_Event event;
            bool is_running = true;

            while(is_running) {
                // SDL_Log("running");

                while (SDL_PollEvent(&event)) {
                    // SDL_Log("checking input");

                    if(event.type == SDL_EVENT_QUIT) {
                        is_running = false;
                    }
                }

                draw_dummy_frame();
            }
        }

        void cleanup() {
            SDL_DestroyWindow(window);
            SDL_Quit();
        }
};

int main() {
    try {
        Triangle program;
        program.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
