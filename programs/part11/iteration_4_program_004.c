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

/* Initialize array with pseudo-random data */
static void init_array(int32_t *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = (int32_t)(lcg_rand() % 1000);
    }
}

/* Complex mask computation that prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        int idx = (i * control + i * i) % 32;
        if (idx < 16) {
            mask_data[i] = idx;
        } else {
            mask_data[i] = idx - 16 + 100; /* Offset to distinguish sources */
        }
    }
    
    return mask;
}

/* Another complex mask with different pattern */
static int32x16_t compute_alternate_mask(volatile int seed) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* More complex pattern with modulo operations */
    for (int i = 0; i < 16; i++) {
        int base = (seed + i * 7) % 29;
        mask_data[i] = (base * 11 + i * 3) % 32;
    }
    
    return mask;
}

int main(void) {
    /* Large arrays for vector operations */
    int32_t array_a[256];
    int32_t array_b[256];
    int32_t array_c[256];
    float float_array[256];
    double double_array[128];
    
    /* Initialize arrays */
    init_array(array_a, 256);
    init_array(array_b, 256);
    init_array(array_c, 256);
    
    for (int i = 0; i < 256; i++) {
        float_array[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    for (int i = 0; i < 128; i++) {
        double_array[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
    
    /* Cast to vector types */
    int32x16_t *vec_a = (int32x16_t*)array_a;
    int32x16_t *vec_b = (int32x16_t*)array_b;
    int32x16_t *vec_c = (int32x16_t*)array_c;
    float32x16_t *vec_f = (float32x16_t*)float_array;
    float64x8_t *vec_d = (float64x8_t*)double_array;
    
    /* Result vectors */
    int32x16_t results[4];
    float32x16_t float_results[2];
    float64x8_t double_results[2];
    
    /* Volatile control variables to prevent constant propagation */
    volatile int control_var = 7;
    volatile int alt_control = 13;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /******************************************************************
     * KERNEL 1: Complex shuffle with computed mask
     * This should generate many operands during RTL expansion
     ******************************************************************/
    for (int iter = 0; iter < 10; iter++) {
        control_var = iter % 5 + 1;
        
        /* Compute dynamic masks */
        int32x16_t mask1 = compute_complex_mask(control_var);
        int32x16_t mask2 = compute_alternate_mask(control_var + iter);
        
        /* Complex shuffle with 2 source vectors and computed mask */
        results[0] = __builtin_shuffle(vec_a[0], vec_b[0], mask1);
        
        /* Chain of shuffles - output of one is input to next */
        int32x16_t temp1 = __builtin_shuffle(results[0], vec_c[0], mask2);
        int32x16_t temp2 = __builtin_shuffle(vec_b[0], temp1, mask1);
        
        /* Another shuffle with mixed sources */
        results[1] = __builtin_shuffle(temp2, vec_a[1], mask2);
        
        /* Compiler barrier between operations */
        asm volatile("" ::: "memory");
    }
    
    /******************************************************************
     * KERNEL 2: Mixed vector types and conditional permutations
     ******************************************************************/
    for (int iter = 0; iter < 8; iter++) {
        alt_control = iter % 3 + 2;
        
        /* Compute masks for float vectors */
        int32x16_t float_mask = compute_complex_mask(alt_control * 3);
        
        /* Cast between types for complex operations */
        int32x16_t int_temp = *(int32x16_t*)&vec_f[0];
        int32x16_t int_temp2 = *(int32x16_t*)&vec_f[1];
        
        /* Conditional vector permutation */
        int32x16_t shuffle_a = __builtin_shuffle(int_temp, vec_a[0], float_mask);
        int32x16_t shuffle_b = __builtin_shuffle(int_temp2, vec_b[0], float_mask);
        
        /* Use conditional operator on vectors - forces complex expansion */
        results[2] = (control_var > 3) ? shuffle_a : shuffle_b;
        
        /* Another chained operation */
        int32x16_t mask3 = compute_alternate_mask(iter * 5);
        results[3] = __builtin_shuffle(results[2], vec_c[0], mask3);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /******************************************************************
     * KERNEL 3: Double precision vector operations
     ******************************************************************/
    {
        /* Create mask for double vectors (8 elements) */
        int64x8_t double_mask = {0, 2, 4, 6, 1, 3, 5, 7};
        
        /* Shuffle double vectors */
        float64x8_t dbl_temp = __builtin_shufflevector(
            vec_d[0], vec_d[1], 
            0, 2, 4, 6, 8, 10, 12, 14
        );
        
        /* Another shuffle with different pattern */
        float64x8_t dbl_temp2 = __builtin_shufflevector(
            vec_d[1], vec_d[0],
            1, 3, 5, 7, 9, 11, 13, 15
        );
        
        /* Conditional selection */
        double_results[0] = (control_var % 2) ? dbl_temp : dbl_temp2;
        
        /* Complex chain with cast to int and back */
        int64x8_t int64_temp = *(int64x8_t*)&double_results[0];
        int64x8_t shuffled_int = __builtin_shuffle(
            int64_temp, 
            *(int64x8_t*)&vec_d[0], 
            double_mask
        );
        
        double_results[1] = *(float64x8_t*)&shuffled_int;
    }
    
    /******************************************************************
     * KERNEL 4: Inline assembly with many vector operands
     * This directly exercises the operand counting logic
     ******************************************************************/
#ifdef __x86_64__
    {
        int32x8_t asm_vec1 = *(int32x8_t*)&array_a[0];
        int32x8_t asm_vec2 = *(int32x8_t*)&array_b[0];
        int32x8_t asm_vec3 = *(int32x8_t*)&array_c[0];
        int32x8_t asm_vec4 = *(int32x8_t*)&array_a[8];
        int32x8_t asm_vec5 = *(int32x8_t*)&array_b[8];
        int32x8_t asm_vec6 = *(int32x8_t*)&array_c[8];
        int32x8_t asm_vec7, asm_vec8, asm_vec9, asm_vec10;
        
        /* Inline asm with many vector operands - approaching 11 total */
        asm volatile (
            "vmovdqa %[v1], %%ymm0\n\t"
            "vmovdqa %[v2], %%ymm1\n\t"
            "vmovdqa %[v3], %%ymm2\n\t"
            "vmovdqa %[v4], %%ymm3\n\t"
            "vmovdqa %[v5], %%ymm4\n\t"
            "vmovdqa %[v6], %%ymm5\n\t"
            /* Complex sequence of operations */
            "vpaddd %%ymm0, %%ymm1, %%ymm6\n\t"
            "vpaddd %%ymm2, %%ymm3, %%ymm7\n\t"
            "vpsubd %%ymm4, %%ymm5, %%ymm8\n\t"
            "vpaddd %%ymm6, %%ymm7, %%ymm9\n\t"
            "vpsubd %%ymm8, %%ymm9, %%ymm10\n\t"
            "vmovdqa %%ymm10, %[out1]\n\t"
            "vmovdqa %%ymm9, %[out2]\n\t"
            "vmovdqa %%ymm8, %[out3]\n\t"
            "vmovdqa %%ymm7, %[out4]\n\t"
            : [out1] "=v" (asm_vec7),
              [out2] "=v" (asm_vec8),
              [out3] "=v" (asm_vec9),
              [out4] "=v" (asm_vec10)
            : [v1] "v" (asm_vec1),
              [v2] "v" (asm_vec2),
              [v3] "v" (asm_vec3),
              [v4] "v" (asm_vec4),
              [v5] "v" (asm_vec5),
              [v6] "v" (asm_vec6)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
              "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "memory"
        );
        
        /* Use results to prevent elimination */
        results[0] = __builtin_shuffle(
            *(int32x16_t*)&asm_vec7,
            *(int32x16_t*)&asm_vec8,
            compute_complex_mask(control_var)
        );
    }
#endif
    
    /******************************************************************
     * Target-specific builtins for different architectures
     ******************************************************************/
#ifdef __ARM_NEON
    {
        /* Use ARM NEON specific builtins */
        int32x4_t neon_vec1 = {1, 2, 3, 4};
        int32x4_t neon_vec2 = {5, 6, 7, 8};
        int32x4_t neon_vec3 = {9, 10, 11, 12};
        int32x4_t neon_vec4 = {13, 14, 15, 16};
        
        /* Chain NEON operations */
        int32x4_t rev1 = __builtin_neon_vrev64q_s32(neon_vec1);
        int32x4_t rev2 = __builtin_neon_vrev64q_s32(neon_vec2);
        int32x4_t added = __builtin_neon_vaddq_s32(rev1, rev2);
        int32x4_t final = __builtin_neon_vaddq_s32(added, neon_vec3);
        
        /* Use in shuffle */
        int32x16_t wide_vec = {0};
        memcpy(&wide_vec, &final, sizeof(int32x4_t));
        results[1] = __builtin_shuffle(wide_vec, results[1], 
                                      compute_alternate_mask(5));
    }
#endif
    
    /******************************************************************
     * Final checksum computation to prevent dead code elimination
     ******************************************************************/
    int64_t checksum = 0;
    
    /* Horizontal addition of result vectors */
    for (int i = 0; i < 4; i++) {
        int32_t *data = (int32_t*)&results[i];
        for (int j = 0; j < 16; j++) {
            checksum += data[j];
        }
    }
    
    /* Add float results */
    for (int i = 0; i < 2; i++) {
        float *fdata = (float*)&float_results[i];
        for (int j = 0; j < 16; j++) {
            checksum += (int64_t)(fdata[j] * 1000);
        }
    }
    
    /* Add double results */
    for (int i = 0; i < 2; i++) {
        double *ddata = (double*)&double_results[i];
        for (int j = 0; j < 8; j++) {
            checksum += (int64_t)(ddata[j] * 1000);
        }
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    return 0;
}
