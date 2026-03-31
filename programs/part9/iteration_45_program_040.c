#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* GCC vector types */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 8x int32 */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 16x int32 */
typedef float v8sf __attribute__((vector_size(32)));        /* 8x float */
typedef float v16sf __attribute__((vector_size(64)));       /* 16x float */
typedef double v4df __attribute__((vector_size(32)));       /* 4x double */
typedef double v8df __attribute__((vector_size(64)));       /* 8x double */

/* Global data arrays */
static int32_t global_ints[512];
static float global_floats[512];
static double global_doubles[512];

/* Accumulator arrays */
static int32_t accum_ints[512];
static float accum_floats[512];
static double accum_doubles[512];

/* Volatile variables to prevent optimization */
volatile int volatile_mask_seed = 0;

/* Initialize global arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    srand(seed);
    for (int i = 0; i < 512; i++) {
        global_ints[i] = rand() % 1000;
        global_floats[i] = (rand() % 1000) / 10.0f;
        global_doubles[i] = (rand() % 1000) / 10.0;
        accum_ints[i] = 0;
        accum_floats[i] = 0.0f;
        accum_doubles[i] = 0.0;
    }
}

/* Function using __builtin_shuffle with many operands (10+ total) */
#ifdef __AVX2__
void shuffle_int_avx2(const int32_t* src, int32_t* dst, int offset, volatile int mask_seed) {
    /* Load 512-bit vectors (16 ints each) */
    v16si v1 = *(const v16si*)(src + offset);
    v16si v2 = *(const v16si*)(src + offset + 16);
    v16si v3 = *(const v16si*)(src + offset + 32);
    v16si v4 = *(const v16si*)(src + offset + 48);
    
    /* Create control mask with volatile-dependent indices */
    int32_t mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = (i + mask_seed) % 64;  /* Mix indices from all 4 vectors */
    }
    v16si mask = *(v16si*)mask_arr;
    
    /* Complex control flow to stress expander */
    if (mask_seed & 1) {
        /* Use __builtin_shuffle with 3 input vectors and mask = 10 operands total */
        v16si result = __builtin_shuffle(v1, v2, v3, mask);
        
        /* Additional operation to prevent elimination */
        result = result + v4;
        
        /* Store result */
        *(v16si*)(dst + offset) = result;
    } else {
        /* Alternative path with different shuffle pattern */
        v16si mask2 = mask + 8;
        v16si result = __builtin_shuffle(v2, v3, v4, mask2);
        result = result * v1;
        *(v16si*)(dst + offset) = result;
    }
}
#endif

#ifdef __AVX512F__
void shuffle_float_avx512(const float* src, float* dst, int offset, volatile int mask_seed) {
    /* Load 512-bit float vectors */
    v16sf f1 = *(const v16sf*)(src + offset);
    v16sf f2 = *(const v16sf*)(src + offset + 16);
    v16sf f3 = *(const v16sf*)(src + offset + 32);
    v16sf f4 = *(const v16sf*)(src + offset + 48);
    
    /* Create volatile-dependent mask */
    int32_t mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = (i * 3 + mask_seed) % 64;
    }
    v16si mask = *(v16si*)mask_arr;
    
    /* Switch statement with different shuffle patterns */
    switch (mask_seed % 4) {
        case 0: {
            /* This should require 11 operands: 4 vectors + mask */
            v16sf result = __builtin_shuffle(f1, f2, f3, f4, mask);
            result = result * 2.0f;
            *(v16sf*)(dst + offset) = result;
            break;
        }
        case 1: {
            /* Alternative with __builtin_shufflevector */
            v16sf result = __builtin_shufflevector(f1, f2, f3, f4, 
                0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
            result = result + f1;
            *(v16sf*)(dst + offset) = result;
            break;
        }
        case 2: {
            /* Mixed shuffle pattern */
            v16sf temp = __builtin_shuffle(f1, f2, mask);
            v16sf result = __builtin_shuffle(temp, f3, f4, mask);
            *(v16sf*)(dst + offset) = result;
            break;
        }
        default: {
            /* Chain multiple shuffles */
            v16sf t1 = __builtin_shuffle(f1, f2, mask);
            v16sf t2 = __builtin_shuffle(f3, f4, mask + 8);
            v16sf result = __builtin_shuffle(t1, t2, mask);
            *(v16sf*)(dst + offset) = result;
            break;
        }
    }
}
#endif

