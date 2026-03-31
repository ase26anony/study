#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Vector type definitions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 8x int32 (256-bit) */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 16x int32 (512-bit) */
typedef float v8sf __attribute__((vector_size(32)));        /* 8x float (256-bit) */
typedef float v16sf __attribute__((vector_size(64)));       /* 16x float (512-bit) */
typedef double v4df __attribute__((vector_size(32)));       /* 4x double (256-bit) */
typedef double v8df __attribute__((vector_size(64)));       /* 8x double (512-bit) */

/* Global data arrays */
static int32_t global_ints[512];
static float global_floats[512];
static double global_doubles[512];

/* Accumulator arrays */
static int32_t accum_ints[512];
static float accum_floats[512];
static double accum_doubles[512];

/* Volatile variables to prevent constant folding */
volatile int volatile_mask_seed = 0;
volatile int volatile_control = 0;

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

/* Function 1: Large integer shuffle with 10+ operands */
#ifdef __AVX2__
void shuffle_large_int_vector(int offset, int mask_mod) {
    /* Load 512-bit vectors (16 ints each) */
    v16si vec1 = *(v16si*)&global_ints[offset];
    v16si vec2 = *(v16si*)&global_ints[offset + 16];
    
    /* Create control mask with runtime-dependent indices */
    int32_t mask_data[16];
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (i + volatile_mask_seed + mask_mod) % 32;
    }
    v16si mask = *(v16si*)mask_data;
    
    /* Complex shuffle with many operands - may expand to 10+ operands */
    v16si result;
    
    if (volatile_control & 1) {
        /* Pattern 1: __builtin_shuffle with 3 vector arguments */
        result = __builtin_shuffle(vec1, vec2, mask);
    } else {
        /* Pattern 2: Multiple shuffle operations combined */
        v16si temp1 = __builtin_shuffle(vec1, vec1, mask);
        v16si temp2 = __builtin_shuffle(vec2, vec2, mask);
        result = temp1 + temp2;
    }
    
    /* Store result to accumulator */
    v16si* accum_ptr = (v16si*)&accum_ints[offset];
    *accum_ptr = *accum_ptr + result;
    
    /* Volatile store to prevent optimization */
    volatile v16si* volatile_ptr = (volatile v16si*)&global_ints[offset];
    *volatile_ptr = result;
}
#endif

