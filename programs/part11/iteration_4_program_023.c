#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Vector type definitions */
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
static void init_array(int32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = (int32_t)(lcg_rand() % 1000);
    }
}

/* Horizontal sum of vector elements */
static int64_t horizontal_sum_int32x16(int32x16_t v) {
    int64_t sum = 0;
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
    /* Large arrays for vector operations */
    int32_t data_a[256];
    int32_t data_b[256];
    int32_t data_c[256];
    float64_t data_f[256];
    
    /* Initialize arrays */
    init_array(data_a, 256);
    init_array(data_b, 256);
    init_array(data_c, 256);
    for (int i = 0; i < 256; i++) {
        data_f[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
    
    /* Volatile variables to prevent constant propagation */
    volatile int vmask_seed = 42;
    volatile int vpattern = 7;
    
    int64_t total_checksum = 0;
    
    /* Kernel 1: Complex shuffle with computed mask vector */
    {
        /* Load vectors from arrays */
        int32x16_t vec_a = *(int32x16_t *)(&data_a[0]);
        int32x16_t vec_b = *(int32x16_t *)(&data_a[16]);
        int32x16_t vec_c = *(int32x16_t *)(&data_b[0]);
        
        /* Compute dynamic mask based on volatile variable */
        int32x16_t mask;
        for (int i = 0; i < 16; i++) {
            mask[i] = (vmask_seed + i * vpattern) % 32;
        }
        
        /* Complex shuffle operation that may require many operands */
        int32x16_t result1 = __builtin_shuffle(vec_a, vec_b, mask);
        
        /* Another shuffle with different sources */
        int32x16_t mask2;
        for (int i = 0; i < 16; i++) {
            mask2[i] = (vmask_seed + i * 3) % 32;
        }
        int32x16_t result2 = __builtin_shuffle(vec_c, result1, mask2);
        
        /* Chain shuffles to increase operand count */
        int32x16_t mask3;
        for (int i = 0; i < 16; i++) {
            mask3[i] = (i * 5 + vmask_seed) % 32;
        }
        int32x16_t final_result1 = __builtin_shuffle(result1, result2, mask3);
        
        total_checksum += horizontal_sum_int32x16(final_result1);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 2: Mixed vector types and widths */
    {
        /* Load different vector types */
        float64x8_t fvec_a = *(float64x8_t *)(&data_f[0]);
        float64x8_t fvec_b = *(float64x8_t *)(&data_f[8]);
        int32x16_t ivec_a = *(int32x16_t *)(&data_a[32]);
        int32x16_t ivec_b = *(int32x16_t *)(&data_a[48]);
        
        /* Convert float vectors to int for mixed operations */
        int64x8_t iconv_a = __builtin_convertvector(fvec_a, int64x8_t);
        int64x8_t iconv_b = __builtin_convertvector(fvec_b, int64x8_t);
        
        /* Create complex mask using arithmetic */
        int64x8_t mask_f;
        for (int i = 0; i < 8; i++) {
            mask_f[i] = (vmask_seed * i + vpattern) % 16;
        }
        
        /* Shuffle with converted vectors */
        int64x8_t shuffled_f = __builtin_shufflevector(iconv_a, iconv_b, 
            0, 9, 2, 11, 4, 13, 6, 15);
        
        /* More complex shufflevector with many indices */
        int32x16_t shuffled_i = __builtin_shufflevector(ivec_a, ivec_b,
            0, 17, 2, 19, 4, 21, 6, 23,
            8, 25, 10, 27, 12, 29, 14, 31);
        
        /* Cross-type operations increase operand complexity */
        float64x8_t final_f = __builtin_convertvector(shuffled_f, float64x8_t);
        total_checksum += (int64_t)horizontal_sum_float64x8(final_f);
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 3: Conditional vector permutations */
    {
        int32x16_t vec1 = *(int32x16_t *)(&data_a[64]);
        int32x16_t vec2 = *(int32x16_t *)(&data_a[80]);
        int32x16_t vec3 = *(int32x16_t *)(&data_b[32]);
        int32x16_t vec4 = *(int32x16_t *)(&data_b[48]);
        
        /* Dynamic condition */
        volatile int condition = vmask_seed > 20;
        
        /* Compute different masks based on condition */
        int32x16_t mask_a, mask_b;
        for (int i = 0; i < 16; i++) {
            mask_a[i] = (i + vmask_seed) % 32;
            mask_b[i] = (i * 3 + vpattern) % 32;
        }
        
        /* Conditional shuffle selection */
        int32x16_t shuffle_a = __builtin_shuffle(vec1, vec2, mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec3, vec4, mask_b);
        
        /* Use conditional operator on whole vectors */
        int32x16_t selected = condition ? shuffle_a : shuffle_b;
        
        /* Chain another operation */
        int32x16_t mask_c;
        for (int i = 0; i < 16; i++) {
            mask_c[i] = (i * 7 + condition) % 32;
        }
        int32x16_t final_result3 = __builtin_shuffle(selected, vec1, mask_c);
        
        total_checksum += horizontal_sum_int32x16(final_result3);
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 4: Inline assembly with many vector operands */
    {
        int32x16_t asm_vec1 = *(int32x16_t *)(&data_a[96]);
        int32x16_t asm_vec2 = *(int32x16_t *)(&data_a[112]);
        int32x16_t asm_vec3 = *(int32x16_t *)(&data_b[64]);
        int32x16_t asm_vec4 = *(int32x16_t *)(&data_b[80]);
        int32x16_t asm_vec5 = *(int32x16_t *)(&data_c[0]);
        int32x16_t asm_result1, asm_result2;
        
        /* Inline assembly with multiple vector operands */
        asm volatile(
            /* Hypothetical multi-operand vector operation */
            "vmovdqa %[v1], %%ymm0\n\t"
            "vmovdqa %[v2], %%ymm1\n\t"
            "vmovdqa %[v3], %%ymm2\n\t"
            "vmovdqa %[v4], %%ymm3\n\t"
            "vmovdqa %[v5], %%ymm4\n\t"
            /* Complex sequence of operations */
            "vperm2i128 $0x21, %%ymm0, %%ymm1, %%ymm5\n\t"
            "vperm2i128 $0x21, %%ymm2, %%ymm3, %%ymm6\n\t"
            "vpblendd $0xF0, %%ymm5, %%ymm6, %%ymm7\n\t"
            "vmovdqa %%ymm7, %[out1]\n\t"
            "vpermq $0x4E, %%ymm4, %%ymm8\n\t"
            "vmovdqa %%ymm8, %[out2]\n\t"
            : [out1] "=v" (asm_result1), [out2] "=v" (asm_result2)
            : [v1] "v" (asm_vec1), [v2] "v" (asm_vec2),
              [v3] "v" (asm_vec3), [v4] "v" (asm_vec4),
              [v5] "v" (asm_vec5)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4",
              "ymm5", "ymm6", "ymm7", "ymm8", "memory"
        );
        
        total_checksum += horizontal_sum_int32x16(asm_result1);
        total_checksum += horizontal_sum_int32x16(asm_result2);
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 5: Architecture-specific builtins */
    {
#ifdef __x86_64__
        /* x86-specific builtins with many operands */
        int32x8_t x86_vec1 = *(int32x8_t *)(&data_a[128]);
        int32x8_t x86_vec2 = *(int32x8_t *)(&data_a[136]);
        
        /* Use x86 shuffle intrinsic */
        int32x8_t x86_result = __builtin_ia32_pshufd(x86_vec1, 0x1B);
        
        /* Chain with other operations */
        int32x8_t x86_result2 = __builtin_ia32_pblendw128(x86_result, x86_vec2, 0xF0);
        
        total_checksum += horizontal_sum_int32x16(__builtin_convertvector(x86_result2, int32x16_t));
#endif
        
#ifdef __aarch64__
        /* ARM-specific builtins */
        int32x4_t arm_vec1 = *(int32x4_t *)(&data_a[144]);
        int32x4_t arm_vec2 = *(int32x4_t *)(&data_a[148]);
        
        /* ARM reverse operation */
        int32x4_t arm_result = __builtin_neon_vrev64q_s32(arm_vec1);
        
        total_checksum += arm_result[0] + arm_result[1] + arm_result[2] + arm_result[3];
#endif
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 6: Loop-dependent vector operations */
    {
        int32x16_t loop_vecs[4];
        for (int i = 0; i < 4; i++) {
            loop_vecs[i] = *(int32x16_t *)(&data_a[160 + i * 16]);
        }
        
        int32x16_t accum = {0};
        
        /* Loop with data-dependent shuffles */
        for (int iter = 0; iter < 8; iter++) {
            volatile int iter_mod = iter % 4;
            
            /* Compute mask based on iteration */
            int32x16_t loop_mask;
            for (int i = 0; i < 16; i++) {
                loop_mask[i] = (i * iter + iter_mod) % 64;
            }
            
            /* Select source vectors based on iteration */
            int32x16_t src1 = loop_vecs[iter % 4];
            int32x16_t src2 = loop_vecs[(iter + 1) % 4];
            
            /* Perform shuffle */
            int32x16_t shuffled = __builtin_shuffle(src1, src2, loop_mask);
            
            /* Accumulate results */
            accum += shuffled;
        }
        
        total_checksum += horizontal_sum_int32x16(accum);
    }
    
    printf("Final checksum: %ld\n", total_checksum);
    
    return 0;
}
