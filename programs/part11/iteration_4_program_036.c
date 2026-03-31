#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define large vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int32_t* arr32, size_t n32, 
                       float* arrf32, size_t nf32,
                       double* arrf64, size_t nf64) {
    for (size_t i = 0; i < n32; i++) arr32[i] = (int32_t)lcg_rand();
    for (size_t i = 0; i < nf32; i++) arrf32[i] = (float)lcg_rand() / 4294967296.0f;
    for (size_t i = 0; i < nf64; i++) arrf64[i] = (double)lcg_rand() / 4294967296.0;
}

/* Complex shuffle with computed mask - prevents constant propagation */
static int32x16_t complex_shuffle(int32x16_t a, int32x16_t b, volatile int mask_seed) {
    /* Create a non-constant mask using arithmetic */
    int32x16_t mask;
    for (int i = 0; i < 16; i++) {
        mask[i] = (i * mask_seed + 7) % 32;  /* Non-constant indices */
    }
    
    /* This shuffle requires 3 operands: a, b, mask */
    return __builtin_shuffle(a, b, mask);
}

/* Chain of shuffles to accumulate operand count */
static int32x16_t shuffle_chain(int32x16_t v1, int32x16_t v2, 
                               int32x16_t v3, int32x16_t v4,
                               volatile int control) {
    /* First shuffle: v1 and v2 with computed mask */
    int32x16_t mask1;
    for (int i = 0; i < 16; i++) {
        mask1[i] = (i + control) % 32;
    }
    int32x16_t r1 = __builtin_shuffle(v1, v2, mask1);
    
    /* Second shuffle: r1 and v3 with different mask */
    int32x16_t mask2;
    for (int i = 0; i < 16; i++) {
        mask2[i] = (i * control + 3) % 32;
    }
    int32x16_t r2 = __builtin_shuffle(r1, v3, mask2);
    
    /* Third shuffle: r2 and v4 with another mask */
    int32x16_t mask3;
    for (int i = 0; i < 16; i++) {
        mask3[i] = (32 - i + control) % 32;
    }
    return __builtin_shuffle(r2, v4, mask3);
}

/* Conditional vector permutation */
static float32x16_t conditional_shuffle(float32x16_t a, float32x16_t b,
                                       float32x16_t c, float32x16_t d,
                                       volatile int condition) {
    /* Create two different shuffle results */
    float32x16_t mask_a, mask_b;
    for (int i = 0; i < 16; i++) {
        mask_a[i] = (i + condition) % 32;
        mask_b[i] = (i * condition + 5) % 32;
    }
    
    float32x16_t result1 = __builtin_shuffle(a, b, mask_a);
    float32x16_t result2 = __builtin_shuffle(c, d, mask_b);
    
    /* Conditional selection between two shuffle results */
    return condition > 0 ? result1 : result2;
}

/* Mixed-type permutation operations */
static void mixed_type_permutations(int32_t* data32, float* dataf32, 
                                   double* dataf64, volatile int iter) {
    /* Cast to various vector types */
    int32x16_t* v32 = (int32x16_t*)data32;
    float32x16_t* vf32 = (float32x16_t*)dataf32;
    float64x8_t* vf64 = (float64x8_t*)dataf64;
    
    /* Complex operation mixing types */
    for (int i = 0; i < 4; i++) {
        /* Create non-constant masks */
        int32x16_t mask32;
        float32x16_t maskf32;
        
        for (int j = 0; j < 16; j++) {
            mask32[j] = (j * iter + i) % 32;
            maskf32[j] = (float)((j + iter * i) % 32);
        }
        
        /* Chain of operations with mixed types */
        int32x16_t temp1 = __builtin_shuffle(v32[i], v32[(i+1)%4], mask32);
        
        /* Convert and shuffle float vectors */
        float32x16_t tempf = __builtin_convertvector(temp1, float32x16_t);
        float32x16_t result = __builtin_shuffle(tempf, vf32[i], maskf32);
        
        /* Store back */
        vf32[i] = result;
    }
}

/* Inline assembly with many vector operands */
#ifdef __x86_64__
static void x86_vector_asm(int32x16_t* v1, int32x16_t* v2, 
                          int32x16_t* v3, int32x16_t* v4,
                          int32x16_t* result) {
    /* Complex inline assembly with multiple vector operands */
    asm volatile (
        /* Hypothetical multi-operand vector operation */
        "vmovdqa %1, %%ymm0\n\t"
        "vmovdqa %2, %%ymm1\n\t"
        "vmovdqa %3, %%ymm2\n\t"
        "vmovdqa %4, %%ymm3\n\t"
        /* Complex permutation sequence */
        "vperm2i128 $0x21, %%ymm0, %%ymm1, %%ymm4\n\t"
        "vperm2i128 $0x30, %%ymm2, %%ymm3, %%ymm5\n\t"
        "vpblendd $0xF0, %%ymm4, %%ymm5, %%ymm6\n\t"
        "vmovdqa %%ymm6, %0\n\t"
        : "=m" (*result)
        : "m" (*v1), "m" (*v2), "m" (*v3), "m" (*v4)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "memory"
    );
}
#endif

