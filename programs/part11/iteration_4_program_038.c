#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random values */
static void init_array(void *arr, size_t size) {
    uint32_t *ptr = (uint32_t *)arr;
    size_t count = size / sizeof(uint32_t);
    for (size_t i = 0; i < count; i++) {
        ptr[i] = lcg_rand();
    }
}

/* Horizontal sum of vector elements */
static int32_t horizontal_sum_int32x16(int32x16_t v) {
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += v[i];
    }
    return sum;
}

static double horizontal_sum_float64x8(float64x8_t v) {
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += v[i];
    }
    return sum;
}

int main(void) {
    /* Large arrays to hold vector data */
    int32_t array_a[256];
    int32_t array_b[256];
    float64_t array_c[128];
    float64_t array_d[128];
    
    /* Initialize with pseudo-random data */
    init_array(array_a, sizeof(array_a));
    init_array(array_b, sizeof(array_b));
    init_array(array_c, sizeof(array_c));
    init_array(array_d, sizeof(array_d));
    
    /* Volatile variables to prevent constant propagation */
    volatile int mask_seed = lcg_rand() % 100;
    volatile int permutation_mode = lcg_rand() % 3;
    
    int32_t checksum = 0;
    
    /* Kernel 1: Complex shuffle with computed mask vector */
    {
        /* Load vectors from arrays */
        int32x16_t vec_a = *(int32x16_t *)&array_a[0];
        int32x16_t vec_b = *(int32x16_t *)&array_b[16];
        
        /* Compute dynamic mask based on volatile seed */
        int32x16_t mask_vec;
        for (int i = 0; i < 16; i++) {
            mask_vec[i] = (mask_seed + i * 3) % 32;
        }
        
        /* Complex shuffle operation - may require many operands during expansion */
        int32x16_t result1 = __builtin_shuffle(vec_a, vec_b, mask_vec);
        
        /* Chain another shuffle operation */
        int32x16_t mask_vec2;
        for (int i = 0; i < 16; i++) {
            mask_vec2[i] = (mask_seed + i * 7) % 32;
        }
        int32x16_t result2 = __builtin_shuffle(result1, vec_b, mask_vec2);
        
        checksum += horizontal_sum_int32x16(result2);
        asm volatile("" ::: "memory"); /* Compiler barrier */
    }
    
    /* Kernel 2: Chain of shuffles with mixed vector types */
    {
        float64x8_t vec_c = *(float64x8_t *)&array_c[0];
        float64x8_t vec_d = *(float64x8_t *)&array_d[8];
        
        /* Create complex permutation sequence */
        int64x8_t mask1, mask2, mask3;
        for (int i = 0; i < 8; i++) {
            mask1[i] = (mask_seed + i * 5) % 16;
            mask2[i] = (mask_seed + i * 11) % 16;
            mask3[i] = (mask_seed + i * 13) % 16;
        }
        
        /* Chain multiple shufflevector operations */
        float64x8_t temp1 = __builtin_shufflevector(vec_c, vec_d, 
            mask1[0], mask1[1], mask1[2], mask1[3],
            mask1[4], mask1[5], mask1[6], mask1[7]);
        
        float64x8_t temp2 = __builtin_shufflevector(vec_d, temp1,
            mask2[0], mask2[1], mask2[2], mask2[3],
            mask2[4], mask2[5], mask2[6], mask2[7]);
        
        float64x8_t result3 = __builtin_shufflevector(temp1, temp2,
            mask3[0], mask3[1], mask3[2], mask3[3],
            mask3[4], mask3[5], mask3[6], mask3[7]);
        
        checksum += (int32_t)horizontal_sum_float64x8(result3);
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 3: Conditional vector permutation */
    {
        int32x8_t vec1 = *(int32x8_t *)&array_a[32];
        int32x8_t vec2 = *(int32x8_t *)&array_a[64];
        int32x8_t vec3 = *(int32x8_t *)&array_b[32];
        
        /* Dynamic mask computation */
        int32x8_t cond_mask;
        for (int i = 0; i < 8; i++) {
            cond_mask[i] = (mask_seed + i) % 16;
        }
        
        /* Conditional shuffle selection */
        int32x8_t shuffle_a = __builtin_shuffle(vec1, vec2, cond_mask);
        
        /* Different mask for alternative shuffle */
        int32x8_t alt_mask;
        for (int i = 0; i < 8; i++) {
            alt_mask[i] = (mask_seed + i * 2) % 16;
        }
        int32x8_t shuffle_b = __builtin_shuffle(vec2, vec3, alt_mask);
        
        /* Conditional selection between two shuffle results */
        int32x8_t selector;
        for (int i = 0; i < 8; i++) {
            selector[i] = (permutation_mode > i) ? -1 : 0;
        }
        int32x8_t result4 = selector ? shuffle_a : shuffle_b;
        
        /* Horizontal sum */
        for (int i = 0; i < 8; i++) {
            checksum += result4[i];
        }
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 4: Inline assembly with many vector operands */
    {
        int32x16_t asm_vec1 = *(int32x16_t *)&array_a[96];
        int32x16_t asm_vec2 = *(int32x16_t *)&array_a[128];
        int32x16_t asm_vec3 = *(int32x16_t *)&array_b[96];
        int32x16_t asm_vec4 = *(int32x16_t *)&array_b[128];
        
        int32x16_t asm_result1, asm_result2;
        
        /* Inline assembly with multiple vector operands */
        asm volatile(
            /* Hypothetical multi-operand vector operation */
            "vmovdqa %[v1], %%ymm0\n\t"
            "vmovdqa %[v2], %%ymm1\n\t"
            "vmovdqa %[v3], %%ymm2\n\t"
            "vmovdqa %[v4], %%ymm3\n\t"
            /* Complex permutation sequence */
            "vperm2i128 $0x21, %%ymm0, %%ymm1, %%ymm4\n\t"
            "vperm2i128 $0x30, %%ymm2, %%ymm3, %%ymm5\n\t"
            "vpblendd $0xAA, %%ymm4, %%ymm5, %%ymm6\n\t"
            "vmovdqa %%ymm6, %[out1]\n\t"
            "vpshufd $0x1B, %%ymm6, %%ymm7\n\t"
            "vmovdqa %%ymm7, %[out2]"
            : [out1] "=v" (asm_result1), [out2] "=v" (asm_result2)
            : [v1] "v" (asm_vec1), [v2] "v" (asm_vec2),
              [v3] "v" (asm_vec3), [v4] "v" (asm_vec4)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7"
        );
        
        checksum += horizontal_sum_int32x16(asm_result1);
        checksum += horizontal_sum_int32x16(asm_result2);
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 5: Architecture-specific builtins (conditional compilation) */
    {
        int32x8_t arch_vec1 = *(int32x8_t *)&array_a[160];
        int32x8_t arch_vec2 = *(int32x8_t *)&array_a[192];
        
        /* Use architecture-specific builtins when available */
#ifdef __x86_64__
        /* x86-specific shuffle builtins */
        int32x8_t x86_result = __builtin_ia32_pshufd256(arch_vec1, 0x1B);
        for (int i = 0; i < 8; i++) {
            checksum += x86_result[i];
        }
#endif
        
#ifdef __ARM_NEON
        /* ARM-specific permutation builtins */
        int32x4_t neon_vec1 = {arch_vec1[0], arch_vec1[1], arch_vec1[2], arch_vec1[3]};
        int32x4_t neon_vec2 = {arch_vec1[4], arch_vec1[5], arch_vec1[6], arch_vec1[7]};
        int32x4_t neon_result = __builtin_neon_vrev64q_s32(neon_vec1);
        for (int i = 0; i < 4; i++) {
            checksum += neon_result[i];
        }
#endif
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 6: Loop-dependent vector operations */
    {
        int32x16_t loop_vec1 = *(int32x16_t *)&array_a[224];
        int32x16_t loop_vec2 = *(int32x16_t *)&array_b[224];
        
        int32x16_t loop_result = {0};
        
        /* Loop with data-dependent shuffles */
        for (int iter = 0; iter < 4; iter++) {
            /* Dynamic mask based on loop iteration */
            int32x16_t loop_mask;
            for (int i = 0; i < 16; i++) {
                loop_mask[i] = (mask_seed + i + iter * 4) % 32;
            }
            
            /* Shuffle operation inside loop - cannot be optimized away */
            int32x16_t temp = __builtin_shuffle(loop_vec1, loop_vec2, loop_mask);
            
            /* Accumulate results */
            for (int i = 0; i < 16; i++) {
                loop_result[i] += temp[i];
            }
            
            /* Modify source vectors for next iteration */
            for (int i = 0; i < 16; i++) {
                loop_vec1[i] = (loop_vec1[i] + iter) & 0xFF;
            }
        }
        
        checksum += horizontal_sum_int32x16(loop_result);
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
