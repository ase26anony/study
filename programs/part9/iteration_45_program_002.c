#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
#define ARRAY_SIZE 512
static volatile int global_seed;
static int32_t global_int_array[ARRAY_SIZE];
static double global_float_array[ARRAY_SIZE];
static int32_t accumulator_int[ARRAY_SIZE];
static double accumulator_float[ARRAY_SIZE];

/* Vector type definitions using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    global_seed = seed;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = (i * 1103515245 + seed) & 0x7FFFFFFF;
        global_float_array[i] = (double)((i * 1103515245 + seed) & 0xFFF) / 4096.0;
        accumulator_int[i] = 0;
        accumulator_float[i] = 0.0;
    }
}

/* Function using __builtin_shuffle with many operands - targeting 10+ operands */
#ifdef __AVX2__
static v8si shuffle_10_operand_int(v8si a, v8si b, v8si c, v8si d, 
                                   v8si mask1, v8si mask2, v8si mask3, 
                                   volatile int idx1, volatile int idx2, volatile int idx3) {
    /* Complex control flow to prevent optimization */
    v8si result;
    if (idx1 & 1) {
        /* First shuffle with 10 operands: 2 data vectors + 8-element mask */
        v8si temp = __builtin_shuffle(a, b, mask1);
        
        /* Nested control flow */
        switch (idx2 & 3) {
            case 0:
                /* Second shuffle mixing in another vector */
                result = __builtin_shuffle(temp, c, mask2);
                break;
            case 1:
                result = __builtin_shuffle(c, temp, mask2);
                break;
            case 2:
                result = __builtin_shuffle(temp, d, mask2);
                break;
            default:
                result = __builtin_shuffle(d, temp, mask2);
                break;
        }
        
        /* Third shuffle to reach 11 operands total in expansion */
        if (idx3 > 0) {
            v8si final_mask = mask3 + (v8si){idx3, idx3, idx3, idx3, idx3, idx3, idx3, idx3};
            result = __builtin_shuffle(result, a, final_mask);
        }
    } else {
        /* Alternative path with different shuffle pattern */
        v8si temp = __builtin_shuffle(b, a, mask2);
        result = __builtin_shuffle(temp, c, mask1);
    }
    
    return result;
}
#endif

