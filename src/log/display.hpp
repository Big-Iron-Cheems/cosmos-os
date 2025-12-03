#pragma once

#include "shell/color.hpp"

namespace cosmos::log::display {
    void init(bool delay = false);

    void print(shell::Color color, const char* str);

    void printf(shell::Color color, const char* fmt, ...);
} // namespace cosmos::log::display
