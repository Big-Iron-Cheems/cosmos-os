#include "heap.hpp"

#include "offsets.hpp"
#include "physical.hpp"
#include "virtual.hpp"

namespace cosmos::memory::heap {
    struct Region {
        Region* next;

        bool used : 1;
        uint64_t size : 63;
    };

    static Region* head;
    static Region* tail;
    static uint64_t page_count;

    bool grow() {
        const auto phys = phys::alloc_pages(1);
        if (phys == 0) return false;

        const auto space = virt::get_current();
        if (!virt::map_pages(space, virt::HEAP / 4096ul + page_count, phys / 4096ul, 1, false)) return false;

        if (tail == nullptr || tail->used) {
            const auto region = reinterpret_cast<Region*>(virt::HEAP + page_count * 4096ul);
            region->next = nullptr;
            region->used = false;
            region->size = 4096ul - sizeof(Region);

            if (tail == nullptr) {
                head = region;
                tail = region;
            } else {
                tail->next = region;
                tail = region;
            }
        } else {
            tail->size += 4096ul;
        }

        page_count++;
        return true;
    }

    void init() {
        head = nullptr;
        tail = nullptr;
        page_count = 0;

        grow();
    }

    void* alloc_from_node(Region* current, const uint64_t size) {
        if (current->size - size < sizeof(Region) + 8) {
            current->used = true;
        } else {
            const auto free = reinterpret_cast<Region*>(reinterpret_cast<uint64_t>(current + 1) + size);
            free->next = current->next;
            free->used = false;
            free->size = current->size - size - sizeof(Region);

            current->next = free;
            current->used = true;
            current->size = size;

            if (current == tail) {
                tail = free;
            }
        }

        return current + 1;
    }

    void* alloc(const uint64_t size) {
#define CHECK_REGION(region) (!region->used && region->size >= size)

        auto current = head;

        do {
            if (CHECK_REGION(current)) {
                break;
            }

            current = current->next;
        } while (current != nullptr);

        if (current == nullptr) {
            do {
                if (!grow()) return nullptr;
            } while (!CHECK_REGION(tail));

            current = tail;
        }

        return alloc_from_node(current, size);

#undef CHECK_REGION
    }

    void merge_forward(Region* region) {
        region->size += sizeof(Region) + region->next->size;

        if (region->next == tail) {
            tail = region;
        }

        region->next = region->next->next;
    }

    void free(void* ptr) {
        Region* prev = nullptr;
        Region* current = head;

        do {
            if (reinterpret_cast<uint64_t>(ptr) == reinterpret_cast<uint64_t>(current + 1)) {
                break;
            }

            prev = current;
            current = current->next;
        } while (current != nullptr);

        if (current == nullptr || !current->used) {
            return;
        }

        current->used = false;

        if (prev != nullptr && !prev->used) {
            merge_forward(prev);
            if (prev->next != nullptr && !prev->next->used) merge_forward(prev);
        } else if (current->next != nullptr && !current->next->used) {
            merge_forward(current);
        }
    }
} // namespace cosmos::memory::heap
