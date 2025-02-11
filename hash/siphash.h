/* Implemenation siphash algorithm in C */
#ifndef SIPHASH_H
#define SIPHASH_H

#include <stdint.h>

#ifndef SIPHASH_DEF
#define SIPHASH_DEF
#endif

#ifdef __cplusplus
extern "C" {    
#endif

typedef struct {
    uint64_t low;
    uint64_t high;
} siphash_key_t;

SIPHASH_DEF uint64_t siphash_base(
    size_t c, size_t d, siphash_key_t key,
    const void* source, size_t count
);

SIPHASH_DEF uint64_t siphash_2_4(
    siphash_key_t key,
    const void* source, size_t count
);

#ifdef __cplusplus
}
#endif

#endif // SIPHASH_H

#ifdef SIPHASH_IMPLEMENTATION

#include <stdbool.h>

void siphash_d_rotl(uint64_t* n, uint8_t shift) {
    *n = (*n << shift) | (*n >> (64 - shift));
}

void sipround(
    uint64_t* v0, uint64_t* v1,
    uint64_t* v2, uint64_t* v3
) {
    *v0 += *v1; *v2 += *v3;
    siphash_d_rotl(v1, 13);
    siphash_d_rotl(v3, 16);
    *v1 ^= *v0; *v3 ^= *v2;
    siphash_d_rotl(v0, 32);

    *v2 += *v1; *v0 += *v3;
    siphash_d_rotl(v1, 17);
    siphash_d_rotl(v3, 21);
    *v1 ^= *v2; *v3 ^= *v0;
    siphash_d_rotl(v2, 32);
}

uint64_t siphash_2_4(siphash_key_t key, const void* src, size_t cnt) {
    return siphash_base(2, 4, key, src, cnt);
}

uint64_t siphash_base(
    size_t c, size_t d, siphash_key_t key,
    const void* source, size_t count
) {
    const uint64_t* src = source;
    const uint64_t put_size = count & 255;
    bool has_next_block = true;

    uint64_t v0 = key.low  ^ 0x736f6d6570736575;
    uint64_t v1 = key.high ^ 0x646f72616e646f6d;
    uint64_t v2 = key.low  ^ 0x6c7967656e657261;
    uint64_t v3 = key.high ^ 0x7465646279746573;

    while (count > 0 || has_next_block) {
        uint64_t mi = 0;
        if (count >= 8) {
            mi = *src++; count -= 8;
        } else {
            const uint8_t* data = (const uint8_t*)src;
            for (size_t i = 0; i < count; i++)
                mi |= ((uint64_t)(*data++) & 255) << (i * 8);
            mi |= put_size << 56;
            has_next_block = false;
            count = 0;
        }

        v3 ^= mi;
        for (size_t i = 0; i < c; i++)
            sipround(&v0, &v1, &v2, &v3);
        v0 ^= mi;
    }

    v2 ^= 0xff;
    for (size_t i = 0; i < d; i++)
        sipround(&v0, &v1, &v2, &v3);

    return v0 ^ v1 ^ v2 ^ v3;
}

#endif // SIPHASH_IMPLEMENTATION