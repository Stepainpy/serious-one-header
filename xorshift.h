#ifndef XORSHIFT_PRNG_H
#define XORSHIFT_PRNG_H
#include <stdint.h>

/* Macros description
XORSHIFT_STATIC_STATE   - storage generator state as static variable
XORSHIFT_IMPLEMENTATION - add implementation of functions
*/

#ifndef XORSHIFT_DEFAULT_SEED
#define XORSHIFT_DEFAULT_SEED 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XORSHIFT_STATIC_STATE

uint32_t xorshift32_state(uint32_t seed);
uint64_t xorshift64_state(uint64_t seed);
uint64_t xorshift64s_state(uint64_t seed);

// xorshift 32-bit
uint32_t xorshift32(void);
// xorshift 64-bit
uint64_t xorshift64(void);
// xorshift* 64-bit
uint64_t xorshift64s(void);

#else

typedef struct { uint32_t state; } xorshift32_state;
typedef struct { uint64_t state; } xorshift64_state;
typedef struct { uint64_t state; } xorshift64s_state;

// xorshift 32-bit
uint32_t xorshift32(xorshift32_state * const state);
// xorshift 64-bit
uint64_t xorshift64(xorshift64_state * const state);
// xorshift* 64-bit
uint64_t xorshift64s(xorshift64s_state * const state);

#endif // XORSHIFT_STATIC_STATE

#ifdef __cplusplus
}
#endif

#endif // XORSHIFT_PRNG_H

#ifdef XORSHIFT_IMPLEMENTATION

#ifdef XORSHIFT_STATIC_STATE

uint32_t xorshift32_state(uint32_t seed) {
    static uint32_t state = XORSHIFT_DEFAULT_SEED;
    if (seed) state = seed;
    return state;
}

uint64_t xorshift64_state(uint64_t seed) {
    static uint64_t state = XORSHIFT_DEFAULT_SEED;
    if (seed) state = seed;
    return state;
}

uint64_t xorshift64s_state(uint64_t seed) {
    static uint64_t state = XORSHIFT_DEFAULT_SEED;
    if (seed) state = seed;
    return state;
}

uint32_t xorshift32(void) {
    uint32_t x = xorshift32_state(0);
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    (void)xorshift32_state(x);
    return x;
}

uint64_t xorshift64(void) {
    uint64_t x = xorshift64_state(0);
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    (void)xorshift64_state(x);
    return x;
}

uint64_t xorshift64s(void) {
    uint64_t x = xorshift64s_state(0);
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    x *= UINT64_C(2685821657736338717);
    (void)xorshift64s_state(x);
    return x;
}

#else

uint32_t xorshift32(xorshift32_state * const state) {
    uint32_t x = state->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state->state = x;
}

uint64_t xorshift64(xorshift64_state * const state) {
    uint64_t x = state->state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return state->state = x;
}

uint64_t xorshift64s(xorshift64s_state * const state) {
    uint64_t x = state->state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    x *= UINT64_C(2685821657736338717);
    return state->state = x;
}

#endif // XORSHIFT_STATIC_STATE

#endif // XORSHIFT_IMPLEMENTATION