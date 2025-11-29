#pragma once

#include "color.hpp"

#include <cstdarg>

namespace cosmos::display {
    void init(bool delay = false);

    void printf(shell::Color color, const char* fmt, va_list args);

    inline void printf(const shell::Color color, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        printf(color, fmt, args);
        va_end(args);
    }

    inline void printf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        printf(shell::WHITE, fmt, args);
        va_end(args);
    }
} // namespace cosmos::display
