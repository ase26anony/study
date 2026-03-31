#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays initialized with deterministic pseudo-random data */
#define ARRAY_SIZE 512
static int32_t global_ints[ARRAY_SIZE];
static float global_floats[ARRAY_SIZE];
static int32_t accumulator[ARRAY_SIZE] = {0};

/* Volatile variables to prevent constant folding */
volatile int volatile_mask_seed = 0;

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
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_ints[i] = (i * 1103515245 + seed) % 1000;
        global_floats[i] = (float)((i * 1103515245 + seed) % 1000) / 100.0f;
    }
}

/* Function that uses __builtin_shuffle with many operands */
#ifdef __AVX2__
v8si shuffle_int_avx2(v8si a, v8si b, v8si c, v8si d, volatile int mask_seed) {
    /* Create control mask from volatile input to prevent constant folding */
    v8si mask;
    for (int i = 0; i < 8; i++) {
        mask[i] = (mask_seed + i * 3) % 32;  /* Indices can reference any of the 4 vectors */
    }
    
    /* Complex control flow to stress expander */
    if (mask_seed & 1) {
        /* First shuffle pattern - uses 10 operands total */
        v8si temp1 = __builtin_shuffle(a, b, mask);
        v8si temp2 = __builtin_shuffle(c, d, mask);
        
        /* Nested shuffle with different mask */
        v8si mask2;
        for (int i = 0; i < 8; i++) {
            mask2[i] = (mask_seed + i * 5) % 32;
        }
        
        /* This shuffle uses 3 input vectors + mask = 4 operands */
        /* Combined with arithmetic, may require 10+ operand expansion */
        return __builtin_shuffle(temp1, temp2, mask2) + (a * b);
    } else {
        /* Alternative path with different shuffle pattern */
        v8si extended_mask;
        for (int i = 0; i < 8; i++) {
            extended_mask[i] = (mask_seed + i * 7) % 16;
        }
        
        /* Shuffle that might require conversion between vector sizes */
        v4si narrow_a = __builtin_convertvector(a, v4si);
        v4si narrow_b = __builtin_convertvector(b, v4si);
        v4si result_narrow = __builtin_shuffle(narrow_a, narrow_b, 
            (v4si){extended_mask[0] % 8, extended_mask[1] % 8, 
                   extended_mask[2] % 8, extended_mask[3] % 8});
        
        /* Convert back and combine */
        return __builtin_convertvector(result_narrow, v8si) + c;
    }
}
#endif

#ifdef __AVX512F__
/* Function specifically designed to require 11 operands during expansion */
v16si shuffle_int_avx512(v16si a, v16si b, v16si c, v16si d, 
                         volatile int mask_seed) {
    /* Create two different masks from volatile input */
    v16si mask1, mask2;
    for (int i = 0; i < 16; i++) {
        mask1[i] = (mask_seed + i * 11) % 64;
        mask2[i] = (mask_seed + i * 13) % 64;
    }
    
    /* Switch statement to create complex control flow */
    switch (mask_seed % 4) {
        case 0: {
            /* Pattern 0: Chain of shuffles */
            v16si temp1 = __builtin_shuffle(a, b, mask1);
            v16si temp2 = __builtin_shuffle(c, d, mask2);
            
            /* Create third mask dynamically */
            v16si mask3;
            for (int i = 0; i < 16; i++) {
                mask3[i] = (mask1[i] + mask2[i]) % 64;
            }
            
            /* This complex expression may require 11 operands:
               temp1, temp2, mask3, plus arithmetic operands */
            return __builtin_shuffle(temp1, temp2, mask3) + 
                   (a * b) - (c & d);
        }
        case 1: {
            /* Pattern 1: __builtin_shufflevector with many arguments */
            /* shufflevector can take many more arguments than shuffle */
            v16si result = __builtin_shufflevector(a, b, c, d,
                /* 16 indices - total of 20 arguments to builtin */
                mask1[0] % 64, mask1[1] % 64, mask1[2] % 64, mask1[3] % 64,
                mask1[4] % 64, mask1[5] % 64, mask1[6] % 64, mask1[7] % 64,
                mask2[0] % 64, mask2[1] % 64, mask2[2] % 64, mask2[3] % 64,
                mask2[4] % 64, mask2[5] % 64, mask2[6] % 64, mask2[7] % 64);
            
            /* Additional arithmetic to increase operand count */
            return result * a + b - c;
        }
        case 2: {
            /* Pattern 2: Mixed float/int operations */
            v16sf fa = __builtin_convertvector(a, v16sf);
            v16sf fb = __builtin_convertvector(b, v16sf);
            v16sf shuffled = __builtin_shuffle(fa, fb, mask1);
            
            /* Convert back and combine */
            return __builtin_convertvector(shuffled, v16si) + c;
        }
        default: {
            /* Pattern 3: Nested shuffles in loop */
            v16si result = a;
            for (int i = 0; i < 3; i++) {
                v16si dynamic_mask;
                for (int j = 0; j < 16; j++) {
                    dynamic_mask[j] = (mask_seed + i * j * 17) % 64;
                }
                result = __builtin_shuffle(result, b, dynamic_mask);
            }
            return result + d;
        }
    }
}
#endif

