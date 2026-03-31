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
#define ARRAY_SIZE 512
static int32_t global_ints[ARRAY_SIZE];
static float global_floats[ARRAY_SIZE];
static double global_doubles[ARRAY_SIZE];

/* Accumulator arrays */
static int32_t acc_ints[ARRAY_SIZE];
static float acc_floats[ARRAY_SIZE];
static double acc_doubles[ARRAY_SIZE];

/* Initialize with deterministic pseudo-random sequence */
void init_arrays(int seed) {
    srand(seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_ints[i] = rand() % 1000;
        global_floats[i] = (rand() % 1000) / 10.0f;
        global_doubles[i] = (rand() % 1000) / 10.0;
        acc_ints[i] = 0;
        acc_floats[i] = 0.0f;
        acc_doubles[i] = 0.0;
    }
}

/* Function 1: Complex shuffle with 10+ operands using __builtin_shuffle */
__attribute__((noinline))
v16si shuffle_int_16way(v16si a, v16si b, volatile int mask_idx) {
    /* Create a control mask with runtime-dependent indices */
    int32_t mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = (mask_idx + i * 3) % 32;  /* 32 possible indices (0-31) */
    }
    
    /* Use volatile to prevent constant folding */
    volatile int32_t* volatile_mask = mask_arr;
    
    /* Create mask vector - this forces runtime evaluation */
    int32_t mask_load[16];
    for (int i = 0; i < 16; i++) {
        mask_load[i] = volatile_mask[i];
    }
    
    v16si mask = *(v16si*)mask_load;
    
    /* This __builtin_shuffle will need 3 operands during expansion:
       a, b, mask = 3 vector operands = 3 * 16 = 48 scalar elements
       The expander may pack them into 10+ operand slots */
    v16si result = __builtin_shuffle(a, b, mask);
    
    /* Additional arithmetic to prevent elimination */
    return result + (v16si){1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
}

/* Function 2: Float shuffle with mixed operations */
__attribute__((noinline))
v16sf shuffle_float_16way(v16sf a, v16sf b, volatile int pattern) {
    /* Complex control flow to stress expander */
    int32_t mask_arr[16];
    
    if (pattern & 1) {
        for (int i = 0; i < 16; i++) {
            mask_arr[i] = (pattern + i * 5) % 32;
        }
    } else {
        for (int i = 0; i < 16; i++) {
            mask_arr[i] = (pattern + i * 7) % 32;
        }
    }
    
    volatile int32_t* volatile_mask = mask_arr;
    int32_t mask_load[16];
    for (int i = 0; i < 16; i++) {
        mask_load[i] = volatile_mask[i];
    }
    
    v16si mask = *(v16si*)mask_load;
    
    /* Shuffle with many operands */
    v16sf result = __builtin_shuffle(a, b, mask);
    
    /* Conditional additional shuffle */
    if (pattern % 3 == 0) {
        /* Another shuffle with different mask */
        int32_t mask2_arr[16];
        for (int i = 0; i < 16; i++) {
            mask2_arr[i] = (pattern + i * 11) % 32;
        }
        v16si mask2 = *(v16si*)mask2_arr;
        result = __builtin_shuffle(result, a, mask2);
    }
    
    return result * 2.0f;
}

/* Function 3: Double precision shuffle with __builtin_shufflevector */
__attribute__((noinline))
v8df shuffle_double_8way(v8df a, v8df b, volatile int idx) {
    /* __builtin_shufflevector with explicit indices - each index is an operand */
    /* With 8+8+8 indices, this requires many operand slots */
    
    switch (idx % 4) {
        case 0:
            /* 24 explicit indices = potentially 24+ operands during expansion */
            return __builtin_shufflevector(a, b, 
                0, 8, 1, 9, 2, 10, 3, 11,  /* 8 operands */
                4, 12, 5, 13, 6, 14, 7, 15, /* 8 more */
                0, 8, 1, 9, 2, 10, 3, 11); /* 8 more = 24 total */
        case 1:
            return __builtin_shufflevector(a, b,
                7, 15, 6, 14, 5, 13, 4, 12,
                3, 11, 2, 10, 1, 9, 0, 8,
                7, 15, 6, 14, 5, 13, 4, 12);
        default:
            return __builtin_shufflevector(a, b,
                idx%8, (idx+1)%8, (idx+2)%8, (idx+3)%8,
                (idx+4)%8, (idx+5)%8, (idx+6)%8, (idx+7)%8,
                8+(idx%8), 8+((idx+1)%8), 8+((idx+2)%8), 8+((idx+3)%8),
                8+((idx+4)%8), 8+((idx+5)%8), 8+((idx+6)%8), 8+((idx+7)%8));
    }
}

