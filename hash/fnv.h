/* Implementation Fowler–Noll–Vo (FNV) hash algorithm in C */
#ifndef FNV_H
#define FNV_H

#include <stdint.h>

#ifndef FNV_DEF
#define FNV_DEF
#endif

#ifdef __cplusplus
extern "C" {
#endif

FNV_DEF uint32_t fnv1_32 (const void* source, size_t count);
FNV_DEF uint32_t fnv1a_32(const void* source, size_t count);
FNV_DEF uint64_t fnv1_64 (const void* source, size_t count);
FNV_DEF uint64_t fnv1a_64(const void* source, size_t count);

#ifdef __cplusplus
}
#endif

#endif // FNV_H

#ifdef FNV_IMPLEMENTATION

uint32_t fnv1_32(const void* source, size_t count) {
    const uint8_t* data = (const uint8_t*)source;
    uint32_t out = 0x811c9dc5;
    while (count --> 0) {
        out *= 0x1000193;
        out ^= *data++;
    }
    return out;
}

uint32_t fnv1a_32(const void* source, size_t count) {
    const uint8_t* data = (const uint8_t*)source;
    uint32_t out = 0x811c9dc5;
    while (count --> 0) {
        out ^= *data++;
        out *= 0x1000193;
    }
    return out;
}

uint64_t fnv1_64(const void* source, size_t count) {
    const uint8_t* data = (const uint8_t*)source;
    uint64_t out = 0xcbf29ce484222325;
    while (count --> 0) {
        out *= 0x100000001b3;
        out ^= *data++;
    }
    return out;
}

uint64_t fnv1a_64(const void* source, size_t count) {
    const uint8_t* data = (const uint8_t*)source;
    uint64_t out = 0xcbf29ce484222325;
    while (count --> 0) {
        out ^= *data++;
        out *= 0x100000001b3;
    }
    return out;
}

#endif // FNV_IMPLEMENTATION