/* Function 2: Mixed float/double shuffle with complex control flow */
#ifdef __AVX512F__
void shuffle_mixed_vectors(int offset, int pattern) {
    v8sf float_vec1 = *(v8sf*)&global_floats[offset];
    v8sf float_vec2 = *(v8sf*)&global_floats[offset + 8];
    v4df double_vec1 = *(v4df*)&global_doubles[offset];
    v4df double_vec2 = *(v4df*)&global_doubles[offset + 4];
    
    /* Runtime-dependent control flow */
    switch (pattern % 4) {
        case 0: {
            /* Complex shuffle pattern requiring many operands */
            int32_t mask_int[8];
            for (int i = 0; i < 8; i++) {
                mask_int[i] = (i * 2 + volatile_mask_seed) % 16;
            }
            v8si int_mask = *(v8si*)mask_int;
            
            /* This may require 10+ operands during expansion */
            v8sf shuffled = __builtin_shuffle(float_vec1, float_vec2, int_mask);
            
            /* Additional operation */
            v8sf scaled = shuffled * (v8sf){2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
            
            v8sf* accum_ptr = (v8sf*)&accum_floats[offset];
            *accum_ptr = *accum_ptr + scaled;
            break;
        }
        case 1: {
            /* Double vector shuffle with __builtin_shufflevector */
            int64_t mask_dbl[4];
            for (int i = 0; i < 4; i++) {
                mask_dbl[i] = (i + pattern + volatile_mask_seed) % 8;
            }
            
            /* __builtin_shufflevector can require many operands */
            v4df shuffled = __builtin_shufflevector(double_vec1, double_vec2, 
                mask_dbl[0], mask_dbl[1], mask_dbl[2], mask_dbl[3]);
            
            v4df* accum_ptr = (v4df*)&accum_doubles[offset];
            *accum_ptr = *accum_ptr + shuffled;
            break;
        }
        case 2: {
            /* Nested shuffles - may create complex expansion */
            v8sf temp1 = __builtin_shuffle(float_vec1, float_vec2, 
                (v8si){0, 2, 4, 6, 8, 10, 12, 14});
            v8sf temp2 = __builtin_shuffle(float_vec1, float_vec2, 
                (v8si){1, 3, 5, 7, 9, 11, 13, 15});
            v8sf result = temp1 + temp2;
            
            v8sf* accum_ptr = (v8sf*)&accum_floats[offset];
            *accum_ptr = *accum_ptr + result;
            break;
        }
        case 3: {
            /* Mixed type operation */
            v4df dbl_result = double_vec1 + double_vec2;
            v4df* accum_ptr = (v4df*)&accum_doubles[offset];
            *accum_ptr = *accum_ptr + dbl_result;
            break;
        }
    }
}
#endif

/* Function 3: 512-bit vector operations with conditional execution */
#ifdef __AVX512VL
void large_vector_operations(int offset, int iter) {
    v16si int_vec = *(v16si*)&global_ints[offset];
    v16sf float_vec = *(v16sf*)&global_floats[offset];
    
    /* Complex control flow with volatile condition */
    if (volatile_control & (1 << (iter % 8))) {
        /* Create a large mask (16 elements) - may require 11 operands */
        int32_t complex_mask[16];
        for (int i = 0; i < 16; i++) {
            complex_mask[i] = (i + iter + volatile_mask_seed) % 32;
        }
        v16si mask_vec = *(v16si*)complex_mask;
        
        /* Large shuffle operation */
        v16si shuffled_int = __builtin_shuffle(int_vec, int_vec, mask_vec);
        
        /* Convert and mix with float operations */
        v16sf converted = (v16sf)shuffled_int;
        v16sf result = float_vec * converted;
        
        /* Store back */
        v16sf* accum_ptr = (v16sf*)&accum_floats[offset];
        *accum_ptr = *accum_ptr + result;
    } else {
        /* Alternative path with different shuffle pattern */
        int32_t alt_mask[16];
        for (int i = 0; i < 16; i++) {
            alt_mask[i] = (16 - i + iter) % 16;
        }
        v16si mask_vec = *(v16si*)alt_mask;
        
        v16si shuffled = __builtin_shuffle(int_vec, int_vec, mask_vec);
        v16si* accum_ptr = (v16si*)&accum_ints[offset];
        *accum_ptr = *accum_ptr + shuffled;
    }
}
#endif

/* Function 4: Narrowing and widening operations */
#ifdef __SSE2__
void narrowing_widening_ops(int offset) {
    /* Start with 256-bit, narrow to 128-bit, then widen back */
    v8si wide_vec = *(v8si*)&global_ints[offset];
    
    /* Extract halves */
    v8si mask_low = {0, 1, 2, 3, 0, 1, 2, 3};
    v8si mask_high = {4, 5, 6, 7, 4, 5, 6, 7};
    
    v8si low_half = __builtin_shuffle(wide_vec, wide_vec, mask_low);
    v8si high_half = __builtin_shuffle(wide_vec, wide_vec, mask_high);
    
    /* Combine with shufflevector requiring multiple operands */
    v8si reconstructed = __builtin_shufflevector(low_half, high_half,
        0, 1, 2, 3, 4, 5, 6, 7);
    
    /* Store result */
    v8si* accum_ptr = (v8si*)&accum_ints[offset];
    *accum_ptr = *accum_ptr + reconstructed;
}
#endif

/* Main function with complex execution flow */
int main(int argc, char* argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    /* Set volatile values from command line or random */
    volatile_mask_seed = seed % 256;
    volatile_control = seed;
    
    /* Main loop with mixed operations */
    for (int iter = 0; iter < 10; iter++) {
        for (int offset = 0; offset < 256; offset += 64) {
            /* Call different vector functions based on iteration */
            if (iter % 3 == 0) {
#ifdef __AVX2__
                shuffle_large_int_vector(offset, iter);
#endif
            } else if (iter % 3 == 1) {
#ifdef __AVX512F__
                shuffle_mixed_vectors(offset, iter);
#endif
            } else {
#ifdef __AVX512VL__
                large_vector_operations(offset, iter);
#endif
            }
            
#ifdef __SSE2__
            /* Always perform narrowing/widening */
            narrowing_widening_ops(offset % 128);
#endif
        }
        
        /* Modify volatile control periodically */
        if (iter % 4 == 0) {
            volatile_control ^= (1 << (iter % 16));
        }
    }
    
    /* Compute checksums */
    int64_t int_sum = 0;
    double float_sum = 0.0;
    double double_sum = 0.0;
    
    for (int i = 0; i < 512; i++) {
        int_sum += accum_ints[i];
        float_sum += accum_floats[i];
        double_sum += accum_doubles[i];
    }
    
    printf("Checksums:\n");
    printf("  Integers: %ld\n", (long)int_sum);
    printf("  Floats: %f\n", float_sum);
    printf("  Doubles: %f\n", double_sum);
    
    /* Final volatile operation to prevent dead code elimination */
    volatile int final_check = (int)(int_sum % 1000);
    printf("Final check: %d\n", final_check);
    
    return 0;
}
