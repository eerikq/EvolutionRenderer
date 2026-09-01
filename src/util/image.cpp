#include "image.h"

#include "util/log.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace image {
    bool LoadFromFile(const char* file_path, Image* image) {
        image->data = stbi_load(file_path, &image->width, &image->height, &image->channels, 0);
        image->size = image->width * image->height * image->channels;

        if (image->data == NULL) {
            evoLog(PrintSeverity::Error, "Failed to load the image at ./{}", file_path);
            return false;
        }

        evoLog(PrintSeverity::Debug, "Loaded image at ./{} ({}x{} with {} channels)", file_path, image->width, image->height, image->channels);
        return true;
    }

    void Free(Image* image) {
        stbi_image_free(image->data);

        int width = 0;
        int height = 0;
        int channels = 0;
        size_t size = 0;
    }
} // namespace image
