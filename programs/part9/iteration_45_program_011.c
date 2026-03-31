#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
static int32_t global_ints[512];
static double global_floats[512];
static volatile int32_t volatile_buffer[512];
static int32_t accumulator[512];

/* Vector types using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */

/* Initialize with deterministic pseudo-random values */
void init_arrays(int seed) {
    srand(seed);
    for (int i = 0; i < 512; i++) {
        global_ints[i] = (rand() % 1000) - 500;
        global_floats[i] = (rand() % 1000) / 100.0 - 5.0;
        accumulator[i] = 0;
    }
}

/* Function using __builtin_shuffle with many operands - targeting 10+ operands */
#ifdef __AVX2__
static v8si shuffle_10_operand_int(v8si a, v8si b, v8si mask1, v8si mask2, 
                                   v8si mask3, volatile int idx) {
    /* Complex control flow to prevent optimization */
    v8si result;
    if (idx & 1) {
        /* First shuffle pattern - uses 10 operands total */
        v8si temp = __builtin_shuffle(a, b, mask1);
        /* Second shuffle with different mask - forces expander usage */
        result = __builtin_shuffle(temp, a, mask2);
    } else {
        /* Alternative path with different shuffle */
        v8si temp = __builtin_shuffle(b, a, mask3);
        result = __builtin_shuffle(temp, b, mask1);
    }
    
    /* Volatile store to prevent dead code elimination */
    volatile_buffer[0] = result[0];
    return result;
}
#endif

#ifdef __AVX512F__
/* Function targeting exactly 11 operands for the 11-operand case */
static v16si shuffle_11_operand_int(v16si a, v16si b, v16si c, 
                                    v16si mask1, v16si mask2, 
                                    volatile int idx) {
    v16si result;
    
    /* Switch statement with volatile condition */
    switch (idx % 4) {
        case 0: {
            /* Pattern 0: Complex shuffle chain - 11 operands total */
            v16si temp1 = __builtin_shuffle(a, b, mask1);
            v16si temp2 = __builtin_shuffle(c, temp1, mask2);
            result = __builtin_shuffle(temp2, a, mask1);
            break;
        }
        case 1: {
            /* Pattern 1: Different arrangement */
            v16si temp1 = __builtin_shuffle(b, c, mask2);
            v16si temp2 = __builtin_shuffle(a, temp1, mask1);
            result = __builtin_shuffle(temp2, b, mask2);
            break;
        }
        case 2: {
            /* Pattern 2: Three-input shuffle simulation */
            v16si temp1 = __builtin_shuffle(a, b, mask1);
            result = __builtin_shuffle(temp1, c, mask2);
            break;
        }
        default: {
            /* Pattern 3: Simple fallback */
            result = __builtin_shuffle(a, b, mask1);
            break;
        }
    }
    
    volatile_buffer[1] = result[0];
    return result;
}

/* Mixed floating-point shuffle with conversion */
static v8df shuffle_mixed_fp(v8df a, v8df b, v16si int_mask, 
                             volatile int idx, int mod) {
    v8df result;
    
    /* Loop with condition to create complex CFG */
    for (int i = 0; i < 2; i++) {
        if ((idx + i) % mod == 0) {
            /* Use __builtin_shufflevector for explicit element selection */
            v8df temp = __builtin_shufflevector(a, b, 
                0, 2, 4, 6, 8, 10, 12, 14);
            result = __builtin_shufflevector(temp, a,
                7, 6, 5, 4, 3, 2, 1, 0);
        } else {
            /* Alternative shuffle pattern */
            v8df temp = __builtin_shufflevector(b, a,
                1, 3, 5, 7, 9, 11, 13, 15);
            result = __builtin_shufflevector(temp, b,
                0, 1, 2, 3, 4, 5, 6, 7);
        }
    }
    
    volatile_buffer[2] = (int32_t)result[0];
    return result;
}
#endif

