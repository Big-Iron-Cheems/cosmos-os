#pragma once

#include "types.hpp"

namespace cosmos::vfs::devfs {
    void create(Node* node);

    void register_device(Node* node, stl::StringView name, const FileOps* ops, void* handle);
} // namespace cosmos::vfs::devfs
