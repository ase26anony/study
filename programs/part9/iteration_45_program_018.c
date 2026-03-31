#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
static volatile int32_t global_int_data[512];
static volatile double global_float_data[512];
static int32_t accumulator_int[512];
static double accumulator_float[512];

/* Vector type definitions using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 8x int32 */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 16x int32 */
typedef double v4df __attribute__((vector_size(32)));       /* 4x double */
typedef double v8df __attribute__((vector_size(64)));       /* 8x double */
typedef float v16sf __attribute__((vector_size(64)));       /* 16x float */

/* Initialize data with deterministic pseudo-random sequence */
void init_data(int seed) {
    srand(seed);
    for (int i = 0; i < 512; i++) {
        global_int_data[i] = rand() % 1000;
        global_float_data[i] = (rand() % 1000) / 10.0;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0;
    }
}

/* Function using __builtin_shuffle with many operands - targeting 10+ operands */
#ifdef __AVX2__
static v16si shuffle_large_int_vector(v16si a, v16si b, volatile int mask_idx) {
    /* Create control mask with runtime-dependent indices */
    int32_t mask_data[16];
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (mask_idx + i * 3) % 32;  /* Indices into 32-element combined vector */
    }
    
    /* Load mask into vector - volatile to prevent constant folding */
    volatile int32_t mask_vol[16];
    memcpy((void*)mask_vol, mask_data, sizeof(mask_data));
    v16si mask = *(v16si*)mask_vol;
    
    /* This shuffle operation requires many operands during RTL expansion:
     * 1. a (vector)
     * 2. b (vector) 
     * 3-18. mask elements (16 values)
     * Total: 18 operands > 11, should trigger uncovered case
     */
    v16si result = __builtin_shuffle(a, b, mask);
    
    /* Additional arithmetic to prevent dead code elimination */
    return result + (a & b);
}
#endif

#ifdef __AVX512F__
static v8df shuffle_large_double_vector(v8df a, v8df b, volatile int pattern) {
    /* Complex mask generation with runtime dependency */
    int64_t mask_data[8];
    for (int i = 0; i < 8; i++) {
        mask_data[i] = (pattern + i * 5) % 16;
    }
    
    volatile int64_t mask_vol[8];
    memcpy((void*)mask_vol, mask_data, sizeof(mask_data));
    v8df mask = *(v8df*)mask_vol;
    
    /* Another multi-operand shuffle */
    v8df result = __builtin_shuffle(a, b, mask);
    
    /* Mix with arithmetic */
    return result * (a + b);
}
#endif

/* Function with nested control flow around vector operations */
static void conditional_shuffle_operations(int mode, volatile int idx) {
    v8si vec1, vec2;
    
    /* Load data from global arrays */
    memcpy(&vec1, (void*)&global_int_data[idx * 8], sizeof(vec1));
    memcpy(&vec2, (void*)&global_int_data[idx * 8 + 8], sizeof(vec2));
    
    /* Complex control flow to stress expander */
    switch (mode % 4) {
        case 0: {
            /* Use __builtin_shufflevector which can require many operands */
            int32_t mask[8] = {7, 6, 5, 4, 3, 2, 1, 0};
            volatile int32_t vmask[8];
            memcpy((void*)vmask, mask, sizeof(mask));
            
            v8si result = __builtin_shufflevector(vec1, vec2, 
                vmask[0], vmask[1], vmask[2], vmask[3],
                vmask[4], vmask[5], vmask[6], vmask[7]);
            
            /* Store result */
            memcpy(&accumulator_int[idx * 8], &result, sizeof(result));
            break;
        }
        case 1: {
            /* Different shuffle pattern */
            int32_t mask[8] = {0, 2, 4, 6, 1, 3, 5, 7};
            volatile int32_t vmask[8];
            memcpy((void*)vmask, mask, sizeof(mask));
            
            v8si result = __builtin_shufflevector(vec1, vec2,
                vmask[0], vmask[1], vmask[2], vmask[3],
                vmask[4], vmask[5], vmask[6], vmask[7]);
            
            /* Mix with arithmetic */
            result = result + vec1 - vec2;
            memcpy(&accumulator_int[idx * 8], &result, sizeof(result));
            break;
        }
        case 2:
        case 3: {
            /* Even more complex pattern with conditional mask */
            int32_t mask[8];
            for (int i = 0; i < 8; i++) {
                mask[i] = (idx + i) % 16;
            }
            volatile int32_t vmask[8];
            memcpy((void*)vmask, mask, sizeof(mask));
            
            v8si result = __builtin_shufflevector(vec1, vec2,
                vmask[0], vmask[1], vmask[2], vmask[3],
                vmask[4], vmask[5], vmask[6], vmask[7]);
            
            /* Nested if inside switch */
            if (idx % 2 == 0) {
                result = result * 2;
            } else {
                result = result / 2;
            }
            memcpy(&accumulator_int[idx * 8], &result, sizeof(result));
            break;
        }
    }
}

