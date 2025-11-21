#pragma once

namespace cosmos::shell {
    using CommandFn = void (*)();

    CommandFn get_command_fn(const char* name);
} // namespace cosmos::shell