#ifdef __AVX512F__
/* Function targeting exactly 11 operands with __builtin_shufflevector */
static v16si shufflevector_11_operand(v16si a, v16si b, v16si c, 
                                      volatile int idx1, volatile int idx2) {
    /* Use volatile indices to prevent constant folding */
    volatile int start = idx1 % 8;
    volatile int count = 8 + (idx2 % 8);
    
    /* __builtin_shufflevector with explicit indices - can generate many operands */
    /* We'll create a pattern that requires 11 operands during expansion */
    v16si result;
    
    if (start & 1) {
        /* Complex shuffle pattern that may expand to 11 operands */
        result = __builtin_shufflevector(a, b, 
            0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
        
        /* Additional operation to ensure the result is used */
        result = result + (v16si){idx1, idx1, idx1, idx1, idx1, idx1, idx1, idx1,
                                  idx2, idx2, idx2, idx2, idx2, idx2, idx2, idx2};
    } else {
        /* Different shuffle pattern */
        result = __builtin_shufflevector(b, a,
            23, 7, 22, 6, 21, 5, 20, 4, 19, 3, 18, 2, 17, 1, 16, 0);
    }
    
    /* Mix with third vector based on runtime condition */
    if ((idx1 + idx2) & 1) {
        v16si temp = __builtin_shufflevector(result, c,
            0, 32, 2, 34, 4, 36, 6, 38, 8, 40, 10, 42, 12, 44, 14, 46);
        result = temp;
    }
    
    return result;
}

/* Mixed floating-point shuffle with many operands */
static v8df shuffle_double_10_operand(v8df a, v8df b, v8df c, v8df mask_vec,
                                      volatile int pattern) {
    v8df result;
    
    /* Create complex control flow */
    for (int i = 0; i < 3; i++) {
        if (i == (pattern % 3)) {
            /* Shuffle that may require many operands during expansion */
            v8df temp = __builtin_shuffle(a, b, 
                (v8si){0, 8, 1, 9, 2, 10, 3, 11});
            
            /* Another shuffle mixing results */
            result = __builtin_shuffle(temp, c,
                (v8si){7, 6, 5, 4, 3, 2, 1, 0});
            
            /* Break early based on volatile condition */
            if (pattern > 100) break;
        } else if (i == ((pattern + 1) % 3)) {
            result = __builtin_shuffle(b, a,
                (v8si){4, 5, 6, 7, 0, 1, 2, 3});
        } else {
            result = __builtin_shuffle(a, c,
                (v8si){0, 1, 2, 3, 4, 5, 6, 7});
        }
        
        /* Modify mask based on loop iteration */
        mask_vec = mask_vec + (v8df){1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    }
    
    return result;
}
#endif

/* Mixed-size vector operations */
#ifdef __SSE2__
static void mixed_size_operations(volatile int idx) {
    typedef int32_t v4si __attribute__((vector_size(16)));
    
    v4si small_vec1 = {global_int_array[idx], global_int_array[idx+1], 
                       global_int_array[idx+2], global_int_array[idx+3]};
    v4si small_vec2 = {global_int_array[idx+4], global_int_array[idx+5], 
                       global_int_array[idx+6], global_int_array[idx+7]};
    
    /* Shuffle that expands to multiple operands */
    v4si shuffled = __builtin_shuffle(small_vec1, small_vec2, 
        (v4si){0, 4, 1, 5});
    
    /* Store to accumulator */
    for (int i = 0; i < 4; i++) {
        accumulator_int[idx + i] += shuffled[i];
    }
}
#endif

/* Main processing function with complex control flow */
void process_vectors(int iterations, volatile int base_idx) {
    for (int iter = 0; iter < iterations; iter++) {
        volatile int idx = (base_idx + iter * 16) % (ARRAY_SIZE - 64);
        volatile int pattern = (iter * 17 + global_seed) & 0xFF;
        
        #ifdef __AVX2__
        /* Load 256-bit vectors */
        v8si avx_int1 = *(v8si*)&global_int_array[idx];
        v8si avx_int2 = *(v8si*)&global_int_array[idx + 8];
        v8si avx_int3 = *(v8si*)&global_int_array[idx + 16];
        v8si avx_int4 = *(v8si*)&global_int_array[idx + 24];
        
        /* Create mask vectors with volatile elements */
        v8si mask1 = {pattern, pattern+1, pattern+2, pattern+3, 
                      pattern+4, pattern+5, pattern+6, pattern+7};
        v8si mask2 = {pattern+8, pattern+9, pattern+10, pattern+11,
                      pattern+12, pattern+13, pattern+14, pattern+15};
        v8si mask3 = {pattern+16, pattern+17, pattern+18, pattern+19,
                      pattern+20, pattern+21, pattern+22, pattern+23};
        
        /* Call function with potential 10-11 operand expansion */
        v8si result = shuffle_10_operand_int(avx_int1, avx_int2, avx_int3, avx_int4,
                                            mask1, mask2, mask3,
                                            idx, pattern, iter);
        
        /* Accumulate results */
        for (int i = 0; i < 8; i++) {
            accumulator_int[idx + i] += result[i];
        }
        #endif
        
        #ifdef __AVX512F__
        if (iter % 2 == 0) {
            /* Load 512-bit vectors */
            v16si avx512_int1 = *(v16si*)&global_int_array[idx];
            v16si avx512_int2 = *(v16si*)&global_int_array[idx + 16];
            v16si avx512_int3 = *(v16si*)&global_int_array[idx + 32];
            
            /* Call shufflevector function */
            v16si result512 = shufflevector_11_operand(avx512_int1, avx512_int2, 
                                                      avx512_int3, idx, pattern);
            
            /* Accumulate */
            for (int i = 0; i < 16; i++) {
                accumulator_int[idx + i] += result512[i];
            }
            
            /* Floating-point shuffles */
            v8df double_vec1 = *(v8df*)&global_float_array[idx];
            v8df double_vec2 = *(v8df*)&global_float_array[idx + 8];
            v8df double_vec3 = *(v8df*)&global_float_array[idx + 16];
            v8df mask_vec = {pattern * 0.1, pattern * 0.2, pattern * 0.3, pattern * 0.4,
                            pattern * 0.5, pattern * 0.6, pattern * 0.7, pattern * 0.8};
            
            v8df float_result = shuffle_double_10_operand(double_vec1, double_vec2,
                                                         double_vec3, mask_vec, pattern);
            
            for (int i = 0; i < 8; i++) {
                accumulator_float[idx + i] += float_result[i];
            }
        }
        #endif
        
        #ifdef __SSE2__
        /* Mixed size operations */
        mixed_size_operations(idx);
        #endif
        
        /* Volatile memory barrier */
        volatile int barrier = idx;
        (void)barrier;
    }
}

/* Compute checksum of results */
uint64_t compute_checksum(void) {
    uint64_t checksum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint64_t)accumulator_int[i];
        checksum += (uint64_t)(accumulator_float[i] * 1000.0);
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Initializing with seed: %d\n", seed);
    init_arrays(seed);
    
    volatile int base_index = seed % 128;
    
    /* Process with different iteration counts and patterns */
    process_vectors(10, base_index);
    
    #ifdef __AVX512F__
    process_vectors(5, base_index + 64);
    #endif
    
    #ifdef __AVX2__
    process_vectors(8, base_index + 128);
    #endif
    
    uint64_t checksum = compute_checksum();
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
