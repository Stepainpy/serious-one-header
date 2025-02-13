#ifndef PJW_H
#define PJW_H

#include <stdint.h>
#include <stdio.h>

#ifndef PJW_DEF
#define PJW_DEF
#endif

#ifdef __cplusplus
extern "C" {
#endif

PJW_DEF uint32_t pjw_32(const void* source, size_t count);
PJW_DEF uint64_t pjw_64(const void* source, size_t count);

PJW_DEF uint32_t pjw_32_file(FILE* file);
PJW_DEF uint64_t pjw_64_file(FILE* file);

#ifdef __cplusplus
}
#endif

#endif // PJW_H

#ifdef PJW_IMPLEMENTATION

uint32_t pjw_32(const void* source, size_t count) {
    const uint8_t* data = (const uint8_t*)source;
    uint32_t out = 0, high;
    while (count --> 0) {
        out = (out << 4) + *data++;
        if ((high = 0xF0000000 & out) != 0) {
            out ^= high >> 24;
            out &= ~high;
        }
    }
    return out;
}

uint64_t pjw_64(const void* source, size_t count) {
    const uint8_t* data = (const uint8_t*)source;
    uint64_t out = 0, high;
    while (count --> 0) {
        out = (out << 8) + *data++;
        if ((high = 0xFF00000000000000 & out) != 0) {
            out ^= high >> 48;
            out &= ~high;
        }
    }
    return out;
}

uint32_t pjw_32_file(FILE* file) {
    int ch = 0;
    uint32_t out = 0, high;
    while ((ch = fgetc(file)) != EOF) {
        out = (out << 4) + (uint8_t)ch;
        if ((high = 0xF0000000 & out) != 0) {
            out ^= high >> 24;
            out &= ~high;
        }
    }
    return out;
}

uint64_t pjw_64_file(FILE* file) {
    int ch = 0;
    uint64_t out = 0, high;
    while ((ch = fgetc(file)) != EOF) {
        out = (out << 8) + (uint8_t)ch;
        if ((high = 0xFF00000000000000 & out) != 0) {
            out ^= high >> 48;
            out &= ~high;
        }
    }
    return out;
}

#endif // PJW_IMPLEMENTATION