#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x8_t __attribute__((vector_size(32)));
typedef float float32x4_t __attribute__((vector_size(16)));
typedef double float64x4_t __attribute__((vector_size(32)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Complex mask computation that prevents constant propagation */
static inline int32x16_t compute_complex_mask(int iteration) {
    volatile int seed = iteration;  /* volatile prevents optimization */
    int32x16_t mask = {0};
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask[i] = (seed + i * 3) % 32;
        if (mask[i] < 0) mask[i] += 32;
    }
    
    return mask;
}

/* Another mask with different pattern */
static inline int32x16_t compute_alternate_mask(int iteration) {
    volatile int base = iteration * 7;
    int32x16_t mask = {0};
    
    for (int i = 0; i < 16; i++) {
        mask[i] = (base + i * 5) % 32;
        mask[i] = mask[i] < 16 ? mask[i] + 16 : mask[i] - 16;
    }
    
    return mask;
}

int main(void) {
    /* Initialize large arrays with pseudo-random data */
    #define ARRAY_SIZE 1024
    int32_t data_a[ARRAY_SIZE];
    int32_t data_b[ARRAY_SIZE];
    int32_t data_c[ARRAY_SIZE];
    float float_data[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data_a[i] = (int32_t)lcg_rand();
        data_b[i] = (int32_t)lcg_rand();
        data_c[i] = (int32_t)lcg_rand();
        float_data[i] = (float)lcg_rand() / 1000.0f;
    }
    
    /* Result arrays */
    int32_t result1[ARRAY_SIZE] = {0};
    int32_t result2[ARRAY_SIZE] = {0};
    float float_result[ARRAY_SIZE] = {0};
    
    /* Loop with data-dependent vector operations */
    for (int iter = 0; iter < 100; iter++) {
        /* KERNEL 1: Complex shuffle with computed mask */
        for (int i = 0; i < ARRAY_SIZE - 31; i += 16) {
            /* Load vectors from different arrays */
            int32x16_t vec_a = *(int32x16_t *)&data_a[i];
            int32x16_t vec_b = *(int32x16_t *)&data_b[i + 16];
            
            /* Compute mask that depends on iteration */
            int32x16_t mask = compute_complex_mask(iter + i);
            
            /* Complex shuffle that may require many operands during expansion */
            int32x16_t shuffled = __builtin_shuffle(vec_a, vec_b, mask);
            
            /* Store result */
            *(int32x16_t *)&result1[i] = shuffled;
        }
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* KERNEL 2: Chain of shuffles accumulating operand count */
        for (int i = 0; i < ARRAY_SIZE - 47; i += 16) {
            /* Load multiple vectors */
            int32x16_t v1 = *(int32x16_t *)&data_a[i];
            int32x16_t v2 = *(int32x16_t *)&data_b[i];
            int32x16_t v3 = *(int32x16_t *)&data_c[i];
            int32x16_t v4 = *(int32x16_t *)&data_a[i + 16];
            
            /* First shuffle */
            int32x16_t mask1 = compute_complex_mask(iter * 2 + i);
            int32x16_t s1 = __builtin_shuffle(v1, v2, mask1);
            
            /* Second shuffle using result of first */
            int32x16_t mask2 = compute_alternate_mask(iter * 3 + i);
            int32x16_t s2 = __builtin_shuffle(s1, v3, mask2);
            
            /* Third shuffle with all vectors involved */
            int32x16_t mask3;
            for (int j = 0; j < 16; j++) {
                mask3[j] = (iter + i + j * 7) % 64;
            }
            
            /* This complex operation may require many operands */
            int32x16_t final_result = __builtin_shufflevector(s2, v4, 
                0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23);
            
            /* Store */
            *(int32x16_t *)&result2[i] = final_result;
        }
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* KERNEL 3: Conditional vector permutation */
        for (int i = 0; i < ARRAY_SIZE - 31; i += 16) {
            int32x16_t vec1 = *(int32x16_t *)&data_a[i];
            int32x16_t vec2 = *(int32x16_t *)&data_b[i];
            
            /* Compute two different masks */
            int32x16_t mask_a = compute_complex_mask(iter + i);
            int32x16_t mask_b = compute_alternate_mask(iter + i);
            
            /* Create two different shuffle results */
            int32x16_t shuffle_a = __builtin_shuffle(vec1, vec2, mask_a);
            int32x16_t shuffle_b = __builtin_shuffle(vec2, vec1, mask_b);
            
            /* Conditional selection between shuffle results */
            int32x16_t selector;
            for (int j = 0; j < 16; j++) {
                selector[j] = (data_a[i + j] & 1) ? -1 : 0;
            }
            
            /* This conditional operation creates complex data flow */
            int32x16_t selected = selector ? shuffle_a : shuffle_b;
            
            /* Mix with another operation */
            int32x16_t final_vec = __builtin_shufflevector(
                selected, vec1, 0, 16, 2, 18, 4, 20, 6, 22, 8, 24, 10, 26, 12, 28, 14, 30);
            
            *(int32x16_t *)&result1[i] = final_vec;
        }
        
        /* KERNEL 4: Mixed vector types and widths */
        for (int i = 0; i < ARRAY_SIZE - 15; i += 8) {
            /* Mix float and int vectors */
            float32x8_t fvec1 = *(float32x8_t *)&float_data[i];
            float32x8_t fvec2 = *(float32x8_t *)&float_data[i + 8];
            
            /* Convert to int for shuffle, then back */
            int32x8_t ivec1 = *(int32x8_t *)&float_data[i];
            int32x8_t ivec2 = *(int32x8_t *)&float_data[i + 8];
            
            /* Complex shuffle pattern */
            int32x8_t mask;
            for (int j = 0; j < 8; j++) {
                mask[j] = (i + iter * 11 + j * 13) % 16;
            }
            
            int32x8_t shuffled_int = __builtin_shuffle(ivec1, ivec2, mask);
            
            /* Convert back to float */
            float32x8_t result_float = *(float32x8_t *)&shuffled_int;
            
            /* Store */
            *(float32x8_t *)&float_result[i] = result_float;
        }
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Inline assembly with many vector operands */
        #ifdef __x86_64__
        for (int i = 0; i < ARRAY_SIZE - 15; i += 8) {
            int32x8_t v1 = *(int32x8_t *)&data_a[i];
            int32x8_t v2 = *(int32x8_t *)&data_b[i];
            int32x8_t v3 = *(int32x8_t *)&data_c[i];
            int32x8_t v4 = *(int32x8_t *)&data_a[i + 8];
            
            /* Inline asm with multiple vector operands */
            asm volatile (
                /* Hypothetical multi-operand vector operation */
                "vpaddd %0, %1, %2\n\t"
                "vpshufd %3, %0, 0x1B\n\t"
                : "=x"(v1), "+x"(v2), "+x"(v3)
                : "x"(v4)
                : "memory"
            );
            
            /* Use the results */
            int32x8_t combined = v1 + v2 + v3;
            *(int32x8_t *)&result2[i] = combined;
        }
        #endif
        
        /* ARM-specific builtins */
        #ifdef __ARM_NEON
        for (int i = 0; i < ARRAY_SIZE - 15; i += 4) {
            int32x4_t v1 = *(int32x4_t *)&data_a[i];
            int32x4_t v2 = *(int32x4_t *)&data_b[i];
            
            /* Use ARM NEON builtin */
            int32x4_t rev = __builtin_neon_vrev64q_s32(v1);
            int32x4_t combined = rev + v2;
            
            *(int32x4_t *)&result2[i] = combined;
        }
        #endif
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += result1[i];
        checksum += result2[i];
        checksum += (int64_t)float_result[i];
    }
    
    printf("Checksum: %ld\n", (long)checksum);
    
    return 0;
}
