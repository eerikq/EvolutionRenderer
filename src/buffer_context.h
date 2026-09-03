#pragma once

#include "renderer/resources/buffer.h"
#include "renderer/resources/texture.h"
#include "types/vertex.h"

#include <vector>
#include <volk.h>

typedef struct BufferContext {
    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

    Buffer vertex_buffer {};
    Buffer index_buffer {};

    std::vector<Buffer> uniform_buffers {};
    std::vector<void*> uniform_buffers_mapped {};

    Image image {};
    Texture texture {};
    VkSampler sampler = nullptr;
} BufferContext;
