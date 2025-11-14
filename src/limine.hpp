#pragma once

#include <cstdint>

namespace cosmos::limine {
    struct Range {
        uint64_t address;
        uint64_t length;
    };

    struct Framebuffer {
        uint32_t width;
        uint32_t height;
        uint32_t pitch;
        void* pixels;
    };

    bool init();

    uint32_t get_usable_memory_range_count();
    const Range& get_usable_memory_range(uint32_t index);

    void* get_hhdm();

    const Framebuffer& get_framebuffer();
} // namespace cosmos::limine
