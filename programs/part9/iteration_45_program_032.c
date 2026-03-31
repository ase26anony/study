#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
volatile int global_seed;
int global_int_array[512] __attribute__((aligned(64)));
float global_float_array[512] __attribute__((aligned(64)));
int accumulator[512] __attribute__((aligned(64)));

/* Vector type definitions */
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef int v16si __attribute__((vector_size(64)));
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef double v8df __attribute__((vector_size(64)));

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    for (int i = 0; i < 512; i++) {
        global_int_array[i] = (i * 1103515245 + seed) % 1000;
        global_float_array[i] = (float)((i * 1103515245 + seed) % 1000) / 100.0f;
        accumulator[i] = 0;
    }
}

/* Function using __builtin_shuffle with many operands - targeting 10+ operands */
#ifdef __AVX2__
void shuffle_10_operand_int(v16si *result, const v16si *a, const v16si *b, volatile int mask_idx) {
    v16si v1 = *a;
    v16si v2 = *b;
    
    /* Complex control flow to prevent optimization */
    if (mask_idx & 1) {
        /* Create a shuffle mask with 16 elements - this requires many operands */
        v16si mask = {
            mask_idx % 16, (mask_idx + 1) % 16, (mask_idx + 2) % 16, (mask_idx + 3) % 16,
            (mask_idx + 4) % 16, (mask_idx + 5) % 16, (mask_idx + 6) % 16, (mask_idx + 7) % 16,
            (mask_idx + 8) % 16, (mask_idx + 9) % 16, (mask_idx + 10) % 16, (mask_idx + 11) % 16,
            (mask_idx + 12) % 16, (mask_idx + 13) % 16, (mask_idx + 14) % 16, (mask_idx + 15) % 16
        };
        
        /* __builtin_shuffle with 2 vectors + mask = 3 operands, but expansion may need more */
        v16si shuffled = __builtin_shuffle(v1, v2, mask);
        
        /* Additional operation to create more operand pressure */
        v16si temp = shuffled + v1;
        *result = temp * v2;
    } else {
        /* Alternative path with different shuffle pattern */
        v16si mask2 = {
            15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0
        };
        
        v16si shuffled = __builtin_shuffle(v1, v2, mask2);
        *result = shuffled - v1;
    }
}
#endif

#ifdef __AVX512F__
/* Function specifically designed to require 11+ operands during expansion */
void shuffle_11_operand_float(v16sf *result, const v16sf *a, const v16sf *b, 
                              const v16sf *c, volatile int pattern) {
    v16sf v1 = *a;
    v16sf v2 = *b;
    v16sf v3 = *c;
    
    switch (pattern & 3) {
        case 0: {
            /* Complex shuffle pattern that may expand to many operands */
            v16si mask_int = {
                pattern, pattern + 1, pattern + 2, pattern + 3,
                pattern + 4, pattern + 5, pattern + 6, pattern + 7,
                pattern + 8, pattern + 9, pattern + 10, pattern + 11,
                pattern + 12, pattern + 13, pattern + 14, pattern + 15
            };
            
            /* Convert int mask to appropriate type for float shuffle */
            v16sf shuffled1 = __builtin_shuffle(v1, v2, mask_int);
            v16sf shuffled2 = __builtin_shuffle(v3, v1, mask_int);
            
            /* Mixed operations to stress different optab paths */
            *result = shuffled1 * shuffled2 + v3;
            break;
        }
        case 1: {
            /* Another pattern using __builtin_shufflevector */
            v16sf temp = __builtin_shufflevector(v1, v2, 
                0, 16, 1, 17, 2, 18, 3, 19,
                4, 20, 5, 21, 6, 22, 7, 23);
            
            /* Nested shuffle operations */
            v16si mask2 = {8, 9, 10, 11, 12, 13, 14, 15,
                          0, 1, 2, 3, 4, 5, 6, 7};
            v16sf final = __builtin_shuffle(temp, v3, mask2);
            *result = final;
            break;
        }
        default: {
            /* Default path with simple operation */
            *result = v1 + v2 + v3;
            break;
        }
    }
}
#endif

