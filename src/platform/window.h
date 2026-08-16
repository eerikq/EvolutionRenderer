#include <volk.h>

bool windowCreate(int window_width, int window_height);
void windowDestroy();

bool windowCreateSurface(VkInstance instance, VkSurfaceKHR* surface);
void windowDestroySurface(VkInstance instance, VkSurfaceKHR* surface);

const char* const* windowGetInstanceExtensions(uint32_t* instance_extensions_count);
bool windowGetSize(int* width, int* height);