/* Mixed SIMD patterns with size conversions */
static void mixed_simd_operations(int idx) {
    /* Work with 256-bit vectors */
    v8si wide_vec;
    memcpy(&wide_vec, (void*)&global_int_data[idx * 16], sizeof(wide_vec));
    
    /* Narrow to 128-bit equivalent */
    typedef int32_t v4si __attribute__((vector_size(16)));
    v4si narrow1, narrow2;
    
    /* Extract halves - each shuffle here requires multiple operands */
    int32_t mask_low[4] = {0, 1, 2, 3};
    int32_t mask_high[4] = {4, 5, 6, 7};
    volatile int32_t vmask_low[4], vmask_high[4];
    memcpy(vmask_low, mask_low, sizeof(mask_low));
    memcpy(vmask_high, mask_high, sizeof(mask_high));
    
    /* These shuffles contribute to operand count */
    narrow1 = __builtin_shufflevector(wide_vec, wide_vec,
        vmask_low[0], vmask_low[1], vmask_low[2], vmask_low[3]);
    narrow2 = __builtin_shufflevector(wide_vec, wide_vec,
        vmask_high[0], vmask_high[1], vmask_high[2], vmask_high[3]);
    
    /* Expand back to 256-bit */
    v8si expanded = __builtin_shufflevector(narrow1, narrow2,
        0, 1, 2, 3, 4, 5, 6, 7);
    
    /* Store result */
    memcpy(&accumulator_int[idx * 8], &expanded, sizeof(expanded));
}

/* Architecture-specific variants */
#ifdef __SSE2__
static void sse2_shuffle_operations(int idx) {
    typedef double v2df __attribute__((vector_size(16)));
    v2df vec1, vec2;
    
    memcpy(&vec1, (void*)&global_float_data[idx * 2], sizeof(vec1));
    memcpy(&vec2, (void*)&global_float_data[idx * 2 + 2], sizeof(vec2));
    
    /* Shuffle with runtime-dependent mask */
    int64_t mask[2];
    mask[0] = idx % 2;
    mask[1] = (idx + 1) % 2;
    volatile int64_t vmask[2];
    memcpy((void*)vmask, mask, sizeof(mask));
    
    v2df result = __builtin_shuffle(vec1, vec2, *(v2df*)vmask);
    
    /* Store to accumulator */
    memcpy(&accumulator_float[idx * 2], &result, sizeof(result));
}
#endif

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_data(seed);
    
    volatile int mode = seed % 10;  /* Volatile to prevent constant propagation */
    
    /* Main loop with various shuffle operations */
    for (int i = 0; i < 10; i++) {
        /* Conditional execution based on volatile variable */
        if (mode > 5) {
            conditional_shuffle_operations(i, i % 8);
        } else {
            mixed_simd_operations(i % 4);
        }
        
        /* Architecture-specific paths */
#ifdef __SSE2__
        sse2_shuffle_operations(i % 16);
#endif
        
#ifdef __AVX2__
        /* Use large vector shuffle */
        v16si avx2_vec1, avx2_vec2;
        memcpy(&avx2_vec1, (void*)&global_int_data[i * 16], sizeof(avx2_vec1));
        memcpy(&avx2_vec2, (void*)&global_int_data[i * 16 + 16], sizeof(avx2_vec2));
        
        v16si avx2_result = shuffle_large_int_vector(avx2_vec1, avx2_vec2, i);
        
        /* Store partial result */
        int32_t temp[16];
        memcpy(temp, &avx2_result, sizeof(avx2_result));
        for (int j = 0; j < 16; j++) {
            accumulator_int[(i * 16 + j) % 512] += temp[j];
        }
#endif
        
#ifdef __AVX512F__
        /* AVX-512 specific large shuffle */
        v8df avx512_vec1, avx512_vec2;
        memcpy(&avx512_vec1, (void*)&global_float_data[i * 8], sizeof(avx512_vec1));
        memcpy(&avx512_vec2, (void*)&global_float_data[i * 8 + 8], sizeof(avx512_vec2));
        
        v8df avx512_result = shuffle_large_double_vector(avx512_vec1, avx512_vec2, i);
        
        /* Accumulate result */
        double ftemp[8];
        memcpy(ftemp, &avx512_result, sizeof(avx512_result));
        for (int j = 0; j < 8; j++) {
            accumulator_float[(i * 8 + j) % 512] += ftemp[j];
        }
#endif
        
        /* Volatile memory store to prevent optimization */
        volatile int barrier = i;
        (void)barrier;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t int_sum = 0;
    double float_sum = 0.0;
    
    for (int i = 0; i < 512; i++) {
        int_sum += accumulator_int[i];
        float_sum += accumulator_float[i];
    }
    
    printf("Checksum - Integer: %lld, Float: %f\n", 
           (long long)int_sum, float_sum);
    
    return 0;
}
