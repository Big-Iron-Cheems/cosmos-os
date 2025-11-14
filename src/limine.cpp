#include "limine.hpp"
#include "serial.hpp"
#include <limine.h>

__attribute__((unused, section(".requests_start"))) //
static volatile uint64_t start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((unused, section(".requests"))) //
static volatile uint64_t base_revision[] = LIMINE_BASE_REVISION(4);

__attribute__((unused, section(".requests"))) //
static volatile limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
};

__attribute__((unused, section(".requests"))) //
static volatile limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
};

__attribute__((unused, section(".requests"))) //
static volatile limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
};

__attribute__((unused, section(".requests_end"))) //
static volatile uint64_t requests_end[] = LIMINE_REQUESTS_END_MARKER;

namespace cosmos::limine {
    static Range usable_memory_ranges[16];
    static uint32_t usable_memory_range_count;

    static Framebuffer fb;

    void init_memory_ranges() {
        usable_memory_range_count = 0;

        for (auto i = 0u; i < memmap_request.response->entry_count; i++) {
            const auto& entry = memmap_request.response->entries[i];

            if (entry->type == LIMINE_MEMMAP_USABLE && usable_memory_range_count < 16) {
                usable_memory_ranges[usable_memory_range_count++] = {
                    .address = entry->base,
                    .length = entry->length,
                };
            }
        }
    }

    void init_framebuffer() {
        const auto limine_fb = framebuffer_request.response->framebuffers[0];

        fb = {
            .width = static_cast<uint32_t>(limine_fb->width),
            .height = static_cast<uint32_t>(limine_fb->height),
            .pitch = static_cast<uint32_t>(limine_fb->pitch / 4),
            .pixels = limine_fb->address,
        };
    }

    bool init() {
        if (!LIMINE_BASE_REVISION_SUPPORTED(base_revision)) {
            serial::print("[limine] Base revision not supported\n");
            return false;
        }

        if (memmap_request.response == nullptr) {
            serial::print("[limine] Memory ranges missing\n");
            return false;
        }

        if (hhdm_request.response == nullptr) {
            serial::print("[limine] HHDM missing\n");
            return false;
        }

        if (framebuffer_request.response == nullptr || framebuffer_request.response->framebuffer_count < 1) {
            serial::print("[limine] Framebuffer missing\n");
            return false;
        }

        init_memory_ranges();
        init_framebuffer();

        serial::print("[limine] Initialized\n");
        return true;
    }

    uint32_t get_usable_memory_range_count() {
        return usable_memory_range_count;
    }

    const Range& get_usable_memory_range(const uint32_t index) {
        return usable_memory_ranges[index];
    }

    void* get_hhdm() {
        return reinterpret_cast<void*>(hhdm_request.response->offset);
    }

    const Framebuffer& get_framebuffer() {
        return fb;
    }
} // namespace cosmos::limine
