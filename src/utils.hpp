#pragma once

#include <concepts>
#include <cstdint>

namespace cosmos::utils {
    [[noreturn]]
    void halt();

    void memset(void* dst, uint8_t value, uint64_t size);
    void memcpy(void* dst, const void* src, uint64_t size);

    template <std::integral T>
    T ceil_div(T a, T b) {
        return (a + b - 1) / b;
    }

    template <typename T>
    T align(T value, T alignment) {
        return (value + (alignment - 1)) & ~(alignment - 1);
    }

    /// Available port widths for port I/O operations (8, 16, or 32 bits)
    template <typename T>
    concept port_width = std::same_as<T, uint8_t> || std::same_as<T, uint16_t> || std::same_as<T, uint32_t>;

    /// Port read
    template <port_width T>
    T port_in(uint16_t port) {
        T value{};
        if constexpr (std::same_as<T, uint8_t>) { // NOLINT(*-branch-clone)
            __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
        } else if constexpr (std::same_as<T, uint16_t>) {
            __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
        } else { // uint32_t
            __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
        }
        return value;
    }

    /// Port write
    template <port_width T>
    void port_out(uint16_t port, T value) {
        if constexpr (std::same_as<T, uint8_t>) { // NOLINT(*-branch-clone)
            __asm__ volatile("outb %0, %1" ::"a"(value), "Nd"(port));
        } else if constexpr (std::same_as<T, uint16_t>) {
            __asm__ volatile("outw %0, %1" ::"a"(value), "Nd"(port));
        } else { // uint32_t
            __asm__ volatile("outl %0, %1" ::"a"(value), "Nd"(port));
        }
    }

    // Byte

    inline uint8_t byte_in(const uint16_t port) {
        return port_in<uint8_t>(port);
    }

    inline void byte_out(const uint16_t port, const uint8_t data) {
        port_out<uint8_t>(port, data);
    }

    // Short

    inline uint16_t short_in(const uint16_t port) {
        return port_in<uint16_t>(port);
    }

    inline void short_out(const uint16_t port, const uint16_t data) {
        port_out<uint16_t>(port, data);
    }

    // Int

    inline uint32_t int_in(const uint16_t port) {
        return port_in<uint32_t>(port);
    }

    inline void int_out(const uint16_t port, const uint32_t data) {
        port_out<uint32_t>(port, data);
    }

    // Other

    inline void wait() {
        port_out<uint8_t>(0x80, 0);
    }
} // namespace cosmos::utils
