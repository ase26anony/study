#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef float float32x8_t __attribute__((vector_size(32)));
typedef double float64x8_t __attribute__((vector_size(64)));
typedef double float64x4_t __attribute__((vector_size(32)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random data */
static void init_array(int32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = (int32_t)lcg_rand();
    }
}

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(const void *data, size_t size) {
    int64_t sum = 0;
    const uint8_t *bytes = (const uint8_t *)data;
    for (size_t i = 0; i < size; i++) {
        sum += bytes[i];
    }
    return sum;
}

int main(void) {
    /* Allocate and initialize data arrays */
    const size_t ARRAY_SIZE = 1024;
    int32_t *data_a = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *data_b = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *data_c = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *result = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    
    init_array(data_a, ARRAY_SIZE);
    init_array(data_b, ARRAY_SIZE);
    init_array(data_c, ARRAY_SIZE);
    memset(result, 0, ARRAY_SIZE * sizeof(int32_t));
    
    /* Volatile variable to prevent constant propagation */
    volatile int dynamic_mask_seed = lcg_rand() % 256;
    
    /* KERNEL 1: Complex shuffle with computed mask vector */
    for (int iter = 0; iter < 100; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 31; i += 16) {
            /* Load vectors */
            int32x16_t vec_a = *(int32x16_t*)(&data_a[i]);
            int32x16_t vec_b = *(int32x16_t*)(&data_b[i]);
            
            /* Compute dynamic mask based on volatile variable */
            int32_t mask_data[16];
            for (int j = 0; j < 16; j++) {
                mask_data[j] = (dynamic_mask_seed + j + iter) % 32;
            }
            int32x16_t mask_vec = *(int32x16_t*)mask_data;
            
            /* Complex shuffle requiring many operands during expansion */
            int32x16_t shuffled = __builtin_shuffle(vec_a, vec_b, mask_vec);
            
            /* Store result */
            *(int32x16_t*)(&result[i]) = shuffled;
        }
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    float32x16_t float_results[4];
    for (int iter = 0; iter < 50; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 63; i += 16) {
            /* Load and cast to different types */
            float32x16_t fvec_a = *(float32x16_t*)(&data_a[i]);
            float32x16_t fvec_b = *(float32x16_t*)(&data_b[i]);
            float32x16_t fvec_c = *(float32x16_t*)(&data_c[i]);
            
            /* Create complex mask using arithmetic */
            int32_t mask1_data[16];
            int32_t mask2_data[16];
            for (int j = 0; j < 16; j++) {
                mask1_data[j] = (j * 3 + dynamic_mask_seed) % 32;
                mask2_data[j] = (j * 5 + dynamic_mask_seed) % 32;
            }
            int32x16_t mask1 = *(int32x16_t*)mask1_data;
            int32x16_t mask2 = *(int32x16_t*)mask2_data;
            
            /* Chain of shuffles - each adds more operands */
            float32x16_t shuffle1 = __builtin_shuffle(fvec_a, fvec_b, mask1);
            float32x16_t shuffle2 = __builtin_shuffle(fvec_c, shuffle1, mask2);
            
            /* Another shuffle with the result */
            int32_t mask3_data[16];
            for (int j = 0; j < 16; j++) {
                mask3_data[j] = (j * 7 + iter) % 32;
            }
            int32x16_t mask3 = *(int32x16_t*)mask3_data;
            float32x16_t shuffle3 = __builtin_shuffle(shuffle2, fvec_a, mask3);
            
            /* Store in array for later use */
            float_results[iter % 4] = shuffle3;
        }
    }
    
    /* KERNEL 3: Conditional vector permutation */
    for (int iter = 0; iter < 30; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 31; i += 8) {
            /* Use smaller vectors to mix widths */
            int32x8_t vec_small_a = *(int32x8_t*)(&data_a[i]);
            int32x8_t vec_small_b = *(int32x8_t*)(&data_b[i]);
            
            /* Create masks with different patterns */
            int32_t mask_even[8] = {0, 2, 4, 6, 8, 10, 12, 14};
            int32_t mask_odd[8] = {1, 3, 5, 7, 9, 11, 13, 15};
            int32x8_t mask_even_vec = *(int32x8_t*)mask_even;
            int32x8_t mask_odd_vec = *(int32x8_t*)mask_odd;
            
            /* Conditional selection between two different shuffle results */
            int32x8_t shuffle_even = __builtin_shuffle(vec_small_a, vec_small_b, mask_even_vec);
            int32x8_t shuffle_odd = __builtin_shuffle(vec_small_a, vec_small_b, mask_odd_vec);
            
            /* Use conditional operator on vectors (creates VEC_COND_EXPR) */
            int32x8_t selector = (vec_small_a > vec_small_b);
            int32x8_t result_vec = selector ? shuffle_even : shuffle_odd;
            
            /* Store back */
            *(int32x8_t*)(&result[i]) = result_vec;
        }
    }
    
    /* KERNEL 4: Inline assembly with many vector operands */
#ifdef __x86_64__
    for (int iter = 0; iter < 10; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 15; i += 8) {
            int32x8_t va = *(int32x8_t*)(&data_a[i]);
            int32x8_t vb = *(int32x8_t*)(&data_b[i]);
            int32x8_t vc = *(int32x8_t*)(&data_c[i]);
            int32x8_t vresult;
            
            /* Inline assembly with multiple vector operands */
            asm volatile (
                /* Hypothetical multi-operand vector operation */
                "vpaddd %0, %1, %2\n\t"
                "vpshufd %0, %0, %3\n\t"
                : "=x"(vresult)
                : "x"(va), "x"(vb), "i"(0x1B)  /* 0x1B = shuffle mask */
                : "memory"
            );
            
            /* Use the result */
            vresult = vresult + vc;
            *(int32x8_t*)(&result[i]) = vresult;
        }
    }
#endif
    
    /* Architecture-specific builtins when available */
#if defined(__AVX2__)
    for (int iter = 0; iter < 5; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 7; i += 8) {
            int32x8_t vec = *(int32x8_t*)(&data_a[i]);
            /* Use AVX2 permutation builtin */
            int32x8_t permuted = __builtin_ia32_pshufd256(vec, 0x1B);
            *(int32x8_t*)(&result[i]) = permuted;
        }
    }
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
    for (int iter = 0; iter < 5; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE - 3; i += 4) {
            int32x4_t vec = *(int32x4_t*)(&data_a[i]);
            /* Use NEON reversal builtin */
            int32x4_t reversed = __builtin_neon_vrev64q_s32(vec);
            *(int32x4_t*)(&result[i]) = reversed;
        }
    }
#endif
    
    /* Final checksum to prevent optimization */
    int64_t checksum = compute_checksum(result, ARRAY_SIZE * sizeof(int32_t));
    printf("Result checksum: %ld\n", (long)checksum);
    
    /* Cleanup */
    free(data_a);
    free(data_b);
    free(data_c);
    free(result);
    
    return 0;
}