#ifdef __AVX2__
void shuffle_double_avx(const double* src, double* dst, int offset, volatile int mask_seed) {
    /* 256-bit double vectors (4 doubles each) */
    v4df d1 = *(const v4df*)(src + offset);
    v4df d2 = *(const v4df*)(src + offset + 4);
    v4df d3 = *(const v4df*)(src + offset + 8);
    v4df d4 = *(const v4df*)(src + offset + 12);
    v4df d5 = *(const v4df*)(src + offset + 16);
    v4df d6 = *(const v4df*)(src + offset + 20);
    
    /* Create mask from volatile value */
    int64_t mask_arr[4];
    for (int i = 0; i < 4; i++) {
        mask_arr[i] = (i + mask_seed) % 24;
    }
    v4df mask = *(v4df*)mask_arr;
    
    /* Loop with conditional shuffle execution */
    for (int i = 0; i < 2; i++) {
        if ((mask_seed >> i) & 1) {
            /* Complex shuffle with many operands */
            v4df result = __builtin_shuffle(d1, d2, d3, d4, d5, mask);
            result = result * 1.5;
            *(v4df*)(dst + offset + i * 4) = result;
        } else {
            /* Different operand combination */
            v4df result = __builtin_shuffle(d2, d3, d4, d5, d6, mask);
            result = result / 2.0;
            *(v4df*)(dst + offset + i * 4) = result;
        }
        
        /* Volatile store to prevent optimization */
        volatile double* vptr = (volatile double*)(dst + offset + i * 4);
        *vptr = *(dst + offset + i * 4);
    }
}
#endif

/* SSE2 fallback for wider compatibility */
#ifdef __SSE2__
typedef int32_t v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

void shuffle_sse2(const int32_t* src, int32_t* dst, int offset, volatile int mask_seed) {
    v4si v1 = *(const v4si*)(src + offset);
    v4si v2 = *(const v4si*)(src + offset + 4);
    v4si v3 = *(const v4si*)(src + offset + 8);
    v4si v4 = *(const v4si*)(src + offset + 12);
    v4si v5 = *(const v4si*)(src + offset + 16);
    
    int32_t mask_arr[4];
    for (int i = 0; i < 4; i++) {
        mask_arr[i] = (i * 2 + mask_seed) % 20;
    }
    v4si mask = *(v4si*)mask_arr;
    
    /* Multiple shuffle patterns in control flow */
    if (mask_seed > 100) {
        v4si result = __builtin_shuffle(v1, v2, v3, v4, mask);
        *(v4si*)(dst + offset) = result;
    } else {
        v4si result = __builtin_shuffle(v2, v3, v4, v5, mask);
        *(v4si*)(dst + offset) = result;
    }
}
#endif

/* Mixed SIMD width operations */
void mixed_width_shuffle(int seed) {
    /* Use smaller vectors to create larger ones via shuffles */
    v8si small1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si small2 = {9, 10, 11, 12, 13, 14, 15, 16};
    
    /* Create a 16-element vector from two 8-element vectors */
    int32_t mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = (i + seed) % 16;
    }
    
    /* This may trigger complex expansion with many operands */
    v16si combined = __builtin_shufflevector(small1, small2,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    
    /* Store to volatile location */
    volatile v16si* vptr = &combined;
    (void)vptr;
}

int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    volatile_mask_seed = seed;
    
    /* Main processing loop */
    for (int iter = 0; iter < 10; iter++) {
        int offset = (iter * 32) % 256;  /* Ensure within bounds */
        
        /* Call architecture-specific shuffle functions */
#ifdef __AVX2__
        shuffle_int_avx2(global_ints, accum_ints, offset, volatile_mask_seed + iter);
#endif
        
#ifdef __AVX512F__
        shuffle_float_avx512(global_floats, accum_floats, offset, volatile_mask_seed + iter);
#endif
        
#ifdef __AVX2__
        shuffle_double_avx(global_doubles, accum_doubles, offset, volatile_mask_seed + iter);
#endif
        
#ifdef __SSE2__
        shuffle_sse2(global_ints, accum_ints, offset * 2, volatile_mask_seed + iter);
#endif
        
        /* Mixed width operations */
        mixed_width_shuffle(seed + iter);
    }
    
    /* Compute checksums to prevent dead code elimination */
    int64_t int_sum = 0;
    double float_sum = 0.0;
    double double_sum = 0.0;
    
    for (int i = 0; i < 512; i++) {
        int_sum += accum_ints[i];
        float_sum += accum_floats[i];
        double_sum += accum_doubles[i];
    }
    
    /* Print results to ensure computation isn't optimized away */
    printf("Checksums:\n");
    printf("  Integers: %ld\n", int_sum);
    printf("  Floats: %f\n", float_sum);
    printf("  Doubles: %f\n", double_sum);
    printf("  Combined: %f\n", float_sum + double_sum + int_sum);
    
    return 0;
}
