#pragma once

#include <fmt/chrono.h>
#include <fmt/color.h>

enum class PrintSeverity { Debug, Info, Warn, Error, Fatal };

// Example Log:
// [03:18:23][ERROR] This is an error message.
template <typename... T>
void evoLog(PrintSeverity severity, const char* message, T&&... args) {
    fmt::color severity_color = fmt::color::mint_cream;
    const char* severity_text = "NONE!";

    switch (severity) {
        case PrintSeverity::Debug:
            severity_color = fmt::color::medium_aquamarine;
            severity_text = "Debug";
            break;
        case PrintSeverity::Info:
            severity_color = fmt::color::mint_cream;
            severity_text = "Info.";
            break;
        case PrintSeverity::Warn:
            severity_color = fmt::color::gold;
            severity_text = "Warn.";
            break;
        case PrintSeverity::Error:
            severity_color = fmt::color::red;
            severity_text = "Error";
            break;
        case PrintSeverity::Fatal:
            severity_color = fmt::color::maroon;
            severity_text = "Fatal";
            break;
    }

    fmt::memory_buffer buffer;
    auto now = std::chrono::system_clock::now();
    auto clean_now = std::chrono::floor<std::chrono::seconds>(now);

    fmt::format_to(std::back_inserter(buffer), fg(fmt::color::dim_gray), "[{:%H:%M:%S}]", clean_now);
    fmt::format_to(std::back_inserter(buffer), fg(severity_color), "[{}]: ", severity_text);
    fmt::format_to(std::back_inserter(buffer), fg(severity_color), fmt::runtime(message), std::forward<T>(args)...);
    fmt::format_to(std::back_inserter(buffer), "\n");

    fmt::print("{}", fmt::string_view(buffer.data(), buffer.size()));
}
