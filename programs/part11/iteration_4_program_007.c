#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Vector type definitions */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));

/* Volatile control variables to prevent constant propagation */
volatile int shuffle_mode = 0;
volatile int permutation_seed = 42;

/* Function to compute complex, non-constant shuffle mask */
static int32x16_t compute_complex_mask(int base) {
    int32x16_t mask = {0};
    volatile int v = permutation_seed; /* Prevent optimization */
    
    for (int i = 0; i < 16; i++) {
        /* Data-dependent mask calculation */
        mask[i] = (base + i * v) % 32;
        if (shuffle_mode) {
            mask[i] = (mask[i] * 3 + 7) % 32;
        }
    }
    return mask;
}

/* Function with inline assembly using many vector operands */
static void multi_operand_asm_operation(int32x16_t *result, 
                                        int32x16_t a, 
                                        int32x16_t b,
                                        int32x16_t c,
                                        int32x16_t d) {
#ifdef __x86_64__
    /* x86-specific inline assembly with many operands */
    asm volatile (
        /* Hypothetical multi-operand vector operation */
        "vmovdqa %[a], %%ymm0\n\t"
        "vmovdqa %[b], %%ymm1\n\t"
        "vmovdqa %[c], %%ymm2\n\t"
        "vmovdqa %[d], %%ymm3\n\t"
        /* Complex permutation sequence */
        "vperm2i128 $0x21, %%ymm0, %%ymm1, %%ymm4\n\t"
        "vperm2i128 $0x30, %%ymm2, %%ymm3, %%ymm5\n\t"
        "vpblendd $0xF0, %%ymm4, %%ymm5, %%ymm6\n\t"
        "vmovdqa %%ymm6, %[res]\n\t"
        : [res] "=v" (*result)
        : [a] "v" (a), [b] "v" (b), [c] "v" (c), [d] "v" (d)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "memory"
    );
#elif defined(__aarch64__)
    /* ARM-specific inline assembly */
    asm volatile (
        /* Complex NEON operation with many registers */
        "mov v0.16b, %[a].16b\n\t"
        "mov v1.16b, %[b].16b\n\t"
        "mov v2.16b, %[c].16b\n\t"
        "mov v3.16b, %[d].16b\n\t"
        "tbl v4.16b, {v0.16b, v1.16b}, v2.16b\n\t"
        "tbl v5.16b, {v1.16b, v2.16b}, v3.16b\n\t"
        "add v6.16b, v4.16b, v5.16b\n\t"
        "mov %[res].16b, v6.16b\n\t"
        : [res] "=w" (*result)
        : [a] "w" (a), [b] "w" (b), [c] "w" (c), [d] "w" (d)
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "memory"
    );
#else
    /* Generic fallback using __builtin_shuffle with many operands */
    int32x16_t temp1 = __builtin_shuffle(a, b, compute_complex_mask(0));
    int32x16_t temp2 = __builtin_shuffle(c, d, compute_complex_mask(8));
    *result = __builtin_shuffle(temp1, temp2, compute_complex_mask(16));
#endif
}

