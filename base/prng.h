#ifndef PRNG_H
#define PRNG_H

// NOTE(acol): SHISHUA is simd so it needs batching, it has decent randomness
typedef struct shishua_state {
    __m256i State[2];  // NOTE(acol): only state[2] is actual state the rest are here for zoomies
    __m256i Output;
    __m256i Counter;
} shishua_state;
static void ShishuaGen(shishua_state *restrict S, u64 *restrict Buffer, u64 Size);
static shishua_state ShishuaSeed(u64 Value[4]);

// NOTE(acol): JSF, it's quick and super simple but its not as random
typedef struct jsf_state {
    u64 A, B, C, D;
} jsf_state;
static u64 JsfGen(jsf_state *Series);
static jsf_state JsfSeed(u8 Value);

#endif
