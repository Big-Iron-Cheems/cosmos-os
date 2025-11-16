#include "limine.hpp"
#include "serial.hpp"
#include "utils.hpp"

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
static volatile limine_executable_address_request executable_address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
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
    static Framebuffer fb;

    void init_framebuffer() {
        const auto limine_fb = framebuffer_request.response->framebuffers[0];

        auto kernel_size = 0ul;

        for (auto i = 0u; i < get_memory_range_count(); i++) {
            const auto [type, address, length] = get_memory_range(i);

            if (type == MemoryType::ExecutableAndModules) {
                kernel_size = length;

                break;
            }
        }

        fb = {
            .width = static_cast<uint32_t>(limine_fb->width),
            .height = static_cast<uint32_t>(limine_fb->height),
            .pitch = static_cast<uint32_t>(limine_fb->pitch / 4),
            .pixels = reinterpret_cast<void*>(utils::align(get_kernel_virt() + kernel_size, 4096ul * 512ul)),
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

        if (executable_address_request.response == nullptr) {
            serial::print("[limine] Executable address missing\n");
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

        init_framebuffer();

        serial::print("[limine] Initialized\n");
        return true;
    }

    uint32_t get_memory_range_count() {
        return memmap_request.response->entry_count;
    }

    MemoryRange get_memory_range(const uint32_t index) {
        const auto entry = memmap_request.response->entries[index];

        auto range = MemoryRange{
            .address = entry->base,
            .size = entry->length,
        };

        switch (entry->type) {
        case LIMINE_MEMMAP_USABLE:
            range.type = MemoryType::Usable;
            break;
        case LIMINE_MEMMAP_RESERVED:
            range.type = MemoryType::Reserved;
            break;
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            range.type = MemoryType::AcpiReclaimable;
            break;
        case LIMINE_MEMMAP_ACPI_NVS:
            range.type = MemoryType::AcpiNvs;
            break;
        case LIMINE_MEMMAP_BAD_MEMORY:
            range.type = MemoryType::BadMemory;
            break;
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            range.type = MemoryType::BootloaderReclaimable;
            break;
        case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
            range.type = MemoryType::ExecutableAndModules;
            break;
        case LIMINE_MEMMAP_FRAMEBUFFER:
            range.type = MemoryType::Framebuffer;
            break;
        case LIMINE_MEMMAP_ACPI_TABLES:
            range.type = MemoryType::AcpiTables;
            break;
        default:
            range.type = MemoryType::Reserved;
            break;
        }

        return range;
    }

    uint64_t get_memory_size() {
        const auto entry = memmap_request.response->entries[memmap_request.response->entry_count - 1];
        return entry->base + entry->length;
    }

    uint64_t get_kernel_phys() {
        return executable_address_request.response->physical_base;
    }

    uint64_t get_kernel_virt() {
        return executable_address_request.response->virtual_base;
    }

    uint64_t get_hhdm() {
        return hhdm_request.response->offset;
    }

    const Framebuffer& get_framebuffer() {
        return fb;
    }
} // namespace cosmos::limine