int main(void) {
    /* Initialize large arrays with pseudo-random data */
    int32_t data_a[64] __attribute__((aligned(64)));
    int32_t data_b[64] __attribute__((aligned(64)));
    int32_t data_c[64] __attribute__((aligned(64)));
    int32_t data_d[64] __attribute__((aligned(64)));
    float float_data[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        data_a[i] = (int32_t)lcg_rand();
        data_b[i] = (int32_t)lcg_rand();
        data_c[i] = (int32_t)lcg_rand();
        data_d[i] = (int32_t)lcg_rand();
        float_data[i] = (float)lcg_rand() / 1000.0f;
    }
    
    /* Cast to various vector types */
    int32x16_t *vec_a = (int32x16_t *)data_a;
    int32x16_t *vec_b = (int32x16_t *)data_b;
    int32x16_t *vec_c = (int32x16_t *)data_c;
    int32x16_t *vec_d = (int32x16_t *)data_d;
    float32x16_t *vec_f = (float32x16_t *)float_data;
    
    /* Result storage */
    int32x16_t results[4] __attribute__((aligned(64)));
    float32x16_t float_results[2] __attribute__((aligned(64)));
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /*** KERNEL 1: Complex shuffle with computed mask ***/
    /* This should trigger multi-operand expansion when mask is non-constant */
    for (int iter = 0; iter < 100; iter++) {
        /* Data-dependent shuffle mode */
        shuffle_mode = (iter % 3 == 0) ? 1 : 0;
        permutation_seed = iter;
        
        /* Complex shuffle with two source vectors and computed mask */
        int32x16_t mask1 = compute_complex_mask(iter);
        results[0] = __builtin_shuffle(vec_a[0], vec_b[0], mask1);
        
        /* Compiler barrier between operations */
        asm volatile("" ::: "memory");
        
        /* Another shuffle with different mask */
        int32x16_t mask2 = compute_complex_mask(iter + 16);
        results[1] = __builtin_shuffle(vec_c[0], vec_d[0], mask2);
    }
    
    /*** KERNEL 2: Chain of shuffles increasing operand count ***/
    /* Chain multiple shuffles where output of one is input to next */
    for (int i = 0; i < 50; i++) {
        /* First shuffle */
        int32x16_t temp_mask1 = compute_complex_mask(i * 2);
        int32x16_t temp1 = __builtin_shuffle(vec_a[0], vec_b[0], temp_mask1);
        
        /* Second shuffle using result of first */
        int32x16_t temp_mask2 = compute_complex_mask(i * 2 + 1);
        int32x16_t temp2 = __builtin_shuffle(temp1, vec_c[0], temp_mask2);
        
        /* Third shuffle with more operands */
        int32x16_t temp_mask3 = compute_complex_mask(i * 3);
        results[2] = __builtin_shuffle(temp2, vec_d[0], temp_mask3);
        
        /* Mixed float/int shuffles */
        float32x16_t float_temp = __builtin_shufflevector(vec_f[0], vec_f[1], 
            0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30);
        float_results[0] = float_temp;
    }
    
    /*** KERNEL 3: Conditional vector permutation ***/
    /* Use conditional operator to select between different shuffle results */
    for (int i = 0; i < 50; i++) {
        int32x16_t mask_a = compute_complex_mask(i);
        int32x16_t mask_b = compute_complex_mask(i + 100);
        
        int32x16_t shuffle_a = __builtin_shuffle(vec_a[0], vec_b[0], mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec_c[0], vec_d[0], mask_b);
        
        /* Conditional selection between two shuffle results */
        results[3] = (i % 2 == 0) ? shuffle_a : shuffle_b;
        
        /* Complex shufflevector with many indices */
        if (i % 3 == 0) {
            /* This __builtin_shufflevector with many indices may require
               many operands during RTL expansion */
            float32x16_t complex_shuffle = __builtin_shufflevector(
                vec_f[0], vec_f[1], vec_f[2], vec_f[3],
                0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60
            );
            float_results[1] = complex_shuffle;
        }
    }
    
    /*** KERNEL 4: Inline assembly with many operands ***/
    /* Use inline assembly that requires many vector register operands */
    multi_operand_asm_operation(&results[0], 
                                vec_a[0], vec_b[0], vec_c[0], vec_d[0]);
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /*** Target-specific builtins (conditional compilation) ***/
#ifdef __x86_64__
    /* Use x86-specific builtins that may expand to multi-operand patterns */
    for (int i = 0; i < 10; i++) {
        int32x8_t x86_vec = ((int32x8_t *)data_a)[0];
        /* __builtin_ia32_pshufd takes immediate and source, but expansion
           may involve multiple operands in the RTL representation */
        int32x8_t shuffled = __builtin_ia32_pshufd(x86_vec, 
            (i % 4) | ((i % 4) << 2) | ((i % 4) << 4) | ((i % 4) << 6));
        ((int32x8_t *)results)[0] = shuffled;
    }
#endif
    
#ifdef __aarch64__
    /* ARM NEON builtins */
    for (int i = 0; i < 10; i++) {
        int32x4_t neon_vec = ((int32x4_t *)data_a)[0];
        /* Complex builtin that may require multiple operands */
        int32x4_t reversed = __builtin_neon_vrev64q_s32(neon_vec);
        ((int32x4_t *)results)[0] = reversed;
    }
#endif
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += results[i][j];
        }
    }
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += (int64_t)float_results[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
