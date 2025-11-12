#include "limine.hpp"

using namespace cosmos;

[[noreturn]]
void halt() {
    for (;;) {
        asm volatile ("hlt");
    }
}

extern "C" [[noreturn]] void main() {
    if (!limine::init()) {
        halt();
    }

    const auto pixels = static_cast<uint32_t*>(limine::get_framebuffer().pixels);
    pixels[0] = 0xFFFFFFFF;
    pixels[1] = 0xFFFF0000;
    pixels[2] = 0xFF00FF00;
    pixels[3] = 0xFF0000FF;

    halt();
}
