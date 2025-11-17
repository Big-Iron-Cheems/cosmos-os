#pragma once

#include <cstdint>

namespace cosmos::memory::heap {
    void init();

    void* alloc(uint64_t size);
    void free(void* ptr);

    template <typename T>
    T* alloc() {
        return static_cast<T*>(alloc(sizeof(T)));
    }

    template <typename T>
    T* alloc_array(const uint32_t count) {
        return static_cast<T*>(alloc(sizeof(T) * count));
    }
} // namespace cosmos::memory::heap
