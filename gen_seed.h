#ifndef GEN_SEED_H
#define GEN_SEED_H

// from: https://stackoverflow.com/questions/322938/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long gs_mix3(unsigned long a, unsigned long b, unsigned long c);
unsigned long generate_seed(void);

#ifdef __cplusplus
}
#endif

#ifdef GEN_SEED_IMPLEMENTATION

#include <unistd.h>
#include <time.h>

unsigned long gs_mix3(unsigned long a, unsigned long b, unsigned long c) {
    a -= b; a -= c; a ^= (c >> 13);
    b -= c; b -= a; b ^= (a <<  8);
    c -= a; c -= b; c ^= (b >> 13);
    a -= b; a -= c; a ^= (c >> 12);
    b -= c; b -= a; b ^= (a << 16);
    c -= a; c -= b; c ^= (b >>  5);
    a -= b; a -= c; a ^= (c >>  3);
    b -= c; b -= a; b ^= (a << 10);
    c -= a; c -= b; c ^= (b >> 15);
    return c;
}

unsigned long generate_seed(void) {
    return gs_mix3(clock(), time(0), getpid());
}

#endif // GEN_SEED_IMPLEMENTATION

#endif // GEN_SEED_H