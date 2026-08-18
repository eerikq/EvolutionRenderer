#pragma once

// #include <chrono>
// #include <iterator>
#include <fmt/chrono.h>
#include <fmt/color.h>

enum class PrintSeverity { Debug, Info, Warn, Error, Fatal };

inline constexpr const char* getSeverityString(PrintSeverity severity) {
    switch (severity) {
        case PrintSeverity::Debug: return "DEBUG";
        case PrintSeverity::Info: return "INFO.";
        case PrintSeverity::Warn: return "WARN.";
        case PrintSeverity::Error: return "ERROR";
        case PrintSeverity::Fatal: return "FATAL";
    }

    return "unknown";
}

inline constexpr fmt::color getSeverityColor(PrintSeverity severity) {
    switch (severity) {
        case PrintSeverity::Debug: return fmt::color::medium_aquamarine;
        case PrintSeverity::Info: return fmt::color::mint_cream;
        case PrintSeverity::Warn: return fmt::color::gold;
        case PrintSeverity::Error: return fmt::color::red;
        case PrintSeverity::Fatal: return fmt::color::maroon;
    }

    return fmt::color::mint_cream;
}

// Example Log:
// [03:18:23][ERROR] This is an error message.
template <typename... T>
void evoLog(PrintSeverity severity, const char* message, T&&... args) {
    fmt::memory_buffer buffer;

    fmt::color severity_color = getSeverityColor(severity);
    auto now = std::chrono::system_clock::now();
    auto clean_now = std::chrono::floor<std::chrono::seconds>(now);

    // time
    fmt::format_to(std::back_inserter(buffer), fg(fmt::color::dim_gray), "[{:%H:%M:%S}]", clean_now);
    // severity
    fmt::format_to(std::back_inserter(buffer), fg(severity_color), "[{}]: ", getSeverityString(severity));
    // message
    fmt::format_to(std::back_inserter(buffer), fg(severity_color), fmt::runtime(message), std::forward<T>(args)...);
    fmt::format_to(std::back_inserter(buffer), "\n");

    fmt::print("{}", fmt::string_view(buffer.data(), buffer.size()));
}
