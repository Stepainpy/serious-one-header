/* Simple implementation SHA-256 hash algorithm on C */
/*
Hash result:
                   sha256("Hello, world!", 13)
                                \/
  315f5bdb76d078c43b8ac0064e4a0164612b1fce77c869345bfc94c75894edd3
in structure:
  hash.value[ 0] = 0x31
  hash.value[ 1] = 0x5f
  hash.value[ 2] = 0x5b
           ...
  hash.value[29] = 0x94
  hash.value[30] = 0xed
  hash.value[31] = 0xd3
*/
#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>
#include <stdio.h>

#ifndef SHA256_DEF
#define SHA256_DEF
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { uint8_t value[32]; } sha256_hash_t;

SHA256_DEF sha256_hash_t sha256(const void* source, size_t count);
SHA256_DEF sha256_hash_t sha256_file(FILE* file);
SHA256_DEF void sha256_put_hash(const sha256_hash_t* hash, FILE* file);

#ifdef __cplusplus
}
#endif

#endif // SHA256_H

#ifdef SHA256_IMPLEMENTATION

#include <stdbool.h>
#include <string.h>

#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define sha256_d_le2be_u64 sha256_d_le2be_u64_
#define sha256_d_le2be_u32 sha256_d_le2be_u32_
#else
#define sha256_d_le2be_u64
#define sha256_d_le2be_u32
#endif

const uint32_t sha256_d_k[64] = {
    0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
    0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
    0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
    0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
    0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
    0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
    0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
    0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
    0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
    0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
    0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
    0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
    0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
    0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
    0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
    0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

uint64_t sha256_d_le2be_u64_(uint64_t n) {
    n = (n & 0xffffffff00000000) >> 32 | (n & 0x00000000ffffffff) << 32;
    n = (n & 0xffff0000ffff0000) >> 16 | (n & 0x0000ffff0000ffff) << 16;
    n = (n & 0xff00ff00ff00ff00) >>  8 | (n & 0x00ff00ff00ff00ff) <<  8;
    return n;
}

uint32_t sha256_d_le2be_u32_(uint32_t n) {
    n = (n & 0xffff0000) >> 16 | (n & 0x0000ffff) << 16;
    n = (n & 0xff00ff00) >>  8 | (n & 0x00ff00ff) <<  8;
    return n;
}

uint32_t sha256_d_rotr_u32(uint32_t n, uint8_t shift) {
    return n >> shift | n << (32 - shift);
}

size_t sha256_d_get_block(void* dbuf, size_t dbufe_sz, void** dof, size_t* count) {
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

sha256_hash_t sha256_d_base(void* source, size_t count) {
    bool has_next_block     = true;
    bool need_paste_one_bit = true;
    size_t all_readed = 0;

    uint32_t hi[8] = {
        0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
        0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
    };

    while ((count > 0 && count != (size_t)-1) || has_next_block) {
        uint32_t w[64] = {0};

        uint8_t* chunk = (uint8_t*)w; // as u64[8]
        size_t readed = sha256_d_get_block(chunk, 64, &source, &count);
        all_readed += readed;

        if (readed < 64 && need_paste_one_bit) {
            *(chunk + readed) = 0x80;
            need_paste_one_bit = false;
        }
        if (readed < 56) {
            *(uint64_t*)(chunk + 56) = // write to last u64
                sha256_d_le2be_u64(all_readed * 8);
            has_next_block = false;
        }

        for (size_t i = 0; i < 16; i++)
            w[i] = sha256_d_le2be_u32(w[i]);
        for (size_t i = 16; i < 64; i++) {
            uint32_t s0 = (w[i - 15] >> 3)
                ^ sha256_d_rotr_u32(w[i - 15],  7)
                ^ sha256_d_rotr_u32(w[i - 15], 18);
            uint32_t s1 = (w[i - 2] >> 10)
                ^ sha256_d_rotr_u32(w[i - 2], 17)
                ^ sha256_d_rotr_u32(w[i - 2], 19);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a, b, c, d, e, f, g, h;
        a = hi[0], b = hi[1], c = hi[2], d = hi[3],
        e = hi[4], f = hi[5], g = hi[6], h = hi[7];
        for (size_t i = 0; i < 64; i++) {
            uint32_t choice = (e & f) ^ (~e & g);
            uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
            uint32_t S0 =
                sha256_d_rotr_u32(a,  2) ^
                sha256_d_rotr_u32(a, 13) ^
                sha256_d_rotr_u32(a, 22);
            uint32_t S1 =
                sha256_d_rotr_u32(e,  6) ^
                sha256_d_rotr_u32(e, 11) ^
                sha256_d_rotr_u32(e, 25);
            uint32_t t1 = h + S1 + choice + sha256_d_k[i] + w[i];
            uint32_t t2 = S0 + majority;

            h = g; g = f; f = e; e =  d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }
        hi[0] += a, hi[1] += b, hi[2] += c, hi[3] += d,
        hi[4] += e, hi[5] += f, hi[6] += g, hi[7] += h;
    }

    sha256_hash_t out = {0};
    for (size_t i = 0; i < 8; i++)
        hi[i] = sha256_d_le2be_u32(hi[i]);
    memcpy(out.value, hi, sizeof out);
    return out;
}

sha256_hash_t sha256(const void* source, size_t count) {
    return sha256_d_base((void*)source, count);
}

sha256_hash_t sha256_file(FILE* file) {
    return sha256_d_base(file, -1);
}

void sha256_put_hash(const sha256_hash_t* hash, FILE* file) {
    for (size_t i = 0; i < 32; i++)
        fprintf(file, "%02hhx", hash->value[i]);
}

#endif // SHA256_IMPLEMENTATION