#ifdef __aarch64__
static void arm_vector_asm(int32x8_t* v1, int32x8_t* v2, 
                          int32x8_t* v3, int32x8_t* v4,
                          int32x8_t* result) {
    /* ARM NEON inline assembly with multiple operands */
    asm volatile (
        "ld1 {v0.4s, v1.4s}, [%1]\n\t"
        "ld1 {v2.4s, v3.4s}, [%2]\n\t"
        "ld1 {v4.4s, v5.4s}, [%3]\n\t"
        "ld1 {v6.4s, v7.4s}, [%4]\n\t"
        /* Complex NEON permutation sequence */
        "trn1 v8.4s, v0.4s, v2.4s\n\t"
        "trn2 v9.4s, v1.4s, v3.4s\n\t"
        "zip1 v10.4s, v4.4s, v6.4s\n\t"
        "zip2 v11.4s, v5.4s, v7.4s\n\t"
        "add v12.4s, v8.4s, v10.4s\n\t"
        "add v13.4s, v9.4s, v11.4s\n\t"
        "st1 {v12.4s, v13.4s}, [%0]\n\t"
        : "=r" (result)
        : "r" (v1), "r" (v2), "r" (v3), "r" (v4)
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "v8", "v9", "v10", "v11", "v12", "v13", "memory"
    );
}
#endif

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(int32_t* data, size_t n) {
    int64_t sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum += data[i];
    }
    return sum;
}

int main(void) {
    /* Allocate aligned memory for vector operations */
    const size_t ARRAY_SIZE = 1024;
    
    /* Use aligned_alloc for proper alignment (C11) */
    int32_t* data32 = (int32_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    float* dataf32 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double* dataf64 = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    if (!data32 || !dataf32 || !dataf64) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(data32, ARRAY_SIZE, dataf32, ARRAY_SIZE, dataf64, ARRAY_SIZE);
    
    /* Cast to vector types */
    int32x16_t* vectors32 = (int32x16_t*)data32;
    float32x16_t* vectorsf32 = (float32x16_t*)dataf32;
    float64x8_t* vectorsf64 = (float64x8_t*)dataf64;
    
    /* Volatile control variable to prevent constant propagation */
    volatile int control = 42;
    
    /* Kernel 1: Complex shuffle with computed mask */
    for (int i = 0; i < 8; i++) {
        int32x16_t result = complex_shuffle(vectors32[i], 
                                           vectors32[(i+1) % 8], 
                                           control + i);
        /* Store result */
        vectors32[i] = result;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Kernel 2: Chain of shuffles */
    for (int i = 0; i < 4; i++) {
        int32x16_t result = shuffle_chain(vectors32[i*2],
                                         vectors32[i*2+1],
                                         vectors32[(i*2+2) % 8],
                                         vectors32[(i*2+3) % 8],
                                         control - i);
        vectors32[i*2] = result;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Kernel 3: Conditional vector permutation */
    for (int i = 0; i < 4; i++) {
        float32x16_t result = conditional_shuffle(vectorsf32[i*2],
                                                 vectorsf32[i*2+1],
                                                 vectorsf32[(i*2+2) % 8],
                                                 vectorsf32[(i*2+3) % 8],
                                                 control + i * 3);
        vectorsf32[i*2] = result;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Kernel 4: Mixed-type permutations */
    mixed_type_permutations(data32, dataf32, dataf64, control);
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Target-specific inline assembly */
#ifdef __x86_64__
    if (ARRAY_SIZE >= 4 * 16) {
        x86_vector_asm(&vectors32[0], &vectors32[1], 
                      &vectors32[2], &vectors32[3],
                      &vectors32[4]);
    }
#endif
    
#ifdef __aarch64__
    if (ARRAY_SIZE >= 8 * 8) {
        int32x8_t* vectors8 = (int32x8_t*)data32;
        arm_vector_asm(&vectors8[0], &vectors8[1], 
                      &vectors8[2], &vectors8[3],
                      &vectors8[4]);
    }
#endif
    
    /* Final checksum to prevent optimization */
    int64_t checksum = compute_checksum(data32, ARRAY_SIZE);
    printf("Checksum: %ld\n", (long)checksum);
    
    /* Cleanup */
    free(data32);
    free(dataf32);
    free(dataf64);
    
    return 0;
}