/* SSE2 fallback for wider compatibility */
#ifdef __SSE2__
typedef int32_t v4si __attribute__((vector_size(16)));
static v4si shuffle_sse2(v4si a, v4si b, v4si mask, volatile int idx) {
    v4si result;
    
    /* Nested if-else to create control flow */
    if (idx > 100) {
        result = __builtin_shuffle(a, b, mask);
    } else if (idx > 50) {
        v4si temp = __builtin_shuffle(b, a, mask);
        result = __builtin_shuffle(temp, a, mask);
    } else {
        result = a + b;
    }
    
    volatile_buffer[3] = result[0];
    return result;
}
#endif

/* Main test function that exercises all patterns */
void test_vector_shuffles(int seed, int iterations) {
    /* Initialize masks with runtime-dependent values */
    v8si mask1 = {0, 2, 4, 6, 1, 3, 5, 7};
    v8si mask2 = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si mask3 = {1, 0, 3, 2, 5, 4, 7, 6};
    
#ifdef __AVX512F__
    v16si mask_big1 = {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15};
    v16si mask_big2 = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    v16si int_mask_fp = {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15};
#endif
    
    for (int iter = 0; iter < iterations; iter++) {
        int base_idx = (iter * 37) % 256;  /* Non-linear progression */
        
#ifdef __AVX2__
        /* Test 256-bit integer shuffles */
        v8si avx2_vec1 = *(v8si*)&global_ints[base_idx];
        v8si avx2_vec2 = *(v8si*)&global_ints[base_idx + 8];
        v8si avx2_result = shuffle_10_operand_int(avx2_vec1, avx2_vec2, 
                                                  mask1, mask2, mask3, 
                                                  base_idx);
        
        /* Accumulate results */
        for (int i = 0; i < 8; i++) {
            accumulator[base_idx + i] += avx2_result[i];
        }
#endif

#ifdef __AVX512F__
        /* Test 512-bit integer shuffles - targeting 11 operands */
        v16si avx512_vec1 = *(v16si*)&global_ints[base_idx];
        v16si avx512_vec2 = *(v16si*)&global_ints[base_idx + 16];
        v16si avx512_vec3 = *(v16si*)&global_ints[base_idx + 32];
        
        v16si avx512_result = shuffle_11_operand_int(avx512_vec1, avx512_vec2,
                                                    avx512_vec3, mask_big1,
                                                    mask_big2, base_idx);
        
        /* Accumulate results */
        for (int i = 0; i < 16; i++) {
            accumulator[base_idx + i] += avx512_result[i];
        }
        
        /* Test mixed floating-point shuffles */
        v8df fp_vec1 = *(v8df*)&global_floats[base_idx];
        v8df fp_vec2 = *(v8df*)&global_floats[base_idx + 8];
        
        v8df fp_result = shuffle_mixed_fp(fp_vec1, fp_vec2, int_mask_fp,
                                         base_idx, iterations);
        
        /* Convert and accumulate */
        for (int i = 0; i < 8; i++) {
            accumulator[base_idx + i] += (int32_t)fp_result[i];
        }
#endif

#ifdef __SSE2__
        /* Test SSE2 shuffles for baseline */
        v4si sse2_vec1 = *(v4si*)&global_ints[base_idx];
        v4si sse2_vec2 = *(v4si*)&global_ints[base_idx + 4];
        v4si sse2_mask = {3, 2, 1, 0};
        
        v4si sse2_result = shuffle_sse2(sse2_vec1, sse2_vec2, sse2_mask, base_idx);
        
        for (int i = 0; i < 4; i++) {
            accumulator[base_idx + i] += sse2_result[i];
        }
#endif
    }
}

int main(int argc, char *argv[]) {
    int seed = 42;
    int iterations = 10;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Testing vector shuffles with seed=%d, iterations=%d\n", seed, iterations);
    
    /* Initialize data */
    init_arrays(seed);
    
    /* Run the vector tests */
    test_vector_shuffles(seed, iterations);
    
    /* Compute checksum */
    int64_t checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
    }
    
    printf("Final checksum: %lld\n", (long long)checksum);
    
    /* Also print a few values to ensure computation isn't optimized away */
    printf("Sample accumulator values: %d, %d, %d\n", 
           accumulator[0], accumulator[128], accumulator[256]);
    
    return 0;
}
