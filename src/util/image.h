#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct Image {
    int width = 0;
    int height = 0;
    int channels = 0;

    size_t size = 0;
    uint8_t* data = 0;
} Image;

bool imageLoad(const char* file_path, Image* image);
void imageFree(Image* image);
