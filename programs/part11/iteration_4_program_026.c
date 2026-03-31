#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x8_t __attribute__((vector_size(32)));
typedef double float64x4_t __attribute__((vector_size(32)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random values */
static void init_array(int32_t *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = (int32_t)(lcg_rand() % 1000);
    }
}

/* Horizontal sum of vector elements */
static int64_t horizontal_sum_i32x16(int32x16_t v) {
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += v[i];
    }
    return sum;
}

static double horizontal_sum_f64x4(float64x4_t v) {
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += v[i];
    }
    return sum;
}

int main(void) {
    /* Large arrays to hold vector data */
    int32_t data_a[256];
    int32_t data_b[256];
    float double_data[256];
    
    /* Initialize with pseudo-random values */
    init_array(data_a, 256);
    init_array(data_b, 256);
    for (int i = 0; i < 256; i++) {
        double_data[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    /* Volatile variables to prevent constant propagation */
    volatile int vmask_seed = 42;
    volatile int vmask_offset = 7;
    
    int64_t total_checksum = 0;
    
    /* KERNEL 1: Complex shuffle with computed mask vector */
    {
        /* Load vectors from memory */
        int32x16_t vec_a = *(int32x16_t *)&data_a[0];
        int32x16_t vec_b = *(int32x16_t *)&data_b[16];
        
        /* Compute a non-constant mask vector using volatile variables */
        int32x16_t mask_vec;
        for (int i = 0; i < 16; i++) {
            mask_vec[i] = (i * vmask_seed + vmask_offset) % 32;
        }
        
        /* Complex shuffle that may require many operands during expansion */
        int32x16_t result1 = __builtin_shuffle(vec_a, vec_b, mask_vec);
        
        /* Store and accumulate checksum */
        total_checksum += horizontal_sum_i32x16(result1);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    {
        int32x8_t vec1 = *(int32x8_t *)&data_a[32];
        int32x8_t vec2 = *(int32x8_t *)&data_b[48];
        int32x8_t vec3 = *(int32x8_t *)&data_a[64];
        int32x8_t vec4 = *(int32x8_t *)&data_b[80];
        
        /* Create complex mask using arithmetic */
        int32x8_t mask1, mask2;
        for (int i = 0; i < 8; i++) {
            mask1[i] = (i + vmask_seed) % 16;
            mask2[i] = (i * 3 + vmask_offset) % 16;
        }
        
        /* Chain shuffles - output of one feeds into another */
        int32x8_t intermed = __builtin_shuffle(vec1, vec2, mask1);
        int32x8_t result2 = __builtin_shuffle(intermed, vec3, mask2);
        
        /* Another shuffle with the result */
        int32x8_t mask3;
        for (int i = 0; i < 8; i++) {
            mask3[i] = (i * 5 + vmask_seed) % 16;
        }
        int32x8_t final_result = __builtin_shuffle(result2, vec4, mask3);
        
        for (int i = 0; i < 8; i++) {
            total_checksum += final_result[i];
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 3: Conditional vector permutation */
    {
        float32x8_t fvec1 = *(float32x8_t *)&double_data[0];
        float32x8_t fvec2 = *(float32x8_t *)&double_data[8];
        float32x8_t fvec3 = *(float32x8_t *)&double_data[16];
        
        /* Create masks with data-dependent computation */
        int32x8_t mask_a, mask_b;
        for (int i = 0; i < 8; i++) {
            mask_a[i] = (data_a[i] % 16);
            mask_b[i] = (data_b[i] % 16);
        }
        
        /* Conditional selection between two different shuffle results */
        float32x8_t shuffle_a = __builtin_shuffle(fvec1, fvec2, mask_a);
        float32x8_t shuffle_b = __builtin_shuffle(fvec1, fvec3, mask_b);
        
        /* Use conditional operator on entire vectors */
        int32x8_t cond_mask;
        for (int i = 0; i < 8; i++) {
            cond_mask[i] = (data_a[i] > data_b[i]) ? -1 : 0;
        }
        
        /* This creates a blend operation that may expand to many operands */
        float32x8_t result3 = __builtin_shufflevector(shuffle_a, shuffle_b, 
            0, 1, 2, 3, 12, 13, 14, 15);
        
        /* Convert to double and accumulate checksum */
        float64x4_t dvec1 = {result3[0], result3[1], result3[2], result3[3]};
        float64x4_t dvec2 = {result3[4], result3[5], result3[6], result3[7]};
        total_checksum += (int64_t)horizontal_sum_f64x4(dvec1);
        total_checksum += (int64_t)horizontal_sum_f64x4(dvec2);
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 4: Inline assembly with many vector operands */
    {
        int32x16_t asm_vec1 = *(int32x16_t *)&data_a[96];
        int32x16_t asm_vec2 = *(int32x16_t *)&data_b[112];
        int32x16_t asm_vec3 = *(int32x16_t *)&data_a[128];
        int32x16_t asm_vec4 = *(int32x16_t *)&data_b[144];
        
        int32x16_t asm_result1, asm_result2;
        
        /* Inline assembly with many vector operands */
        asm volatile(
            /* Hypothetical multi-operand vector operation */
            "# Complex vector operation with many operands\n\t"
            "vmovdqa %[v1], %%ymm0\n\t"
            "vmovdqa %[v2], %%ymm1\n\t"
            "vmovdqa %[v3], %%ymm2\n\t"
            "vmovdqa %[v4], %%ymm3\n\t"
            /* Some computation that uses all vectors */
            "vpaddd %%ymm0, %%ymm1, %%ymm4\n\t"
            "vpaddd %%ymm2, %%ymm3, %%ymm5\n\t"
            "vpsubd %%ymm4, %%ymm5, %%ymm6\n\t"
            "vmovdqa %%ymm6, %[r1]\n\t"
            /* Another computation chain */
            "vpslld $2, %%ymm0, %%ymm7\n\t"
            "vpor %%ymm1, %%ymm7, %%ymm7\n\t"
            "vmovdqa %%ymm7, %[r2]\n\t"
            : [r1] "=v" (asm_result1), [r2] "=v" (asm_result2)
            : [v1] "v" (asm_vec1), [v2] "v" (asm_vec2),
              [v3] "v" (asm_vec3), [v4] "v" (asm_vec4)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
              "memory"
        );
        
        total_checksum += horizontal_sum_i32x16(asm_result1);
        total_checksum += horizontal_sum_i32x16(asm_result2);
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 5: Mixed-type operations with __builtin_shufflevector */
    {
        /* Use __builtin_shufflevector with many indices */
        int64x8_t lvec1 = *(int64x8_t *)&data_a[160];
        int64x8_t lvec2 = *(int64x8_t *)&data_b[176];
        
        /* shufflevector with 16 indices (8 from each source) */
        int64x8_t shuffled = __builtin_shufflevector(lvec1, lvec2,
            0, 2, 4, 6, 8, 10, 12, 14);
        
        /* Another shuffle with different pattern */
        int64x8_t shuffled2 = __builtin_shufflevector(lvec1, lvec2,
            1, 3, 5, 7, 9, 11, 13, 15);
        
        /* Combine results */
        for (int i = 0; i < 8; i++) {
            total_checksum += shuffled[i] + shuffled2[i];
        }
        
        /* Complex shuffle with computed indices */
        int indices[16];
        for (int i = 0; i < 16; i++) {
            indices[i] = (i * vmask_seed + vmask_offset) % 16;
        }
        
        /* This may trigger the multi-operand path when expanded */
        int32x16_t vec_src1 = *(int32x16_t *)&data_a[192];
        int32x16_t vec_src2 = *(int32x16_t *)&data_b[208];
        
        /* Create mask from computed indices */
        int32x16_t complex_mask;
        for (int i = 0; i < 16; i++) {
            complex_mask[i] = indices[i];
        }
        
        int32x16_t final_shuffle = __builtin_shuffle(vec_src1, vec_src2, complex_mask);
        total_checksum += horizontal_sum_i32x16(final_shuffle);
    }
    
    /* Target-specific builtins (conditional compilation) */
#ifdef __x86_64__
    {
        int32x8_t x86_vec = *(int32x8_t *)&data_a[224];
        /* Use x86-specific shuffle intrinsic */
        int32x8_t x86_shuffled = __builtin_ia32_pshufd(x86_vec, 0x1B);
        
        for (int i = 0; i < 8; i++) {
            total_checksum += x86_shuffled[i];
        }
    }
#endif
    
#ifdef __ARM_NEON
    {
        int32x4_t neon_vec = *(int32x4_t *)&data_a[240];
        /* Use ARM NEON-specific intrinsic */
        int32x4_t neon_rev = __builtin_neon_vrev64q_s32(neon_vec);
        
        for (int i = 0; i < 4; i++) {
            total_checksum += neon_rev[i];
        }
    }
#endif
    
    /* Loop-dependent vector operations */
    {
        int32x16_t loop_vec1 = *(int32x16_t *)&data_a[0];
        int32x16_t loop_vec2 = *(int32x16_t *)&data_b[0];
        int32x16_t loop_result;
        
        /* Loop where shuffle mask depends on iteration */
        for (int iter = 0; iter < 4; iter++) {
            int32x16_t dynamic_mask;
            for (int i = 0; i < 16; i++) {
                /* Data-dependent mask calculation */
                dynamic_mask[i] = (i + iter + data_a[i % 256]) % 32;
            }
            
            /* Shuffle operation inside loop - cannot be optimized away */
            loop_result = __builtin_shuffle(loop_vec1, loop_vec2, dynamic_mask);
            
            /* Modify source vectors for next iteration */
            for (int i = 0; i < 16; i++) {
                loop_vec1[i] += loop_result[i] % 100;
            }
        }
        
        total_checksum += horizontal_sum_i32x16(loop_result);
    }
    
    printf("Total checksum: %ld\n", (long)total_checksum);
    return 0;
}
