#pragma once

#include <cstdint>

namespace cosmos::vfs {
    enum class SeekType : uint8_t {
        Start,
        Current,
        End,
    };

    struct FileOps {
        int64_t (*seek)(void* handle, SeekType type, int64_t offset);
        int64_t (*read)(void* handle, void* buffer, uint64_t length);
        int64_t (*write)(void* handle, const void* buffer, uint64_t length);
        void (*close)(void* handle);
    };

    struct File {
        void* handle;
        FileOps* ops;
    };

    enum class Mode : uint8_t {
        Read,
        Write,
        ReadWrite,
    };

    struct FsOps {
        File* (*open)(void* handle, const char* path, Mode mode);
    };

    struct Fs {
        void* handle;
        FsOps* ops;
    };

    Fs* mount(const char* path);

    File* open(const char* path, Mode mode);

    void close(File* file);
} // namespace cosmos::vfs
