#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random data */
static void init_array(int32_t *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = (int32_t)lcg_rand();
    }
}

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(int32_t *arr, size_t n) {
    int64_t sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(void) {
    /* Allocate aligned memory for vector operations */
    const size_t ARRAY_SIZE = 256;
    int32_t *data = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    int32_t *result = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    
    if (!data || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_array(data, ARRAY_SIZE);
    memset(result, 0, ARRAY_SIZE * sizeof(int32_t));
    
    /* Volatile variable to prevent constant propagation */
    volatile int control = lcg_rand() % 256;
    
    /* KERNEL 1: Complex shuffle with computed mask vector */
    for (int iter = 0; iter < 100; iter++) {
        /* Load vectors from data */
        int32x16_t vec_a = *(int32x16_t*)(data + iter * 16);
        int32x16_t vec_b = *(int32x16_t*)(data + (iter + 1) * 16);
        
        /* Compute dynamic mask based on control variable */
        int32x16_t mask;
        for (int i = 0; i < 16; i++) {
            mask[i] = (control + i * 3) % 32;
        }
        
        /* Complex shuffle operation that may require many operands */
        int32x16_t shuffled = __builtin_shuffle(vec_a, vec_b, mask);
        
        /* Store result */
        *(int32x16_t*)(result + iter * 16) = shuffled;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    {
        float32x16_t fvec_a = *(float32x16_t*)(data);
        float32x16_t fvec_b = *(float32x16_t*)(data + 16);
        float32x16_t fvec_c = *(float32x16_t*)(data + 32);
        
        /* Create complex mask using arithmetic */
        int32x16_t mask1, mask2;
        for (int i = 0; i < 16; i++) {
            mask1[i] = (i * 5 + control) % 48;
            mask2[i] = (i * 7 + control) % 48;
        }
        
        /* Chain multiple shuffle operations */
        float32x16_t temp1 = __builtin_shufflevector(fvec_a, fvec_b, 
            0, 17, 2, 19, 4, 21, 6, 23, 8, 25, 10, 27, 12, 29, 14, 31);
        
        float32x16_t temp2 = __builtin_shufflevector(fvec_b, fvec_c,
            16, 1, 18, 3, 20, 5, 22, 7, 24, 9, 26, 11, 28, 13, 30, 15);
        
        /* Final shuffle combining previous results */
        float32x16_t final_result = __builtin_shuffle(temp1, temp2, mask1);
        
        /* Store and use asm barrier */
        *(float32x16_t*)(result) = final_result;
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 3: Conditional vector permutation */
    {
        int64x8_t vec64_a = *(int64x8_t*)(data);
        int64x8_t vec64_b = *(int64x8_t*)(data + 16);
        
        /* Dynamic condition */
        int use_alt_mask = (control & 0x1);
        
        /* Two different mask patterns */
        int64x8_t mask_alt1 = {0, 9, 2, 11, 4, 13, 6, 15};
        int64x8_t mask_alt2 = {8, 1, 10, 3, 12, 5, 14, 7};
        
        /* Conditional selection of shuffle result */
        int64x8_t shuffled_result = use_alt_mask ?
            __builtin_shuffle(vec64_a, vec64_b, mask_alt1) :
            __builtin_shuffle(vec64_a, vec64_b, mask_alt2);
        
        *(int64x8_t*)(result + 8) = shuffled_result;
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 4: Inline assembly with many vector operands */
#ifdef __x86_64__
    {
        int32x8_t v1 = *(int32x8_t*)(data);
        int32x8_t v2 = *(int32x8_t*)(data + 8);
        int32x8_t v3 = *(int32x8_t*)(data + 16);
        int32x8_t v4 = *(int32x8_t*)(data + 24);
        int32x8_t v5, v6, v7, v8;
        
        /* Inline asm with many vector operands */
        asm volatile (
            "vmovdqa %[in1], %%ymm0\n\t"
            "vmovdqa %[in2], %%ymm1\n\t"
            "vmovdqa %[in3], %%ymm2\n\t"
            "vmovdqa %[in4], %%ymm3\n\t"
            "vpermq $0x1B, %%ymm0, %%ymm4\n\t"
            "vpermq $0x4E, %%ymm1, %%ymm5\n\t"
            "vpblendd $0xF0, %%ymm4, %%ymm5, %%ymm6\n\t"
            "vpshufd $0x1B, %%ymm6, %%ymm7\n\t"
            "vmovdqa %%ymm7, %[out1]\n\t"
            "vmovdqa %%ymm6, %[out2]\n\t"
            "vmovdqa %%ymm4, %[out3]\n\t"
            "vmovdqa %%ymm5, %[out4]\n\t"
            : [out1] "=v" (v5), [out2] "=v" (v6), 
              [out3] "=v" (v7), [out4] "=v" (v8)
            : [in1] "v" (v1), [in2] "v" (v2), 
              [in3] "v" (v3), [in4] "v" (v4)
            : "ymm0", "ymm1", "ymm2", "ymm3", 
              "ymm4", "ymm5", "ymm6", "ymm7", "memory"
        );
        
        /* Store results */
        *(int32x8_t*)(result + 32) = v5;
        *(int32x8_t*)(result + 40) = v6;
        asm volatile("" ::: "memory");
    }
#endif
    
    /* Target-specific builtins for different architectures */
#ifdef __ARM_NEON
    {
        int32x4_t neon_vec1 = *(int32x4_t*)(data);
        int32x4_t neon_vec2 = *(int32x4_t*)(data + 4);
        
        /* Use NEON-specific builtins */
        int32x4_t rev1 = __builtin_neon_vrev64q_s32(neon_vec1);
        int32x4_t rev2 = __builtin_neon_vrev64q_s32(neon_vec2);
        
        /* Complex permutation */
        int32x4_t combined = __builtin_shuffle(rev1, rev2, 
            (int32x4_t){4, 1, 6, 3});
        
        *(int32x4_t*)(result + 48) = combined;
        asm volatile("" ::: "memory");
    }
#endif
    
    /* Final checksum computation */
    int64_t checksum = compute_checksum(result, ARRAY_SIZE);
    printf("Checksum: %ld\n", (long)checksum);
    
    /* Cleanup */
    free(data);
    free(result);
    
    return 0;
}