/* Mixed SIMD patterns with narrowing/expanding conversions */
#ifdef __SSE2__
void mixed_simd_patterns(v8si *result, const v8si *a, const v8si *b, volatile int idx) {
    v8si v1 = *a;
    v8si v2 = *b;
    
    /* Create complex control flow */
    for (int i = 0; i < 4; i++) {
        if ((idx >> i) & 1) {
            /* Shuffle pattern that may require many operands during expansion */
            v8si mask = { 
                (idx + i) % 8, (idx + i + 1) % 8, (idx + i + 2) % 8, (idx + i + 3) % 8,
                (idx + i + 4) % 8, (idx + i + 5) % 8, (idx + i + 6) % 8, (idx + i + 7) % 8
            };
            
            v8si shuffled = __builtin_shuffle(v1, v2, mask);
            
            /* Store to volatile memory to prevent elimination */
            volatile v8si volatile_store __attribute__((unused));
            volatile_store = shuffled;
            
            /* Accumulate result */
            *result = *result + shuffled;
        }
    }
}
#endif

/* Double precision shuffle with many operands */
#ifdef __AVX2__
void double_shuffle_pattern(v4df *result, const v4df *a, const v4df *b, 
                           const v4df *c, volatile int mask_val) {
    v4df v1 = *a;
    v4df v2 = *b;
    v4df v3 = *c;
    
    /* Complex expression that may expand to many operands */
    v4df mask_vec = { 
        (double)(mask_val % 4), 
        (double)((mask_val + 1) % 4),
        (double)((mask_val + 2) % 4),
        (double)((mask_val + 3) % 4)
    };
    
    /* Multiple shuffle operations in one expression */
    v4df temp1 = __builtin_shuffle(v1, v2, 
        (v4si){0, 4, 1, 5});  /* 4 operands visible, but expansion may need more */
    
    v4df temp2 = __builtin_shuffle(v3, v1, 
        (v4si){2, 6, 3, 7});
    
    /* Complex expression to increase operand count */
    *result = temp1 * mask_vec + temp2 / (mask_vec + 1.0);
}
#endif

/* Main test driver */
int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    global_seed = seed;
    init_arrays(seed);
    
    long long checksum = 0;
    
    /* Main test loop */
    for (int iter = 0; iter < 10; iter++) {
        volatile int pattern = (iter * 17 + seed) % 256;
        
#ifdef __AVX2__
        /* Test with 256-bit vectors */
        v8si avx2_vec1, avx2_vec2, avx2_result;
        memcpy(&avx2_vec1, &global_int_array[iter * 8], sizeof(v8si));
        memcpy(&avx2_vec2, &global_int_array[iter * 8 + 8], sizeof(v8si));
        
        mixed_simd_patterns(&avx2_result, &avx2_vec1, &avx2_vec2, pattern);
        
        /* Store result to accumulator */
        memcpy(&accumulator[iter * 8], &avx2_result, sizeof(v8si));
#endif

#ifdef __AVX512F__
        /* Test with 512-bit vectors - most likely to trigger 10-11 operand paths */
        v16si avx512_vec1, avx512_vec2, avx512_result;
        v16sf avx512_fvec1, avx512_fvec2, avx512_fvec3, avx512_fresult;
        
        memcpy(&avx512_vec1, &global_int_array[iter * 16], sizeof(v16si));
        memcpy(&avx512_vec2, &global_int_array[iter * 16 + 16], sizeof(v16si));
        
        shuffle_10_operand_int(&avx512_result, &avx512_vec1, &avx512_vec2, pattern);
        
        /* Store to memory to prevent optimization */
        memcpy(&accumulator[iter * 16], &avx512_result, sizeof(v16si));
        
        /* Test float version */
        memcpy(&avx512_fvec1, &global_float_array[iter * 16], sizeof(v16sf));
        memcpy(&avx512_fvec2, &global_float_array[iter * 16 + 16], sizeof(v16sf));
        memcpy(&avx512_fvec3, &global_float_array[iter * 16 + 32], sizeof(v16sf));
        
        shuffle_11_operand_float(&avx512_fresult, &avx512_fvec1, &avx512_fvec2, 
                                &avx512_fvec3, pattern);
        
        /* Convert float result to int and accumulate */
        for (int i = 0; i < 16; i++) {
            accumulator[iter * 16 + i] += (int)avx512_fresult[i];
        }
#endif

        /* Additional volatile operations to prevent dead code elimination */
        volatile int volatile_sink __attribute__((unused)) = pattern;
    }
    
    /* Compute final checksum */
    for (int i = 0; i < 512; i++) {
        checksum += accumulator[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    printf("Seed used: %d\n", seed);
    
    return 0;
}
