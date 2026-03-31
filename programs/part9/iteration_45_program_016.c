#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays initialized with deterministic pseudo-random data */
#define ARRAY_SIZE 512
static int32_t global_int_array[ARRAY_SIZE];
static float global_float_array[ARRAY_SIZE];
static int32_t accumulator_int[ARRAY_SIZE];
static float accumulator_float[ARRAY_SIZE];

/* Vector type definitions using GCC extensions */
typedef int32_t v4si __attribute__((vector_size(16)));
typedef int32_t v8si __attribute__((vector_size(32)));
typedef int32_t v16si __attribute__((vector_size(64)));
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef double v8df __attribute__((vector_size(64)));

/* Initialize global arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    unsigned int r = seed;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Simple LCG for deterministic randomness */
        r = r * 1103515245 + 12345;
        global_int_array[i] = (int32_t)(r >> 16) & 0x7FFF;
        global_float_array[i] = (float)((r >> 16) & 0x7FFF) / 1000.0f;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0f;
    }
}

/* Function using __builtin_shuffle with 10+ operands (integer vectors) */
void shuffle_int_10_operands(int offset, volatile int mask_val) {
    /* Load 256-bit vectors (8 ints each) */
    v8si a = *(v8si*)(&global_int_array[offset]);
    v8si b = *(v8si*)(&global_int_array[offset + 8]);
    
    /* Complex control flow to prevent optimization */
    if (mask_val & 1) {
        /* Create a shuffle mask with runtime-dependent indices */
        v8si mask;
        for (int i = 0; i < 8; i++) {
            mask[i] = (mask_val + i) % 16;
        }
        
        /* This should generate 10 operands: a, b, and mask elements */
        v8si result = __builtin_shuffle(a, b, mask);
        
        /* Store with volatile to prevent elimination */
        volatile v8si* dest = (v8si*)(&accumulator_int[offset]);
        *dest = *dest + result;
    }
}

/* Function using __builtin_shufflevector with 11+ operands (float vectors) */
void shuffle_float_11_operands(int offset, volatile int mask_val) {
    /* Load 512-bit vectors (16 floats each) */
    v16sf a = *(v16sf*)(&global_float_array[offset]);
    v16sf b = *(v16sf*)(&global_float_array[offset + 16]);
    
    /* Switch statement to create complex control flow */
    switch (mask_val & 3) {
        case 0: {
            /* Use __builtin_shufflevector with explicit indices */
            v16sf result = __builtin_shufflevector(a, b, 
                0, 16, 1, 17, 2, 18, 3, 19,  /* 8 indices */
                4, 20, 5, 21, 6, 22, 7, 23); /* 8 more = 16 total */
            
            volatile v16sf* dest = (v16sf*)(&accumulator_float[offset]);
            *dest = *dest + result;
            break;
        }
        case 1: {
            /* Different pattern with runtime-dependent indices */
            int idx[16];
            for (int i = 0; i < 16; i++) {
                idx[i] = (mask_val * i) % 32;
            }
            
            /* This should be expanded to many operands */
            v16sf result = __builtin_shuffle(a, b, 
                (v16si){idx[0], idx[1], idx[2], idx[3], idx[4], idx[5], idx[6], idx[7],
                        idx[8], idx[9], idx[10], idx[11], idx[12], idx[13], idx[14], idx[15]});
            
            volatile v16sf* dest = (v16sf*)(&accumulator_float[offset]);
            *dest = *dest + result;
            break;
        }
        default:
            break;
    }
}

/* Mixed SIMD patterns with narrowing/expanding conversions */
void mixed_simd_patterns(int offset, volatile int pattern) {
#ifdef __AVX2__
    /* 256-bit integer operations */
    v8si avx_int = *(v8si*)(&global_int_array[offset]);
    v8si avx_int2 = *(v8si*)(&global_int_array[offset + 8]);
    
    if (pattern & 1) {
        /* Complex shuffle with many indices */
        v8si mask = {7, 6, 5, 4, 3, 2, 1, 0};
        v8si shuffled = __builtin_shuffle(avx_int, avx_int2, mask);
        
        /* Narrow to 128-bit */
        v4si low = __builtin_shufflevector(shuffled, shuffled, 0, 1, 2, 3);
        v4si high = __builtin_shufflevector(shuffled, shuffled, 4, 5, 6, 7);
        
        /* Expand back with different pattern */
        v8si expanded = __builtin_shufflevector(low, high,
            0, 1, 2, 3, 4, 5, 6, 7);
        
        volatile v8si* dest = (v8si*)(&accumulator_int[offset]);
        *dest = *dest + expanded;
    }
#endif
    
#ifdef __AVX512F__
    /* 512-bit float operations */
    v16sf avx512_float = *(v16sf*)(&global_float_array[offset]);
    
    /* Shuffle with pattern that might require many operands */
    v16si shuffle_mask;
    for (int i = 0; i < 16; i++) {
        shuffle_mask[i] = (pattern + i * 3) % 16;
    }
    
    v16sf shuffled = __builtin_shuffle(avx512_float, shuffle_mask);
    
    /* Store with volatile */
    volatile v16sf* dest = (v16sf*)(&accumulator_float[offset]);
    *dest = *dest + shuffled;
#endif
}

/* Double precision shuffle with many operands */
void double_shuffle_operations(int offset, volatile int mask) {
#ifdef __AVX512F__
    v8df a = *(v8df*)((double*)&global_float_array[offset]);
    
    /* Create complex shuffle pattern */
    v8si mask_vec;
    for (int i = 0; i < 8; i++) {
        mask_vec[i] = (mask + i * 5) % 8;
    }
    
    /* Shuffle operation */
    v8df result = __builtin_shuffle(a, mask_vec);
    
    /* Additional arithmetic to ensure it's not dead */
    result = result * 2.0;
    
    volatile v8df* dest = (v8df*)((double*)&accumulator_float[offset]);
    *dest = *dest + result;
#endif
}

/* Main test driver */
int main(int argc, char* argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    volatile int runtime_mask = seed;
    
    /* Loop with different offsets and patterns */
    for (int iter = 0; iter < 10; iter++) {
        for (int offset = 0; offset < ARRAY_SIZE - 64; offset += 16) {
            /* Call various shuffle functions with runtime-dependent parameters */
            shuffle_int_10_operands(offset, runtime_mask + iter);
            shuffle_float_11_operands(offset, runtime_mask + iter * 2);
            mixed_simd_patterns(offset, runtime_mask + iter * 3);
            double_shuffle_operations(offset, runtime_mask + iter * 4);
        }
        
        /* Modify runtime_mask to create varying patterns */
        runtime_mask = runtime_mask * 1664525 + 1013904223;
    }
    
    /* Compute checksums to prevent dead code elimination */
    int64_t int_sum = 0;
    double float_sum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_sum += accumulator_int[i];
        float_sum += accumulator_float[i];
    }
    
    printf("Integer checksum: %ld\n", (long)int_sum);
    printf("Float checksum: %f\n", float_sum);
    
    return 0;
}