/* SSE2 version for baseline */
#ifdef __SSE2__
v4si shuffle_int_sse2(v4si a, v4si b, v4si c, v4si d, volatile int mask_seed) {
    v4si mask = {mask_seed % 4, (mask_seed + 1) % 4, 
                 (mask_seed + 2) % 4, (mask_seed + 3) % 4};
    
    /* Chain operations to increase complexity */
    v4si temp1 = __builtin_shuffle(a, b, mask);
    v4si temp2 = __builtin_shuffle(c, d, mask);
    
    v4si mask2 = {(mask_seed + 4) % 4, (mask_seed + 5) % 4,
                  (mask_seed + 6) % 4, (mask_seed + 7) % 4};
    
    return __builtin_shuffle(temp1, temp2, mask2);
}
#endif

/* Float version with double vectors */
#ifdef __AVX2__
v8df shuffle_double_avx2(v8df a, v8df b, volatile int mask_seed) {
    v8di mask;  /* Using integer mask for double shuffle */
    for (int i = 0; i < 8; i++) {
        mask[i] = (mask_seed + i * 9) % 16;
    }
    
    /* Conditional shuffle patterns */
    if (mask_seed & 2) {
        /* Complex expression that may expand to many operands */
        return __builtin_shuffle(a, b, mask) * 
               __builtin_shuffle(b, a, mask) + 
               (a - b);
    } else {
        /* Alternative: shufflevector with many indices */
        return __builtin_shufflevector(a, b,
            mask[0] % 8, mask[1] % 8, mask[2] % 8, mask[3] % 8,
            mask[4] % 8, mask[5] % 8, mask[6] % 8, mask[7] % 8);
    }
}
#endif

/* Main processing function */
void process_vectors(int iterations, int seed) {
    volatile_mask_seed = seed;
    
    for (int iter = 0; iter < iterations; iter++) {
        int base_idx = (iter * 16) % (ARRAY_SIZE - 64);
        
        /* Load data into vectors */
#ifdef __AVX512F__
        v16si avx512_vec1, avx512_vec2, avx512_vec3, avx512_vec4;
        for (int i = 0; i < 16; i++) {
            avx512_vec1[i] = global_ints[base_idx + i];
            avx512_vec2[i] = global_ints[base_idx + 16 + i];
            avx512_vec3[i] = global_ints[base_idx + 32 + i];
            avx512_vec4[i] = global_ints[base_idx + 48 + i];
        }
        
        /* Call function that may trigger 11-operand expansion */
        v16si result = shuffle_int_avx512(avx512_vec1, avx512_vec2, 
                                         avx512_vec3, avx512_vec4,
                                         volatile_mask_seed + iter);
        
        /* Store result to accumulator */
        for (int i = 0; i < 16; i++) {
            accumulator[base_idx + i] += result[i];
        }
        
        /* Volatile store to prevent optimization */
        volatile int volatile_dummy = result[0];
        (void)volatile_dummy;
#endif

#ifdef __AVX2__
        /* Process with AVX2 vectors */
        v8si avx2_vec1, avx2_vec2, avx2_vec3, avx2_vec4;
        for (int i = 0; i < 8; i++) {
            avx2_vec1[i] = global_ints[base_idx + i + 64];
            avx2_vec2[i] = global_ints[base_idx + i + 72];
            avx2_vec3[i] = global_ints[base_idx + i + 80];
            avx2_vec4[i] = global_ints[base_idx + i + 88];
        }
        
        v8si result_avx2 = shuffle_int_avx2(avx2_vec1, avx2_vec2,
                                           avx2_vec3, avx2_vec4,
                                           volatile_mask_seed + iter * 3);
        
        for (int i = 0; i < 8; i++) {
            accumulator[base_idx + i + 64] += result_avx2[i];
        }
        
        /* Process double vectors */
        v8df double_vec1, double_vec2;
        for (int i = 0; i < 8; i++) {
            double_vec1[i] = (double)global_floats[base_idx + i];
            double_vec2[i] = (double)global_floats[base_idx + i + 8];
        }
        
        v8df double_result = shuffle_double_avx2(double_vec1, double_vec2,
                                                volatile_mask_seed + iter * 5);
        
        /* Convert and accumulate */
        for (int i = 0; i < 8; i++) {
            accumulator[base_idx + i + 96] += (int32_t)double_result[i];
        }
#endif

#ifdef __SSE2__
        /* SSE2 fallback */
        v4si sse_vec1, sse_vec2, sse_vec3, sse_vec4;
        for (int i = 0; i < 4; i++) {
            sse_vec1[i] = global_ints[base_idx + i + 128];
            sse_vec2[i] = global_ints[base_idx + i + 132];
            sse_vec3[i] = global_ints[base_idx + i + 136];
            sse_vec4[i] = global_ints[base_idx + i + 140];
        }
        
        v4si sse_result = shuffle_int_sse2(sse_vec1, sse_vec2,
                                          sse_vec3, sse_vec4,
                                          volatile_mask_seed + iter * 7);
        
        for (int i = 0; i < 4; i++) {
            accumulator[base_idx + i + 128] += sse_result[i];
        }
#endif
    }
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Initializing with seed: %d\n", seed);
    init_arrays(seed);
    
    /* Process vectors with different patterns */
    process_vectors(10, seed);
    
    /* Compute checksum */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += accumulator[i];
    }
    
    printf("Final checksum: %lld\n", (long long)checksum);
    
    /* Additional verification: print first few values */
    printf("First 8 accumulator values: ");
    for (int i = 0; i < 8; i++) {
        printf("%d ", accumulator[i]);
    }
    printf("\n");
    
    return 0;
}
