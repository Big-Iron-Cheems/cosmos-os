#include "physical.hpp"

#include "limine.hpp"
#include "utils.hpp"

namespace cosmos::memory::phys {
    static uint64_t* entries;
    static uint32_t entry_count;

    static uint32_t total_pages;
    static uint32_t used_pages;

    void mark_page(const uint32_t index, const bool used) {
        uint64_t& entry = entries[index / 64u];
        const uint64_t mask = 1ull << (index % 64u);

        if (used) {
            entry |= mask;
        } else {
            entry = entry & ~mask;
        }
    }

    void mark_pages(const uint32_t first, uint32_t count, const bool used) {
        if (first >= total_pages) return;
        count = utils::min(count, total_pages - first);

        for (auto i = 0u; i < count; i++) {
            mark_page(first + i, used);
        }

        if (used) {
            used_pages += count;
        } else {
            used_pages -= count;
        }
    }

    void init() {
        // Calculate total memory size
        total_pages = 0;

        for (auto i = 0u; i < limine::get_memory_range_count(); i++) {
            const auto [type, first_page, page_count] = limine::get_memory_range(i);

            if (limine::memory_type_ram(type) && first_page + page_count > total_pages) {
                total_pages = first_page + page_count;
            }
        }

        entry_count = utils::ceil_div(total_pages, 64u);

        // Find usable range to store entries in
        const uint32_t entries_page_count = utils::ceil_div(entry_count * 8ul, 4096ul);
        uint32_t entries_page_index = 0xFFFFFFFF;

        for (auto i = 0u; i < limine::get_memory_range_count(); i++) {
            const auto [type, first_page, page_count] = limine::get_memory_range(i);

            if (type == limine::MemoryType::Usable && first_page >= 1 && page_count >= entries_page_count) {
                entries = reinterpret_cast<uint64_t*>(limine::get_hhdm() + first_page * 4096);
                entries_page_index = first_page;

                break;
            }
        }

        if (entries_page_index == 0xFFFFFFFF) {
            utils::panic(nullptr, "[memory] Failed to find enough memory to store physical memory bitmask");
        }

        // Mark all pages as used
        utils::memset(entries, 0xFF, entry_count * 8ul);
        used_pages = total_pages;

        // Mark usable ranges as unused
        for (auto i = 0u; i < limine::get_memory_range_count(); i++) {
            const auto [type, first_page, page_count] = limine::get_memory_range(i);

            if (type == limine::MemoryType::Usable) {
                mark_pages(first_page, page_count, false);
            }
        }

        // Mark entries bitmask as used
        mark_pages(entries_page_index, entries_page_count, true);
    }

    uint64_t alloc_pages(const uint32_t count) {
        uint32_t first_empty = 0;
        uint32_t empty_count = 0;

        for (auto i = 0u; i < entry_count; i++) {
            uint64_t entry = entries[i];

            for (auto j = 0; j < 64; j++) {
                if ((entry & 1u) == 0) {
                    if (empty_count == 0) {
                        first_empty = i * 64 + j;
                        empty_count = 1;
                    } else {
                        empty_count++;
                    }
                } else {
                    empty_count = 0;
                }

                if (empty_count >= count) {
                    mark_pages(first_empty, count, true);
                    return static_cast<uint64_t>(first_empty) * 4096ul;
                }

                entry >>= 1;
            }
        }

        return 0;
    }

    void free_pages(const uint32_t first, const uint32_t count) {
        mark_pages(first, count, false);
    }

    uint32_t get_total_pages() {
        return total_pages;
    }

    uint32_t get_used_pages() {
        return used_pages;
    }
} // namespace cosmos::memory::phys
