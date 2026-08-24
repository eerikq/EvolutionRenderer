#include "file.h"

#include "util/log.h"

#include <filesystem>
#include <fstream>

std::vector<char> evoReadFile(const char* filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) evoLog(PrintSeverity::Error, "Failed to open file! Current working directory: {}", std::filesystem::current_path().string());

    std::vector<char> buffer(file.tellg());

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
}
