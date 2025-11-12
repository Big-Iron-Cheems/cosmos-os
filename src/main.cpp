[[noreturn]]
void halt() {
    for (;;) {
        asm volatile ("hlt");
    }
}

extern "C" [[noreturn]] void main() {
    halt();
}
