#pragma once

typedef struct EngineState {
    int frame_index = 0;

    bool framebuffer_resized = false;
    bool is_running = false;
} EngineState;
