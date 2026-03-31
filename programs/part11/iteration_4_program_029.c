#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int32_t int32x4_t __attribute__((vector_size(16)));
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
        arr[i] = (int32_t)(lcg_rand() % 1000);
    }
}

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(int32_t *arr, size_t size) {
    int64_t sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(void) {
    /* Allocate aligned memory for vector operations */
    const size_t ARRAY_SIZE = 1024;
    int32_t *data_a = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *data_b = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *data_c = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *result = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    
    if (!data_a || !data_b || !data_c || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_array(data_a, ARRAY_SIZE);
    init_array(data_b, ARRAY_SIZE);
    init_array(data_c, ARRAY_SIZE);
    memset(result, 0, ARRAY_SIZE * sizeof(int32_t));
    
    /* Volatile variables to prevent constant propagation */
    volatile int vmask_seed = 42;
    volatile int vmask_offset = 7;
    
    /* KERNEL 1: Complex shuffle with computed mask vector */
    for (int iter = 0; iter < 10; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE; i += 16) {
            /* Load vectors */
            int32x16_t vec_a = *(int32x16_t*)(&data_a[i]);
            int32x16_t vec_b = *(int32x16_t*)(&data_b[i]);
            
            /* Compute dynamic mask based on volatile variables */
            int32_t mask_data[16];
            for (int j = 0; j < 16; j++) {
                /* Complex mask calculation preventing compile-time evaluation */
                mask_data[j] = (j * vmask_seed + vmask_offset + iter) % 32;
            }
            int32x16_t mask_vec = *(int32x16_t*)mask_data;
            
            /* Complex shuffle operation - may require many operands during expansion */
            int32x16_t shuffled = __builtin_shuffle(vec_a, vec_b, mask_vec);
            
            /* Store result */
            *(int32x16_t*)(&result[i]) = shuffled;
        }
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    for (int iter = 0; iter < 5; iter++) {
        for (size_t i = 0; i < ARRAY_SIZE; i += 8) {
            /* Load multiple vectors */
            int32x8_t v1 = *(int32x8_t*)(&data_a[i]);
            int32x8_t v2 = *(int32x8_t*)(&data_b[i]);
            int32x8_t v3 = *(int32x8_t*)(&data_c[i]);
            
            /* Create complex mask patterns */
            int32_t mask1[8] = {1, 0, 3, 2, 5, 4, 7, 6};
            int32_t mask2[8] = {2, 3, 0, 1, 6, 7, 4, 5};
            int32_t mask3[8];
            for (int j = 0; j < 8; j++) {
                mask3[j] = (mask1[j] + mask2[j] + vmask_offset) % 8;
            }
            
            /* Chain of shuffle operations - each may add to operand count */
            int32x8_t s1 = __builtin_shuffle(v1, v2, *(int32x8_t*)mask1);
            int32x8_t s2 = __builtin_shuffle(s1, v3, *(int32x8_t*)mask2);
            int32x8_t s3 = __builtin_shuffle(s2, v1, *(int32x8_t*)mask3);
            
            /* Mix with float vectors */
            float32x8_t fv1 = __builtin_convertvector(v1, float32x8_t);
            float32x8_t fv2 = __builtin_convertvector(s3, float32x8_t);
            
            /* Shuffle with float vectors */
            int32_t fmask[8] = {7, 6, 5, 4, 3, 2, 1, 0};
            float32x8_t fs1 = __builtin_shuffle(fv1, fv2, *(int32x8_t*)fmask);
            
            /* Convert back and store */
            int32x8_t final_vec = __builtin_convertvector(fs1, int32x8_t);
            *(int32x8_t*)(&result[i]) = final_vec;
        }
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 3: Conditional vector permutation */
    for (int iter = 0; iter < 3; iter++) {
        volatile int selector = iter % 3;
        
        for (size_t i = 0; i < ARRAY_SIZE; i += 16) {
            int32x16_t vec1 = *(int32x16_t*)(&data_a[i]);
            int32x16_t vec2 = *(int32x16_t*)(&data_b[i]);
            int32x16_t vec3 = *(int32x16_t*)(&data_c[i]);
            
            /* Create two different mask patterns */
            int32_t mask_a[16], mask_b[16];
            for (int j = 0; j < 16; j++) {
                mask_a[j] = (j + selector) % 32;
                mask_b[j] = (j * 2 + vmask_offset) % 32;
            }
            
            /* Conditional selection between two different shuffle results */
            int32x16_t shuffle_a = __builtin_shuffle(vec1, vec2, *(int32x16_t*)mask_a);
            int32x16_t shuffle_b = __builtin_shuffle(vec2, vec3, *(int32x16_t*)mask_b);
            
            int32x16_t selected = (selector > 0) ? shuffle_a : shuffle_b;
            
            /* Additional shuffle with the selected result */
            int32_t final_mask[16];
            for (int j = 0; j < 16; j++) {
                final_mask[j] = (mask_a[j] + mask_b[j]) % 32;
            }
            int32x16_t final_result = __builtin_shuffle(selected, vec1, *(int32x16_t*)final_mask);
            
            *(int32x16_t*)(&result[i]) = final_result;
        }
    }
    
    /* KERNEL 4: Inline assembly with many vector operands */
#ifdef __x86_64__
    for (size_t i = 0; i < ARRAY_SIZE; i += 8) {
        int32x8_t v1 = *(int32x8_t*)(&data_a[i]);
        int32x8_t v2 = *(int32x8_t*)(&data_b[i]);
        int32x8_t v3 = *(int32x8_t*)(&data_c[i]);
        int32x8_t v4 = *(int32x8_t*)(&result[i]);
        
        /* Inline assembly with multiple vector operands */
        asm volatile (
            /* Hypothetical multi-operand vector operation */
            "vpaddd %0, %1, %2\n\t"
            "vpsubd %0, %3, %4\n\t"
            "vpmulld %0, %1, %5\n\t"
            : "+x" (v1), "+x" (v2), "+x" (v3), "+x" (v4)
            : 
            : "memory"
        );
        
        /* Use architecture-specific builtins when available */
#if defined(__SSE4_2__) || defined(__AVX2__)
        /* These builtins may map to complex multi-operand instructions */
        int32x8_t shuffled = __builtin_ia32_pshufd256(v1, 0x1B);
        int32x8_t blended = __builtin_ia32_pblendd256(v2, v3, 0xF0);
#endif
        
        *(int32x8_t*)(&result[i]) = v1;
    }
#endif
    
#ifdef __ARM_NEON
    /* ARM-specific vector operations */
    for (size_t i = 0; i < ARRAY_SIZE; i += 4) {
        int32x4_t v1 = *(int32x4_t*)(&data_a[i]);
        int32x4_t v2 = *(int32x4_t*)(&data_b[i]);
        
        /* ARM NEON builtins */
        int32x4_t rev = __builtin_neon_vrev64q_s32(v1);
        int32x4_t ext = __builtin_neon_vextq_s32(v1, v2, 2);
        
        *(int32x4_t*)(&result[i]) = rev;
    }
#endif
    
    /* Final checksum to prevent optimization */
    int64_t checksum = compute_checksum(result, ARRAY_SIZE);
    printf("Checksum: %ld\n", (long)checksum);
    
    /* Cleanup */
    free(data_a);
    free(data_b);
    free(data_c);
    free(result);
    
    return 0;
}
