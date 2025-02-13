/* Implemenation siphash algorithm in C */
#ifndef SIPHASH_H
#define SIPHASH_H

#include <stdint.h>
#include <stdio.h>

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

SIPHASH_DEF uint64_t siphash(
    size_t c, size_t d, siphash_key_t key,
    const void* source, size_t count);
SIPHASH_DEF uint64_t siphash_2_4(siphash_key_t key,
    const void* source, size_t count);

SIPHASH_DEF uint64_t siphash_file(
    size_t c, size_t d, siphash_key_t key, FILE* file);
SIPHASH_DEF uint64_t siphash_2_4_file(
    siphash_key_t key, FILE* file);

#ifdef __cplusplus
}
#endif

#endif // SIPHASH_H

#ifdef SIPHASH_IMPLEMENTATION

#include <stdbool.h>
#include <string.h>

void siphash_d_rotl(uint64_t* n, uint8_t shift) {
    *n = (*n << shift) | (*n >> (64 - shift));
}

void siphash_d_round(
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

size_t siphash_d_get_block(void* dbuf, size_t dbufe_sz, void** dof, size_t* count) {
    if (*count == (size_t)-1) { // 'dof' is file
        return fread(dbuf, 1, dbufe_sz, (FILE*)*dof);
    } else if (*count >= dbufe_sz) {
        memcpy(dbuf, *dof, dbufe_sz);
        *dof = (uint8_t*)*dof + dbufe_sz;
        *count -= dbufe_sz;
        return dbufe_sz;
    } else {
        memcpy(dbuf, *dof, *count);
        size_t readed = *count;
        *count = 0;
        return readed;
    }
}

uint64_t siphash_d_base(
    size_t c, size_t d, siphash_key_t key,
    void* source, size_t count
) {
    bool has_next_block = true;
    uint64_t all_readed = 0;

    uint64_t v0 = key.low  ^ UINT64_C(0x736f6d6570736575);
    uint64_t v1 = key.high ^ UINT64_C(0x646f72616e646f6d);
    uint64_t v2 = key.low  ^ UINT64_C(0x6c7967656e657261);
    uint64_t v3 = key.high ^ UINT64_C(0x7465646279746573);

    while ((count > 0 && count != (size_t)-1) || has_next_block) {
        uint64_t mi = 0;
        size_t readed = siphash_d_get_block(&mi, 8, &source, &count);
        all_readed += readed;

        if (readed < 8) {
            mi |= (all_readed & 255) << 56;
            has_next_block = false;
        }

        v3 ^= mi;
        for (size_t i = 0; i < c; i++)
            siphash_d_round(&v0, &v1, &v2, &v3);
        v0 ^= mi;
    }

    v2 ^= 0xff;
    for (size_t i = 0; i < d; i++)
        siphash_d_round(&v0, &v1, &v2, &v3);

    return v0 ^ v1 ^ v2 ^ v3;
}

uint64_t siphash(size_t c, size_t d, siphash_key_t key,
    const void* source, size_t count) {
    return siphash_d_base(c, d, key, (void*)source, count);
}

uint64_t siphash_2_4(siphash_key_t key, const void* source, size_t count) {
    return siphash_d_base(2, 4, key, (void*)source, count);
}

uint64_t siphash_file(size_t c, size_t d, siphash_key_t key, FILE* file) {
    return siphash_d_base(c, d, key, file, -1);
}

uint64_t siphash_2_4_file(siphash_key_t key, FILE* file) {
    return siphash_d_base(2, 4, key, file, -1);
}

#endif // SIPHASH_IMPLEMENTATION