#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int32_t int32x4_t __attribute__((vector_size(16)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));
typedef double float64x4_t __attribute__((vector_size(32)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function to create non-constant mask vectors */
static inline int32x16_t create_complex_mask(int offset) {
    volatile int seed = offset; /* Prevent constant propagation */
    int32x16_t mask = {0};
    int32_t *mask_ptr = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        /* Complex, data-dependent mask calculation */
        mask_ptr[i] = (seed + i * 3) % 32;
        mask_ptr[i] ^= (lcg_rand() & 0x1F);
        if (mask_ptr[i] < 0) mask_ptr[i] += 32;
    }
    return mask;
}

/* Kernel 1: Complex shuffle with computed mask */
static void kernel1_complex_shuffle(int32x16_t *results, 
                                   const int32x16_t *src1, 
                                   const int32x16_t *src2,
                                   int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Create non-constant mask to prevent optimization */
        int32x16_t mask = create_complex_mask(i);
        
        /* Complex shuffle requiring many operands during expansion */
        results[i] = __builtin_shuffle(src1[i], src2[i], mask);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
}

/* Kernel 2: Chain of shuffles accumulating operand count */
static void kernel2_shuffle_chain(int32x16_t *results,
                                 const int32x16_t *src1,
                                 const int32x16_t *src2,
                                 const int32x16_t *src3,
                                 int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* First shuffle */
        int32x16_t mask1 = create_complex_mask(i * 2);
        int32x16_t intermediate = __builtin_shuffle(src1[i], src2[i], mask1);
        
        /* Second shuffle using result of first */
        int32x16_t mask2 = create_complex_mask(i * 2 + 1);
        int32x16_t result = __builtin_shuffle(intermediate, src3[i], mask2);
        
        /* Third shuffle with mixed sources */
        int32x16_t mask3 = create_complex_mask(i * 3);
        results[i] = __builtin_shuffle(result, src1[i], mask3);
        
        asm volatile("" ::: "memory");
    }
}

/* Kernel 3: Conditional vector permutation */
static void kernel3_conditional_shuffle(int32x16_t *results,
                                       const int32x16_t *src1,
                                       const int32x16_t *src2,
                                       const int32x16_t *src3,
                                       int iterations) {
    volatile int condition_seed = 42; /* Prevent constant propagation */
    
    for (int i = 0; i < iterations; i++) {
        /* Create two different masks */
        int32x16_t mask_a = create_complex_mask(i + condition_seed);
        int32x16_t mask_b = create_complex_mask(i + condition_seed + 1000);
        
        /* Create two different shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(src1[i], src2[i], mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(src2[i], src3[i], mask_b);
        
        /* Conditional selection between shuffle results */
        int condition = (lcg_rand() & 1);
        results[i] = condition ? shuffle_a : shuffle_b;
        
        asm volatile("" ::: "memory");
    }
}

/* Kernel 4: Mixed vector types and widths */
static void kernel4_mixed_types(float64x8_t *fresults,
                               int32x16_t *iresults,
                               const float64x8_t *fsrc1,
                               const float64x8_t *fsrc2,
                               const int32x16_t *isrc1,
                               const int32x16_t *isrc2,
                               int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Float shuffle with computed mask */
        int32x8_t float_mask;
        int32_t *fmask_ptr = (int32_t*)&float_mask;
        for (int j = 0; j < 8; j++) {
            fmask_ptr[j] = (i + j * 5) % 16;
        }
        
        /* Cast and shuffle float vectors */
        int32x8_t fsrc1_int = *(int32x8_t*)&fsrc1[i];
        int32x8_t fsrc2_int = *(int32x8_t*)&fsrc2[i];
        int32x8_t fshuffled = __builtin_shuffle(fsrc1_int, fsrc2_int, float_mask);
        fresults[i] = *(float64x8_t*)&fshuffled;
        
        /* Integer shuffle with different mask */
        int32x16_t int_mask = create_complex_mask(i + 500);
        iresults[i] = __builtin_shuffle(isrc1[i], isrc2[i], int_mask);
        
        asm volatile("" ::: "memory");
    }
}

/* Kernel 5: Inline assembly with many vector operands */
static void kernel5_inline_asm(int32x16_t *results,
                              const int32x16_t *src1,
                              const int32x16_t *src2,
                              const int32x16_t *src3,
                              int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly with multiple vector operands */
        int32x16_t a = src1[i];
        int32x16_t b = src2[i];
        int32x16_t c = src3[i];
        int32x16_t result;
        
        /* Complex inline assembly that may require many operands */
        asm volatile (
            /* Hypothetical multi-operand vector operation */
            "/* Complex vector operation with many inputs */\n\t"
            "vmovdqa %[vec_a], %%ymm0\n\t"
            "vmovdqa %[vec_b], %%ymm1\n\t"
            "vmovdqa %[vec_c], %%ymm2\n\t"
            "/* Additional processing would go here */\n\t"
            "vmovdqa %%ymm0, %[out]"
            : [out] "=v" (result)
            : [vec_a] "v" (a),
              [vec_b] "v" (b),
              [vec_c] "v" (c)
            : "ymm0", "ymm1", "ymm2", "memory"
        );
        
        results[i] = result;
        asm volatile("" ::: "memory");
    }
}