/* Function 4: Mixed integer/float operations with nested control flow */
__attribute__((noinline))
void mixed_vector_ops(int iter, volatile int mode) {
    /* Load data from global arrays */
    v16si int_vec1 = *(v16si*)&global_ints[iter * 16];
    v16si int_vec2 = *(v16si*)&global_ints[iter * 16 + 16];
    
    v16sf float_vec1 = *(v16sf*)&global_floats[iter * 16];
    v16sf float_vec2 = *(v16sf*)&global_floats[iter * 16 + 16];
    
    v8df double_vec1 = *(v8df*)&global_doubles[iter * 8];
    v8df double_vec2 = *(v8df*)&global_doubles[iter * 8 + 8];
    
    /* Complex control flow */
    if (mode & 1) {
        /* Integer shuffle path */
        v16si int_result = shuffle_int_16way(int_vec1, int_vec2, iter);
        
        /* Store with volatile to prevent elimination */
        volatile v16si* volatile_store = (v16si*)&acc_ints[iter * 16];
        *volatile_store = *volatile_store + int_result;
    }
    
    if (mode & 2) {
        /* Float shuffle path with loop */
        for (int j = 0; j < 3; j++) {
            v16sf float_result = shuffle_float_16way(float_vec1, float_vec2, iter + j);
            
            /* Conditional store */
            if ((iter + j) % 2 == 0) {
                volatile v16sf* volatile_store = (v16sf*)&acc_floats[iter * 16];
                *volatile_store = *volatile_store + float_result;
            }
        }
    }
    
    if (mode & 4) {
        /* Double shuffle with switch */
        v8df double_result;
        switch (iter % 3) {
            case 0:
                double_result = shuffle_double_8way(double_vec1, double_vec2, iter);
                break;
            case 1:
                double_result = shuffle_double_8way(double_vec2, double_vec1, iter + 1);
                break;
            default:
                double_result = shuffle_double_8way(double_vec1, double_vec1, iter + 2);
                break;
        }
        
        volatile v8df* volatile_store = (v8df*)&acc_doubles[iter * 8];
        *volatile_store = *volatile_store + double_result;
    }
}

/* Function 5: Narrowing and widening operations */
__attribute__((noinline))
void width_conversion_ops(int iter) {
#ifdef __AVX2__
    /* 256-bit operations */
    v8si avx_int1 = *(v8si*)&global_ints[iter * 8];
    v8si avx_int2 = *(v8si*)&global_ints[iter * 8 + 8];
    
    /* Shuffle that might require many operands */
    int32_t mask_arr[8];
    for (int i = 0; i < 8; i++) {
        mask_arr[i] = (iter * 7 + i * 3) % 16;
    }
    
    v8si mask = *(v8si*)mask_arr;
    v8si result = __builtin_shuffle(avx_int1, avx_int2, mask);
    
    /* Store result */
    v8si* store_ptr = (v8si*)&acc_ints[iter * 8];
    *store_ptr = *store_ptr + result;
#endif
    
#ifdef __AVX512F__
    /* 512-bit operations - more likely to hit 10+ operand case */
    v16si avx512_int1 = *(v16si*)&global_ints[iter * 16];
    v16si avx512_int2 = *(v16si*)&global_ints[iter * 16 + 16];
    
    /* Complex mask generation */
    int32_t mask512_arr[16];
    volatile int vol_idx = iter;
    for (int i = 0; i < 16; i++) {
        mask512_arr[i] = (vol_idx + i * 13) % 32;
    }
    
    v16si mask512 = *(v16si*)mask512_arr;
    
    /* This shuffle with 512-bit vectors has high operand count */
    v16si result512 = __builtin_shuffle(avx512_int1, avx512_int2, mask512);
    
    /* Additional operation to prevent dead code */
    result512 = result512 + (v16si){1};
    
    volatile v16si* vol_store = (v16si*)&acc_ints[iter * 16];
    *vol_store = *vol_store + result512;
#endif
}

/* Main function with architecture-specific paths */
int main(int argc, char* argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    volatile int mode = 0;
    if (argc > 2) {
        mode = atoi(argv[2]);
    }
    
    /* Main loop with different vector operations */
    for (int iter = 0; iter < 10; iter++) {
        /* Vary the mode based on iteration */
        volatile int iter_mode = (mode + iter) % 8;
        
        /* Call mixed operations with volatile mode */
        mixed_vector_ops(iter, iter_mode);
        
        /* Call width conversion operations */
        width_conversion_ops(iter);
        
        /* Additional architecture-specific paths */
#ifdef __SSE2__
        /* 128-bit operations for baseline */
        typedef int32_t v4si __attribute__((vector_size(16)));
        v4si sse_vec1 = *(v4si*)&global_ints[iter * 4];
        v4si sse_vec2 = *(v4si*)&global_ints[iter * 4 + 4];
        
        int32_t sse_mask_arr[4] = {0, 2, 4, 6};
        v4si sse_mask = *(v4si*)sse_mask_arr;
        v4si sse_result = __builtin_shuffle(sse_vec1, sse_vec2, sse_mask);
        
        v4si* sse_store = (v4si*)&acc_ints[iter * 4];
        *sse_store = *sse_store + sse_result;
#endif
    }
    
    /* Compute checksum */
    int64_t int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_sum += acc_ints[i];
        float_sum += acc_floats[i];
        double_sum += acc_doubles[i];
    }
    
    /* Print results to prevent elimination */
    printf("Checksums:\n");
    printf("  Integer: %ld\n", (long)int_sum);
    printf("  Float: %f\n", float_sum);
    printf("  Double: %f\n", double_sum);
    
    /* Simple hash of all results */
    uint64_t hash = (uint64_t)int_sum ^ 
                   *(uint64_t*)&float_sum ^ 
                   *(uint64_t*)&double_sum;
    printf("Final hash: 0x%016lx\n", hash);
    
    return 0;
}
