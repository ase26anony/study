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

/* Initialize array with pseudo-random values */
static void init_array(int32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = (int32_t)(lcg_rand() % 1000);
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

static float horizontal_sum_float32x16(float32x16_t v) {
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += v[i];
    }
    return sum;
}

int main(void) {
    /* Large arrays to hold vector data */
    int32_t data_int[256];
    float data_float[256];
    double data_double[128];
    
    /* Initialize with pseudo-random data */
    init_array(data_int, 256);
    for (int i = 0; i < 256; i++) {
        data_float[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    for (int i = 0; i < 128; i++) {
        data_double[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
    
    /* Volatile variables to prevent constant propagation */
    volatile int mask_seed = lcg_rand() % 100;
    volatile int use_alt_mask = 0;
    
    int32_t checksum = 0;
    
    /* ========== KERNEL 1: Complex shuffle with computed mask ========== */
    {
        /* Load vectors from memory */
        int32x16_t vec_a = *(int32x16_t *)&data_int[0];
        int32x16_t vec_b = *(int32x16_t *)&data_int[16];
        
        /* Compute a non-constant mask vector using arithmetic */
        int32x16_t mask_vec;
        for (int i = 0; i < 16; i++) {
            /* Complex mask calculation that can't be constant folded */
            int idx = (i * mask_seed + 7) % 32;
            mask_vec[i] = idx;
        }
        
        /* This shuffle with 3 vector operands (32 elements total) 
           may require many operands during RTL expansion */
        int32x16_t result1 = __builtin_shuffle(vec_a, vec_b, mask_vec);
        
        /* Chain another shuffle operation */
        int32x16_t mask_vec2;
        for (int i = 0; i < 16; i++) {
            mask_vec2[i] = (mask_vec[i] + i + mask_seed) % 32;
        }
        
        int32x16_t result2 = __builtin_shuffle(result1, vec_b, mask_vec2);
        
        checksum += horizontal_sum_int32x16(result2);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* ========== KERNEL 2: Chain of shuffles with mixed types ========== */
    {
        float32x16_t fvec_a = *(float32x16_t *)&data_float[0];
        float32x16_t fvec_b = *(float32x16_t *)&data_float[16];
        
        /* Create a complex permutation chain */
        int32x16_t int_mask;
        for (int i = 0; i < 16; i++) {
            int_mask[i] = (i * 3 + mask_seed) % 32;
        }
        
        /* First shuffle */
        float32x16_t fresult1 = __builtin_shuffle(fvec_a, fvec_b, int_mask);
        
        /* Second shuffle with different mask */
        int32x16_t int_mask2;
        for (int i = 0; i < 16; i++) {
            int_mask2[i] = (int_mask[i] * 5 + i) % 32;
        }
        
        float32x16_t fresult2 = __builtin_shuffle(fresult1, fvec_a, int_mask2);
        
        /* Third shuffle - creating a chain that accumulates operands */
        int32x16_t int_mask3;
        for (int i = 0; i < 16; i++) {
            int_mask3[i] = (int_mask2[i] + int_mask[i] + i) % 32;
        }
        
        float32x16_t fresult3 = __builtin_shuffle(fresult2, fvec_b, int_mask3);
        
        checksum += (int32_t)horizontal_sum_float32x16(fresult3);
        
        asm volatile("" ::: "memory");
    }
    
    /* ========== KERNEL 3: Conditional vector permutations ========== */
    {
        int32x8_t vec_small_a = *(int32x8_t *)&data_int[64];
        int32x8_t vec_small_b = *(int32x8_t *)&data_int[72];
        int32x8_t vec_small_c = *(int32x8_t *)&data_int[80];
        
        /* Create masks that depend on volatile variable */
        int32x8_t mask1, mask2;
        for (int i = 0; i < 8; i++) {
            mask1[i] = (i + mask_seed) % 16;
            mask2[i] = (i * 2 + mask_seed + 5) % 16;
        }
        
        /* Conditional selection between two different shuffle results */
        int32x8_t shuffle1 = __builtin_shufflevector(vec_small_a, vec_small_b, 
            0, 2, 4, 6, 8, 10, 12, 14);
        int32x8_t shuffle2 = __builtin_shufflevector(vec_small_a, vec_small_c,
            1, 3, 5, 7, 9, 11, 13, 15);
        
        /* Use __builtin_shuffle with computed mask for more complexity */
        int32x8_t mask_cond;
        for (int i = 0; i < 8; i++) {
            mask_cond[i] = (use_alt_mask) ? mask1[i] : mask2[i];
        }
        
        int32x8_t result3 = __builtin_shuffle(shuffle1, shuffle2, mask_cond);
        
        /* Horizontal sum */
        for (int i = 0; i < 8; i++) {
            checksum += result3[i];
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* ========== KERNEL 4: Inline assembly with many operands ========== */
    {
        /* Use inline assembly with vector constraints */
        int32x16_t asm_vec1 = *(int32x16_t *)&data_int[96];
        int32x16_t asm_vec2 = *(int32x16_t *)&data_int[112];
        int32x16_t asm_vec3 = *(int32x16_t *)&data_int[128];
        int32x16_t asm_result;
        
        /* Inline assembly that uses multiple vector registers */
        asm volatile (
            /* Hypothetical multi-operand vector operation */
            "vmovdqa %[v1], %%ymm0\n\t"
            "vmovdqa %[v2], %%ymm1\n\t"
            "vmovdqa %[v3], %%ymm2\n\t"
            "vpalignr $4, %%ymm0, %%ymm1, %%ymm3\n\t"
            "vpshufd $0x1B, %%ymm3, %%ymm4\n\t"
            "vpaddd %%ymm2, %%ymm4, %%ymm5\n\t"
            "vmovdqa %%ymm5, %[out]"
            : [out] "=v" (asm_result)
            : [v1] "v" (asm_vec1),
              [v2] "v" (asm_vec2),
              [v3] "v" (asm_vec3)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "memory"
        );
        
        checksum += horizontal_sum_int32x16(asm_result);
        
        asm volatile("" ::: "memory");
    }
    
    /* ========== KERNEL 5: Architecture-specific builtins ========== */
#ifdef __x86_64__
    {
        int32x8_t x86_vec = *(int32x8_t *)&data_int[144];
        
        /* Use x86-specific shuffle intrinsic */
        int32x8_t x86_result;
        /* __builtin_ia32_pshufd takes immediate mask - use variable to prevent const prop */
        int imm_mask = (mask_seed % 256);
        
        /* This may expand to different patterns depending on optimization */
        x86_result = __builtin_ia32_pshufd(x86_vec, imm_mask);
        
        for (int i = 0; i < 8; i++) {
            checksum += x86_result[i];
        }
    }
#elif defined(__aarch64__)
    {
        int32x4_t neon_vec = *(int32x4_t *)&data_int[152];
        
        /* Use ARM NEON intrinsic */
        int32x4_t neon_result = __builtin_neon_vrev64q_s32(neon_vec);
        
        for (int i = 0; i < 4; i++) {
            checksum += neon_result[i];
        }
    }
#endif
    
    /* ========== KERNEL 6: Loop-dependent vector operations ========== */
    {
        /* Data-dependent loop with vector operations */
        int32x16_t accum_vec = {0};
        
        for (int iter = 0; iter < 10; iter++) {
            int32x16_t loop_vec_a = *(int32x16_t *)&data_int[iter * 16];
            int32x16_t loop_vec_b = *(int32x16_t *)&data_int[iter * 16 + 8];
            
            /* Compute mask based on loop iteration and volatile */
            int32x16_t loop_mask;
            for (int i = 0; i < 16; i++) {
                loop_mask[i] = (i * iter + mask_seed) % 32;
            }
            
            /* Shuffle that depends on loop variable */
            int32x16_t loop_result = __builtin_shuffle(loop_vec_a, loop_vec_b, loop_mask);
            
            /* Accumulate */
            accum_vec += loop_result;
        }
        
        checksum += horizontal_sum_int32x16(accum_vec);
    }
    
    /* Final checksum output to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
