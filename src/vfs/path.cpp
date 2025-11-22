#include "path.hpp"

namespace cosmos::vfs {
    uint32_t check_abs_path(const char* path) {
        if (*path != '/') return 0;
        auto length = 1u;

        while (path[length] != '\0') {
            if (path[length] == '/' && path[length - 1] == '/') {
                return 0;
            }

            if (path[length] == ' ' && (path[length - 1] == '/' || path[length + 1] == '/' || path[length + 1] == '\0')) {
                return 0;
            }

            length++;
        }

        if (length > 1 && path[length - 1] == '/') {
            length--;
        }

        return length;
    }
} // namespace cosmos::vfs
