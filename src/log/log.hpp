#pragma once

#include <cstdint>

namespace cosmos::log {
    enum class Type : uint8_t {
        Info,
        Warning,
        Error,
    };

    void enable_display(bool delay = false);
    void disable_display();

    void enable_paging();

    void println(Type type, const char* file, uint32_t line, const char* fmt, ...);

    const uint8_t* get_start();
    uint64_t get_size();
} // namespace cosmos::log

#define INFO(fmt, ...) cosmos::log::println(cosmos::log::Type::Info, __FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define WARN(fmt, ...) cosmos::log::println(cosmos::log::Type::Warning, __FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define ERROR(fmt, ...) cosmos::log::println(cosmos::log::Type::Error, __FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
