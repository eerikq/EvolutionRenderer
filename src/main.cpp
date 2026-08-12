#include <stdexcept>
#include <utility>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <filesystem>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <glm/glm.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 900
#define MAX_FRAMES_IN_FLIGHT 2

#if NDEBUG
bool enable_validation_layers = false;
#else
bool enable_validation_layers = true;
#endif

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = vk::VertexInputRate::eVertex
        };
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
        return {{
            {
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = offsetof(Vertex, pos)
            },
            {
                .location = 1,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = offsetof(Vertex, color)
            }
        }};
    }
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

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

        // vulkan instances
        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;
        vk::raii::SurfaceKHR surface = nullptr;
        vk::raii::DebugUtilsMessengerEXT debug_messenger = nullptr;

        // surface & devices
        vk::raii::PhysicalDevice physical_device = nullptr;
        vk::raii::Device device = nullptr;
        vk::raii::Queue graphics_queue = nullptr;

        // swap chain
       	vk::raii::SwapchainKHR swap_chain = nullptr;

        std::vector<vk::Image> swap_chain_images;
    	vk::SurfaceFormatKHR swap_chain_surface_format;
    	vk::Extent2D swap_chain_extent;
        std::vector<vk::raii::ImageView> swap_chain_image_views;

        vk::raii::PipelineLayout pipeline_layout = nullptr;
        vk::raii::Pipeline graphics_pipeline = nullptr;
        vk::raii::CommandPool command_pool = nullptr;
        std::vector<vk::raii::CommandBuffer> command_buffers;

        std::vector<vk::raii::Semaphore> present_complete_semaphores;
        std::vector<vk::raii::Semaphore> render_finished_semaphores;
        std::vector<vk::raii::Fence> in_flight_fences;

        // buffers
        vk::raii::Buffer vertex_buffer = nullptr;
        vk::raii::DeviceMemory vertex_buffer_memory = nullptr;
        vk::raii::Buffer index_buffer = nullptr;
        vk::raii::DeviceMemory index_buffer_memory = nullptr;

        uint32_t queueIndex = ~0;
        uint32_t frame_index = 0;

        bool framebuffer_resized = false;
        bool is_running = true;

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
                {.synchronization2 = true, .dynamicRendering = true }, // dynamic rendering from Vulkan 1.3
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
            SDL_GetWindowSizeInPixels(window, &width, &height);

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
            swap_chain_extent = chooseSwapExtent(surfaceCapabilities);
            uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

            std::vector<vk::SurfaceFormatKHR> availableFormats = physical_device.getSurfaceFormatsKHR(*surface);
            swap_chain_surface_format = choose_swap_surface_format(availableFormats);

            std::vector<vk::PresentModeKHR> availablePresentModes = physical_device.getSurfacePresentModesKHR(*surface);
    		vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

    		vk::SwapchainCreateInfoKHR swapChainCreateInfo {
                .surface = *surface,
                .minImageCount = minImageCount,
                .imageFormat = swap_chain_surface_format.format,
                .imageColorSpace = swap_chain_surface_format.colorSpace,
                .imageExtent = swap_chain_extent,
                .imageArrayLayers = 1,
                .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
                .imageSharingMode = vk::SharingMode::eExclusive,
                .preTransform = surfaceCapabilities.currentTransform,
                .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                .presentMode = presentMode,
                .clipped = true
            };

    		swap_chain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    		swap_chain_images = swap_chain.getImages();
        }

        void create_image_views() {
            assert(swap_chain_image_views.empty());

            vk::ImageViewCreateInfo imageViewCreateInfo {
                .viewType = vk::ImageViewType::e2D,
                .format = swap_chain_surface_format.format,
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

            for (auto &image : swap_chain_images) {
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
    		vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();
            vk::PipelineVertexInputStateCreateInfo vertexInputInfo {
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = &bindingDescription,
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
                .pVertexAttributeDescriptions = attributeDescriptions.data()
            };

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

            pipeline_layout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

            vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo {
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &swap_chain_surface_format.format
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
                    .layout = pipeline_layout,
                    .renderPass = nullptr
                },
                {
                    .colorAttachmentCount = 1,
                    .pColorAttachmentFormats = &swap_chain_surface_format.format
                }
            };

            graphics_pipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
        }

        void create_command_pool() {
            vk::CommandPoolCreateInfo poolInfo {
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = queueIndex
            };

            command_pool = vk::raii::CommandPool(device, poolInfo);
        }

        void create_command_buffers() {
            vk::CommandBufferAllocateInfo allocInfo {
                .commandPool = command_pool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = MAX_FRAMES_IN_FLIGHT
            };

            command_buffers = vk::raii::CommandBuffers(device, allocInfo);
        }

        void record_command_buffer(uint32_t imageIndex) {
            auto &command_buffer = command_buffers[frame_index];
            command_buffer.begin({});

            // Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
            transition_image_layout(
                imageIndex,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eColorAttachmentOptimal,
                {},
                vk::AccessFlagBits2::eColorAttachmentWrite,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput
            );

            vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
            vk::RenderingAttachmentInfo attachmentInfo = {
                .imageView = swap_chain_image_views[imageIndex],
                .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .loadOp = vk::AttachmentLoadOp::eClear,
                .storeOp = vk::AttachmentStoreOp::eStore,
                .clearValue = clearColor
            };

            vk::RenderingInfo renderingInfo = {
                .renderArea = {
                    .offset = {0, 0},
                    .extent = swap_chain_extent
                },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &attachmentInfo
            };

            command_buffer.beginRendering(renderingInfo);
            command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphics_pipeline);

            command_buffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swap_chain_extent.width), static_cast<float>(swap_chain_extent.height), 0.0f, 1.0f));
            command_buffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swap_chain_extent));

            command_buffer.bindVertexBuffers(0, *vertex_buffer, {0});
            command_buffer.bindIndexBuffer(*index_buffer, 0, vk::IndexTypeValue<decltype(indices)::value_type>::value);
            command_buffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
            command_buffer.endRendering();

            transition_image_layout(
                imageIndex,
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::ImageLayout::ePresentSrcKHR,
                vk::AccessFlagBits2::eColorAttachmentWrite,
                {},
                vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                vk::PipelineStageFlagBits2::eBottomOfPipe
            );

            command_buffer.end();
        }

        void transition_image_layout(
    	    uint32_t imageIndex,
    	    vk::ImageLayout old_layout,
    	    vk::ImageLayout new_layout,
    	    vk::AccessFlags2 src_access_mask,
    	    vk::AccessFlags2 dst_access_mask,
    	    vk::PipelineStageFlags2 src_stage_mask,
    	    vk::PipelineStageFlags2 dst_stage_mask
        ){
    		vk::ImageMemoryBarrier2 barrier = {
    		    .srcStageMask = src_stage_mask,
    		    .srcAccessMask = src_access_mask,
    		    .dstStageMask = dst_stage_mask,
    		    .dstAccessMask = dst_access_mask,
    		    .oldLayout = old_layout,
    		    .newLayout = new_layout,
    		    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    		    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    		    .image = swap_chain_images[imageIndex],
    		    .subresourceRange = {
    		           .aspectMask = vk::ImageAspectFlagBits::eColor,
    		           .baseMipLevel = 0,
    		           .levelCount = 1,
    		           .baseArrayLayer = 0,
    		           .layerCount = 1
                }
            };
    		vk::DependencyInfo dependency_info = {
    		    .dependencyFlags = {},
    		    .imageMemoryBarrierCount = 1,
    		    .pImageMemoryBarriers = &barrier
            };

            command_buffers[frame_index].pipelineBarrier2(dependency_info);
        }

        void create_sync_objects() {
    		assert(present_complete_semaphores.empty() && render_finished_semaphores.empty() && in_flight_fences.empty());

    		for (size_t i = 0; i < swap_chain_images.size(); i++) {
    			render_finished_semaphores.emplace_back(device, vk::SemaphoreCreateInfo());
    		}

    		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    			present_complete_semaphores.emplace_back(device, vk::SemaphoreCreateInfo());
    			in_flight_fences.emplace_back(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
    		}
        }

        void cleanup_swap_chain() {
            swap_chain_image_views.clear();
            swap_chain = nullptr;
        }

        void recreate_swap_chain() {
            device.waitIdle();

            cleanup_swap_chain();
            create_swap_chain();
            create_image_views();
        }

        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
            vk::PhysicalDeviceMemoryProperties mem_properties = physical_device.getMemoryProperties();

            for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }

            throw std::runtime_error("failed to find suitable memory type!");
        }

        void create_vertex_buffer() {
            vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

            auto [stagingBuffer, stagingBufferMemory] = create_buffer(
                bufferSize,
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
            );

            void *data = stagingBufferMemory.mapMemory(0, bufferSize);
            memcpy(data, vertices.data(), bufferSize);
            stagingBufferMemory.unmapMemory();

            std::tie(vertex_buffer, vertex_buffer_memory) = create_buffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
            copy_buffer(stagingBuffer, vertex_buffer, bufferSize);
        }

        void create_index_buffer() {
           	vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    		auto [stagingBuffer, stagingBufferMemory] = create_buffer(
                bufferSize,
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
            );

    		void *data = stagingBufferMemory.mapMemory(0, bufferSize);
    		memcpy(data, indices.data(), (size_t) bufferSize);
    		stagingBufferMemory.unmapMemory();

    		std::tie(index_buffer, index_buffer_memory) = create_buffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
    		copy_buffer(stagingBuffer, index_buffer, bufferSize);
        }

        void copy_buffer(vk::raii::Buffer & srcBuffer, vk::raii::Buffer & dstBuffer, vk::DeviceSize size) {
            vk::CommandBufferAllocateInfo allocInfo{
                .commandPool = command_pool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1
            };

            vk::raii::CommandBuffer commandCopyBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());

            commandCopyBuffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
            commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));
            commandCopyBuffer.end();

            graphics_queue.submit(vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer}, nullptr);
            graphics_queue.waitIdle();
        }

        std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
            vk::BufferCreateInfo bufferInfo {
                .size = size,
                .usage = usage,
                .sharingMode = vk::SharingMode::eExclusive
            };
            vk::raii::Buffer buffer = vk::raii::Buffer(device, bufferInfo);

            vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
            vk::MemoryAllocateInfo allocInfo {
                .allocationSize = memRequirements.size,
                .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
            };

            vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
            buffer.bindMemory(*bufferMemory, 0);

            return {
                std::move(buffer),
                std::move(bufferMemory)
            };
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
            SDL_Log("picked physical device (%s)", physical_device.getProperties().deviceName.data());

            create_logical_device();
            SDL_Log("created logical device");

            create_swap_chain();
            SDL_Log("created swap chain");

            create_image_views();
            SDL_Log("created image views");

            create_graphics_pipeline();
            SDL_Log("created graphics pipeline");

            create_command_pool();
            SDL_Log("created command pool");

            create_vertex_buffer();
            SDL_Log("created vertex buffer");

            create_index_buffer();
            SDL_Log("created index buffer");

            create_command_buffers();
            SDL_Log("created command buffer");

            create_sync_objects();
            SDL_Log("created sync objects");
        }

        void draw_frame() {
           	auto fenceResult = device.waitForFences(*in_flight_fences[frame_index], vk::True, UINT64_MAX);
      		if (fenceResult != vk::Result::eSuccess) throw std::runtime_error("failed to wait for fence!");

            auto [result, imageIndex] = swap_chain.acquireNextImage(UINT64_MAX, *present_complete_semaphores[frame_index], nullptr);

            if (result == vk::Result::eErrorOutOfDateKHR || framebuffer_resized) {
                framebuffer_resized = false;
                recreate_swap_chain();
                return;
            }

            if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
                assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
                throw std::runtime_error("failed to acquire swap chain image!");
            }

            device.resetFences(*in_flight_fences[frame_index]);

            command_buffers[frame_index].reset();
            record_command_buffer(imageIndex);

            graphics_queue.waitIdle();

            vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
            const vk::SubmitInfo submitInfo{
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &*present_complete_semaphores[frame_index],
                .pWaitDstStageMask = &waitDestinationStageMask,
                .commandBufferCount = 1,
                .pCommandBuffers = &*command_buffers[frame_index],
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &*render_finished_semaphores[frame_index]
            };

           	graphics_queue.submit(submitInfo, *in_flight_fences[frame_index]);

    		const vk::PresentInfoKHR presentInfoKHR {
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &*render_finished_semaphores[frame_index],
                .swapchainCount = 1,
                .pSwapchains = &*swap_chain,
                .pImageIndices = &imageIndex
            };

            result = graphics_queue.presentKHR(presentInfoKHR);
    		switch (result) {
    			case vk::Result::eSuccess:
    				break;
    			case vk::Result::eSuboptimalKHR:
    				std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
    				break;
    			default:
    				break; // an unexpected result is returned!
    		}

            frame_index = (frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        void main_loop() {
            SDL_Event event;

            while(is_running) {
                draw_frame();

                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_EVENT_QUIT) {
                        is_running = false;
                    }

                    if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                        framebuffer_resized = true;
                    }
                }
            }

            device.waitIdle();
        }

        void cleanup() {
            cleanup_swap_chain();

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