/* Target-specific builtins (conditional compilation) */
#ifdef __x86_64__
static void kernel_x86_specific(int32x4_t *results,
                               const int32x4_t *src,
                               int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Use x86-specific shuffle intrinsic */
        results[i] = __builtin_ia32_pshufd(src[i], 
                                          (i & 0xFF) | ((i+1) << 8) | 
                                          ((i+2) << 16) | ((i+3) << 24));
    }
}
#endif

#ifdef __ARM_NEON
static void kernel_arm_specific(int32x4_t *results,
                               const int32x4_t *src,
                               int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Use ARM-specific reversal intrinsic */
        results[i] = __builtin_neon_vrev64q_s32(src[i]);
    }
}
#endif

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(const int32x16_t *vectors, int count) {
    int64_t checksum = 0;
    const int32_t *data = (const int32_t*)vectors;
    
    for (int i = 0; i < count * 16; i++) {
        checksum += data[i];
    }
    
    return checksum;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int VEC_COUNT = ARRAY_SIZE / 16; /* 16 ints per vector */
    
    /* Allocate and initialize arrays with pseudo-random data */
    int32_t *base_data = aligned_alloc(64, ARRAY_SIZE * sizeof(int32_t));
    float64_t *float_data = aligned_alloc(64, ARRAY_SIZE * sizeof(float64_t));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base_data[i] = (int32_t)lcg_rand();
        float_data[i] = (float64_t)lcg_rand() / 1000.0;
    }
    
    /* Cast to vector types */
    int32x16_t *vec_data = (int32x16_t*)base_data;
    int32x16_t *vec_results = aligned_alloc(64, VEC_COUNT * sizeof(int32x16_t));
    float64x8_t *float_vec_data = (float64x8_t*)float_data;
    float64x8_t *float_results = aligned_alloc(64, VEC_COUNT * sizeof(float64x8_t));
    
    /* Initialize result arrays */
    memset(vec_results, 0, VEC_COUNT * sizeof(int32x16_t));
    memset(float_results, 0, VEC_COUNT * sizeof(float64x8_t));
    
    /* Execute kernels */
    int iterations = VEC_COUNT / 4; /* Use subset for speed */
    
    printf("Running Kernel 1...\n");
    kernel1_complex_shuffle(vec_results, vec_data, &vec_data[VEC_COUNT/2], iterations);
    
    printf("Running Kernel 2...\n");
    kernel2_shuffle_chain(&vec_results[iterations], 
                         vec_data, 
                         &vec_data[VEC_COUNT/4], 
                         &vec_data[VEC_COUNT/2], 
                         iterations);
    
    printf("Running Kernel 3...\n");
    kernel3_conditional_shuffle(&vec_results[iterations*2],
                               vec_data,
                               &vec_data[VEC_COUNT/4],
                               &vec_data[VEC_COUNT/2],
                               iterations);
    
    printf("Running Kernel 4...\n");
    kernel4_mixed_types(float_results,
                       &vec_results[iterations*3],
                       float_vec_data,
                       &float_vec_data[VEC_COUNT/2],
                       vec_data,
                       &vec_data[VEC_COUNT/4],
                       iterations);
    
    printf("Running Kernel 5...\n");
    kernel5_inline_asm(&vec_results[0], /* Overwrite first results */
                      vec_data,
                      &vec_data[VEC_COUNT/4],
                      &vec_data[VEC_COUNT/2],
                      iterations);
    
    /* Target-specific kernels */
#ifdef __x86_64__
    printf("Running x86-specific kernel...\n");
    int32x4_t *x86_results = aligned_alloc(16, iterations * sizeof(int32x4_t));
    int32x4_t *x86_src = (int32x4_t*)base_data;
    kernel_x86_specific(x86_results, x86_src, iterations);
    free(x86_results);
#endif
    
#ifdef __ARM_NEON
    printf("Running ARM-specific kernel...\n");
    int32x4_t *arm_results = aligned_alloc(16, iterations * sizeof(int32x4_t));
    int32x4_t *arm_src = (int32x4_t*)base_data;
    kernel_arm_specific(arm_results, arm_src, iterations);
    free(arm_results);
#endif
    
    /* Compute and print checksum to prevent optimization */
    int64_t checksum = compute_checksum(vec_results, VEC_COUNT);
    printf("Final checksum: %ld\n", (long)checksum);
    
    /* Cleanup */
    free(base_data);
    free(float_data);
    free(vec_results);
    free(float_results);
    
    return 0;
}
