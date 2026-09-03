#include <glm/glm.hpp>

typedef struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
} UniformBufferObject;
