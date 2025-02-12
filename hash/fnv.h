/* Implementation Fowler–Noll–Vo (FNV) hash algorithm in C */
#ifndef FNV_H
#define FNV_H

#include <stdint.h>
#include <stdio.h>

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

// for correct work need open file with mode "rb"

FNV_DEF uint32_t fnv1_32_file (FILE* file);
FNV_DEF uint32_t fnv1a_32_file(FILE* file);
FNV_DEF uint64_t fnv1_64_file (FILE* file);
FNV_DEF uint64_t fnv1a_64_file(FILE* file);

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

uint32_t fnv1_32_file(FILE* file) {
    int ch = 0;
    uint32_t out = 0x811c9dc5;
    while ((ch = fgetc(file)) != EOF) {
        out *= 0x1000193;
        out ^= (uint8_t)ch;
    }
    return out;
}

uint32_t fnv1a_32_file(FILE* file) {
    int ch = 0;
    uint32_t out = 0x811c9dc5;
    while ((ch = fgetc(file)) != EOF) {
        out ^= (uint8_t)ch;
        out *= 0x1000193;
    }
    return out;
}

uint64_t fnv1_64_file(FILE* file) {
    int ch = 0;
    uint64_t out = 0xcbf29ce484222325;
    while ((ch = fgetc(file)) != EOF) {
        out *= 0x100000001b3;
        out ^= (uint8_t)ch;
    }
    return out;
}

uint64_t fnv1a_64_file(FILE* file) {
    int ch = 0;
    uint64_t out = 0xcbf29ce484222325;
    while ((ch = fgetc(file)) != EOF) {
        out ^= (uint8_t)ch;
        out *= 0x100000001b3;
    }
    return out;
}

#endif // FNV_IMPLEMENTATION