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

/* Initialize array with pseudo-random values */
static void init_array(int32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = (int32_t)lcg_rand();
    }
}

/* Compute shuffle mask dynamically to prevent constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Volatile read to prevent optimization */
    volatile int base = control;
    
    for (int i = 0; i < 16; i++) {
        /* Complex, non-constant mask calculation */
        mask_data[i] = (base + i * 3) % 32;
        mask_data[i] = mask_data[i] < 0 ? mask_data[i] + 32 : mask_data[i];
        mask_data[i] ^= (i & 1) ? 16 : 0;
    }
    
    return mask;
}

/* Another mask computation with different pattern */
static int32x8_t compute_mixed_mask(volatile int seed) {
    int32x8_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    for (int i = 0; i < 8; i++) {
        mask_data[i] = (seed + i * 5) % 16;
        mask_data[i] |= (i << 8);
    }
    
    return mask;
}

int main(void) {
    /* Large arrays to hold vector data */
    int32_t data_a[256];
    int32_t data_b[256];
    int32_t data_c[256];
    float float_data[256];
    
    /* Initialize with pseudo-random values */
    init_array(data_a, 256);
    init_array(data_b, 256);
    init_array(data_c, 256);
    
    for (int i = 0; i < 256; i++) {
        float_data[i] = (float)lcg_rand() / 1000.0f;
    }
    
    /* Volatile control variables to prevent compile-time evaluation */
    volatile int control1 = 7;
    volatile int control2 = 13;
    volatile int control3 = 42;
    
    /* Result storage */
    int32x16_t results[4];
    float32x16_t float_results[2];
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 1: Complex shuffle with computed mask
     * This should generate many operands during RTL expansion
     ******************************************************************/
    {
        /* Cast array segments to vectors */
        int32x16_t *vec_a = (int32x16_t*)&data_a[0];
        int32x16_t *vec_b = (int32x16_t*)&data_a[16]; /* Different region */
        int32x16_t *vec_c = (int32x16_t*)&data_b[0];
        
        /* Compute dynamic mask */
        int32x16_t mask = compute_complex_mask(control1);
        
        /* Complex shuffle operation - cannot be simplified at compile time */
        results[0] = __builtin_shuffle(*vec_a, *vec_b, mask);
        
        /* Chain another shuffle with different mask */
        int32x16_t mask2 = compute_complex_mask(control2);
        results[1] = __builtin_shuffle(results[0], *vec_c, mask2);
        
        /* Mix with arithmetic operation */
        results[0] = results[0] + results[1];
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 2: Chain of shuffles with mixed types
     * Increasing operand count through chaining
     ******************************************************************/
    {
        /* Use different vector sizes */
        int32x8_t *vec_small_a = (int32x8_t*)&data_a[32];
        int32x8_t *vec_small_b = (int32x8_t*)&data_a[40];
        int32x8_t *vec_small_c = (int32x8_t*)&data_b[32];
        
        /* Compute mask for 8-element vectors */
        int32x8_t mask8 = compute_mixed_mask(control3);
        
        /* Chain multiple shuffle operations */
        int32x8_t temp1 = __builtin_shuffle(*vec_small_a, *vec_small_b, mask8);
        int32x8_t temp2 = __builtin_shuffle(temp1, *vec_small_c, mask8);
        int32x8_t temp3 = __builtin_shuffle(*vec_small_c, temp2, mask8);
        
        /* Expand to larger vector */
        int32x16_t expanded = __builtin_shufflevector(temp2, temp3, 
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
        
        /* Store result */
        memcpy(&results[2], &expanded, sizeof(expanded));
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 3: Conditional vector permutation with mixed float/int
     ******************************************************************/
    {
        /* Float vectors */
        float32x16_t *fvec_a = (float32x16_t*)&float_data[0];
        float32x16_t *fvec_b = (float32x16_t*)&float_data[16];
        
        /* Create integer mask from float comparison */
        int32x16_t cmp_mask;
        float *fa = (float*)fvec_a;
        float *fb = (float*)fvec_b;
        int32_t *mask_data = (int32_t*)&cmp_mask;
        
        for (int i = 0; i < 16; i++) {
            mask_data[i] = (fa[i] > fb[i]) ? i : (15 - i);
        }
        
        /* Conditional shuffle based on comparison */
        float32x16_t shuffle1 = __builtin_shuffle(*fvec_a, *fvec_b, cmp_mask);
        
        /* Another mask with different pattern */
        for (int i = 0; i < 16; i++) {
            mask_data[i] = (fa[i] < fb[i]) ? (i + 8) % 16 : (i * 3) % 16;
        }
        
        float32x16_t shuffle2 = __builtin_shuffle(*fvec_b, *fvec_a, cmp_mask);
        
        /* Select between two shuffle results based on control */
        float_results[0] = (control1 > 10) ? shuffle1 : shuffle2;
        
        /* Mix with integer results */
        int32x16_t *int_view = (int32x16_t*)&float_results[0];
        results[3] = results[2] + *int_view;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 4: Inline assembly with many vector operands
     * Targeting the 10-11 operand expansion path
     ******************************************************************/
    {
        /* Prepare vectors for assembly */
        int32x16_t vec1 = results[0];
        int32x16_t vec2 = results[1];
        int32x16_t vec3 = results[2];
        int32x16_t vec4 = results[3];
        int32x16_t out1, out2;
        
        /* Inline assembly with many vector operands */
        asm volatile(
            /* Hypothetical multi-operand vector operation */
            "# Multi-operand vector operation\n\t"
            "vmovdqa %[v1], %%ymm0\n\t"
            "vmovdqa %[v2], %%ymm1\n\t"
            "vmovdqa %[v3], %%ymm2\n\t"
            "vmovdqa %[v4], %%ymm3\n\t"
            /* Complex sequence using many operands */
            "vpaddd %%ymm0, %%ymm1, %%ymm4\n\t"
            "vpaddd %%ymm2, %%ymm3, %%ymm5\n\t"
            "vpshufd $0x1B, %%ymm4, %%ymm6\n\t"
            "vpshufd $0x39, %%ymm5, %%ymm7\n\t"
            "vpblendd $0xF0, %%ymm6, %%ymm7, %%ymm0\n\t"
            "vmovdqa %%ymm0, %[o1]\n\t"
            "vmovdqa %%ymm7, %[o2]"
            : [o1] "=v" (out1), [o2] "=v" (out2)
            : [v1] "v" (vec1), [v2] "v" (vec2), 
              [v3] "v" (vec3), [v4] "v" (vec4)
            : "ymm0", "ymm1", "ymm2", "ymm3", 
              "ymm4", "ymm5", "ymm6", "ymm7", "memory"
        );
        
        results[0] = out1;
        results[1] = out2;
    }
    
    /******************************************************************
     * Target-specific builtins (conditional compilation)
     ******************************************************************/
#ifdef __x86_64__
    {
        /* Use x86-specific shuffle intrinsics */
        int32x4_t xmm_vec1 = {1, 2, 3, 4};
        int32x4_t xmm_vec2 = {5, 6, 7, 8};
        int32x4_t xmm_vec3 = {9, 10, 11, 12};
        
        /* These builtins often expand to multi-operand patterns */
        int32x4_t shuffled = __builtin_ia32_pshufd(xmm_vec1, 0x1B);
        shuffled = __builtin_ia32_pblendw128(shuffled, xmm_vec2, 0xF0);
        
        /* Chain multiple operations */
        for (int i = 0; i < 4; i++) {
            shuffled = __builtin_ia32_pshufd(shuffled, (i * 0x55) & 0xFF);
        }
    }
#endif
    
#ifdef __aarch64__
    {
        /* ARM Neon builtins */
        int32x4_t neon_vec1 = {1, 2, 3, 4};
        int32x4_t neon_vec2 = {5, 6, 7, 8};
        
        /* Complex sequence of Neon operations */
        int32x4_t rev = __builtin_neon_vrev64q_s32(neon_vec1);
        rev = __builtin_neon_vzip1q_s32(rev, neon_vec2);
        rev = __builtin_neon_vzip2q_s32(rev, neon_vec2);
    }
#endif
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * Loop-dependent vector operations
     * Prevents optimization into simpler forms
     ******************************************************************/
    {
        int32x8_t accum = {0};
        volatile int dynamic_control = 100;
        
        for (int iter = 0; iter < 8; iter++) {
            /* Data-dependent mask calculation */
            int32x8_t loop_mask = compute_mixed_mask(dynamic_control + iter);
            
            /* Source vectors from different parts of array */
            int32x8_t *src1 = (int32x8_t*)&data_a[iter * 8];
            int32x8_t *src2 = (int32x8_t*)&data_b[iter * 8];
            
            /* Shuffle that depends on loop iteration */
            int32x8_t shuffled = __builtin_shuffle(*src1, *src2, loop_mask);
            
            /* Accumulate results */
            accum = accum + shuffled;
            
            /* Modify control for next iteration */
            dynamic_control = (dynamic_control * 13 + 17) & 0xFF;
        }
        
        /* Use the accumulated result */
        int32x16_t final = __builtin_shufflevector(accum, accum, 
            0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
        results[3] = results[3] + final;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        int32_t *data = (int32_t*)&results[i];
        for (int j = 0; j < 16; j++) {
            checksum += data[j];
        }
    }
    
    printf("Checksum: %ld\n", (long)checksum);
    
    return 0;
}
