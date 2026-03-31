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

/* Complex mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control & 1) ? (i * 3) % 16 : (i * 5) % 16;
        if (i % 4 == 0) mask_data[i] += control;
        if (i % 3 == 0) mask_data[i] ^= control;
    }
    
    return mask;
}

/* Another complex mask with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control & 2) ? (i * 7) % 16 : (i * 11) % 16;
        mask_data[i] = (mask_data[i] + i + control) % 16;
    }
    
    return mask;
}

int main(void) {
    /* Large arrays to hold vector data */
    int32_t data_a[256];
    int32_t data_b[256];
    int32_t data_c[256];
    float float_data[256];
    
    /* Volatile control variables to prevent optimization */
    volatile int control1 = 42;
    volatile int control2 = 17;
    volatile int control3 = 99;
    
    /* Initialize data */
    init_array(data_a, 256);
    init_array(data_b, 256);
    init_array(data_c, 256);
    
    for (int i = 0; i < 256; i++) {
        float_data[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    /* Cast to various vector types */
    int32x16_t *vec_a = (int32x16_t*)data_a;
    int32x16_t *vec_b = (int32x16_t*)data_b;
    int32x16_t *vec_c = (int32x16_t*)data_c;
    float32x16_t *vec_f = (float32x16_t*)float_data;
    
    /* Result vectors */
    int32x16_t results[8];
    float32x16_t float_results[4];
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 1: Complex shuffle with computed mask
     * This should generate many operands during RTL expansion
     ******************************************************************/
    for (int iter = 0; iter < 100; iter++) {
        /* Compute masks that depend on volatile variables */
        int32x16_t mask1 = compute_complex_mask(control1 + iter);
        int32x16_t mask2 = compute_alternate_mask(control2 + iter);
        
        /* Complex shuffle operation with 10+ operands in expansion */
        results[0] = __builtin_shuffle(vec_a[0], vec_b[0], mask1);
        results[1] = __builtin_shuffle(vec_b[1], vec_c[1], mask2);
        
        /* Chain shuffles - output of one is input to next */
        results[2] = __builtin_shuffle(results[0], results[1], mask1);
        
        /* Another shuffle with mixed sources */
        int32x16_t temp = __builtin_shuffle(vec_a[2], vec_c[2], mask2);
        results[3] = __builtin_shuffle(temp, vec_b[2], mask1);
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 2: Chain of shuffles with mixed types
     * Creates dependency chain requiring many operands
     ******************************************************************/
    for (int iter = 0; iter < 50; iter++) {
        /* Convert between int and float vectors */
        float32x16_t float_temp1 = __builtin_convertvector(vec_a[iter % 4], float32x16_t);
        float32x16_t float_temp2 = __builtin_convertvector(vec_b[iter % 4], float32x16_t);
        
        /* Create mask for float shuffle */
        int32x16_t float_mask = compute_complex_mask(control3 + iter);
        
        /* Shuffle float vectors */
        float_results[0] = __builtin_shuffle(float_temp1, float_temp2, float_mask);
        
        /* Chain another operation */
        float_results[1] = __builtin_shuffle(float_results[0], vec_f[iter % 4], float_mask);
        
        /* Convert back to int and shuffle again */
        int32x16_t int_temp = __builtin_convertvector(float_results[1], int32x16_t);
        results[4] = __builtin_shuffle(int_temp, vec_c[iter % 4], float_mask);
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 3: Conditional vector permutations
     * Uses ?: operator to select between different shuffle results
     ******************************************************************/
    for (int iter = 0; iter < 75; iter++) {
        int32x16_t mask_a = compute_complex_mask(control1 ^ iter);
        int32x16_t mask_b = compute_alternate_mask(control2 ^ iter);
        
        /* Conditional shuffle selection */
        int32x16_t shuffle_a = __builtin_shuffle(vec_a[3], vec_b[3], mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec_b[3], vec_c[3], mask_b);
        
        /* Select between two different shuffle results */
        results[5] = (control1 > 50) ? shuffle_a : shuffle_b;
        
        /* Nested conditional shuffles */
        int32x16_t shuffle_c = __builtin_shuffle(shuffle_a, shuffle_b, mask_a);
        int32x16_t shuffle_d = __builtin_shuffle(shuffle_b, shuffle_a, mask_b);
        
        results[6] = (control2 < 25) ? shuffle_c : shuffle_d;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 4: Inline assembly with many vector operands
     * Directly uses inline asm with vector constraints
     ******************************************************************/
#ifdef __x86_64__
    for (int iter = 0; iter < 25; iter++) {
        /* Inline asm with multiple vector operands */
        asm volatile (
            /* Hypothetical multi-operand vector operation */
            "vmovdqa %[vec1], %%ymm0\n\t"
            "vmovdqa %[vec2], %%ymm1\n\t"
            "vmovdqa %[vec3], %%ymm2\n\t"
            "vpermq $0x1B, %%ymm0, %%ymm3\n\t"
            "vpermq $0x39, %%ymm1, %%ymm4\n\t"
            "vpblendd $0xF0, %%ymm3, %%ymm4, %%ymm5\n\t"
            "vmovdqa %%ymm5, %[result]\n\t"
            : [result] "=v" (results[7])
            : [vec1] "v" (vec_a[0]),
              [vec2] "v" (vec_b[0]),
              [vec3] "v" (vec_c[0])
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "memory"
        );
    }
#endif
    
#ifdef __ARM_NEON
    for (int iter = 0; iter < 25; iter++) {
        /* ARM NEON inline assembly */
        int32x4_t neon_vec1 = ((int32x4_t*)data_a)[0];
        int32x4_t neon_vec2 = ((int32x4_t*)data_b)[0];
        int32x4_t neon_vec3 = ((int32x4_t*)data_c)[0];
        int32x4_t neon_result;
        
        asm volatile (
            "vrev64.32 %q0, %q1\n\t"
            "vrev64.32 %q2, %q3\n\t"
            "vtrn.32 %q0, %q2\n\t"
            : "=w" (neon_result)
            : "w" (neon_vec1), "w" (neon_vec2), "w" (neon_vec3)
            : "memory"
        );
        
        /* Store back to larger vector */
        memcpy(&results[7], &neon_result, sizeof(neon_result));
    }
#endif
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 5: Mixed-width vector operations
     * Uses vector_size(32) and vector_size(64) types together
     ******************************************************************/
    {
        int32x8_t medium_vec1 = ((int32x8_t*)data_a)[0];
        int32x8_t medium_vec2 = ((int32x8_t*)data_b)[0];
        int32x8_t medium_vec3 = ((int32x8_t*)data_c)[0];
        
        /* Create mask for 8-element vector */
        int32x8_t medium_mask = {0, 2, 4, 6, 1, 3, 5, 7};
        
        /* Shuffle medium vectors */
        int32x8_t medium_result = __builtin_shufflevector(
            medium_vec1, medium_vec2, 
            0, 8, 2, 10, 4, 12, 6, 14
        );
        
        /* Another shuffle with different pattern */
        medium_result = __builtin_shufflevector(
            medium_result, medium_vec3,
            7, 6, 5, 4, 3, 2, 1, 0
        );
        
        /* Combine two medium vectors into one large vector */
        int32x16_t combined;
        memcpy(&combined, &medium_result, sizeof(medium_result));
        memcpy((char*)&combined + 32, &medium_vec2, sizeof(medium_vec2));
        
        results[0] = combined;
    }
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * Final checksum computation to prevent dead code elimination
     ******************************************************************/
    int64_t checksum = 0;
    
    for (int i = 0; i < 8; i++) {
        int32_t *result_data = (int32_t*)&results[i];
        for (int j = 0; j < 16; j++) {
            checksum += result_data[j];
        }
    }
    
    for (int i = 0; i < 4; i++) {
        float *float_result_data = (float*)&float_results[i];
        for (int j = 0; j < 16; j++) {
            checksum += (int64_t)float_result_data[j];
        }
    }
    
    printf("Final checksum: %lld\n", (long long)checksum);
    
    return 0;
}
