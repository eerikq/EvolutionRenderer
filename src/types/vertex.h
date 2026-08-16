#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

typedef struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
} Vertex;

typedef struct VertexDescription {
    constexpr static VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    // even as a cpp & stl hater, cpp arrays are kinda awesome because they:
    // 1. have no hidden allocations like std::vectors do (big)
    // 2. can be returned by value (used here)
    // 3. can be copied and assigned to other std::array<T> values with the same type
    // 4. has a lightweight
    // 5. dont suffer from pointer decay (i.e. type and array dimension isn't lost)
    // 6. can be directly handed to C APIs via .data()
    constexpr static std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {
        {{
             .location = 0,
             .binding = 0,
             .format = VK_FORMAT_R32G32_SFLOAT,
             .offset = offsetof(Vertex, pos),
         },
         {
             .location = 1,
             .binding = 0,
             .format = VK_FORMAT_R32G32B32_SFLOAT,
             .offset = offsetof(Vertex, color),
         }}
    };
} VertexDescription;
