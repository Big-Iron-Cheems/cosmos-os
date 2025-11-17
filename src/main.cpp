#include "interrupts/isr.hpp"
#include "limine.hpp"
#include "memory/heap.hpp"
#include "memory/physical.hpp"
#include "memory/virtual.hpp"
#include "serial.hpp"
#include "utils.hpp"

using namespace cosmos;

extern "C" [[noreturn]] void main() {
    serial::init();

    if (!limine::init()) {
        utils::halt();
    }

    isr::init();
    memory::phys::init();

    const auto space = memory::virt::create();
    memory::virt::switch_to(space);

    memory::heap::init();

    serial::printf("[cosmos] %s\n", "Initialized");

    serial::printf("[cosmos] Total memory: %d mB\n", static_cast<uint64_t>(memory::phys::get_total_pages()) * 4096 / 1024 / 1024);
    serial::printf("[cosmos] Free memory: %d mB\n", static_cast<uint64_t>(memory::phys::get_free_pages()) * 4096 / 1024 / 1024);

    const auto pixels = static_cast<uint32_t*>(limine::get_framebuffer().pixels);
    pixels[0] = 0xFFFFFFFF;
    pixels[1] = 0xFFFF0000;
    pixels[2] = 0xFF00FF00;
    pixels[3] = 0xFF0000FF;

    utils::halt();
}
