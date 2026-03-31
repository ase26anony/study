#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Vector type definitions */
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

/* Initialize arrays with pseudo-random data */
static void init_arrays(int32_t *arr1, int32_t *arr2, int32_t *arr3, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr1[i] = (int32_t)lcg_rand();
        arr2[i] = (int32_t)lcg_rand();
        arr3[i] = (int32_t)lcg_rand();
    }
}

/* Complex shuffle mask computation - prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int selector) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (selector + i * 3) % 32;
        if (selector & 1) {
            mask_data[i] = (mask_data[i] + 7) % 32;
        } else {
            mask_data[i] = (mask_data[i] * 2) % 32;
        }
    }
    
    return mask;
}

/* Another mask computation with different pattern */
static int32x16_t compute_alternate_mask(volatile int phase) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (phase + i * 5) % 32;
        mask_data[i] ^= (phase >> 2);
        mask_data[i] &= 31;
    }
    
    return mask;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    int32_t *array1 = malloc(ARRAY_SIZE * sizeof(int32_t));
    int32_t *array2 = malloc(ARRAY_SIZE * sizeof(int32_t));
    int32_t *array3 = malloc(ARRAY_SIZE * sizeof(int32_t));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array1, array2, array3, ARRAY_SIZE);
    
    volatile int control_var = 0;
    int64_t checksum = 0;
    
    /* Kernel 1: Complex shuffle with computed mask */
    for (int iter = 0; iter < 100; iter++) {
        control_var = iter % 7;
        
        /* Load vectors from different parts of arrays */
        int32x16_t vec_a = *((int32x16_t*)(array1 + iter * 4));
        int32x16_t vec_b = *((int32x16_t*)(array2 + iter * 4 + 16));
        int32x16_t vec_c = *((int32x16_t*)(array3 + iter * 4 + 32));
        
        /* Compute dynamic mask */
        int32x16_t mask1 = compute_complex_mask(control_var);
        
        /* Complex shuffle operation - may require many operands during expansion */
        int32x16_t result1 = __builtin_shuffle(vec_a, vec_b, mask1);
        
        /* Chain another shuffle with different mask */
        int32x16_t mask2 = compute_alternate_mask(control_var + 1);
        int32x16_t result2 = __builtin_shuffle(result1, vec_c, mask2);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Accumulate checksum */
        int32_t *res_ptr = (int32_t*)&result2;
        for (int i = 0; i < 16; i++) {
            checksum += res_ptr[i];
        }
    }
    
    /* Kernel 2: Chain of shuffles with mixed types */
    for (int iter = 0; iter < 50; iter++) {
        control_var = iter % 5;
        
        /* Load as different types for type mixing */
        int32x8_t vec_i32_8a = *((int32x8_t*)(array1 + iter * 8));
        int32x8_t vec_i32_8b = *((int32x8_t*)(array2 + iter * 8));
        float32x8_t vec_f32_8 = *((float32x8_t*)(array3 + iter * 8));
        
        /* Create complex shufflevector with many operands */
        int32x8_t shuffled1 = __builtin_shufflevector(
            vec_i32_8a, vec_i32_8b,
            0, 2, 4, 6, 8, 10, 12, 14
        );
        
        /* Another shuffle with computed indices */
        int indices[8];
        for (int i = 0; i < 8; i++) {
            indices[i] = (control_var + i * 2) % 16;
        }
        
        /* Cast and shuffle - forcing type conversions */
        int32x8_t temp_vec = (int32x8_t)vec_f32_8;
        int32x8_t shuffled2 = __builtin_shuffle(temp_vec, shuffled1, 
            (int32x8_t){indices[0], indices[1], indices[2], indices[3],
                       indices[4], indices[5], indices[6], indices[7]});
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Accumulate checksum */
        int32_t *res_ptr = (int32_t*)&shuffled2;
        for (int i = 0; i < 8; i++) {
            checksum += res_ptr[i];
        }
    }
    
    /* Kernel 3: Conditional vector permutations */
    for (int iter = 0; iter < 75; iter++) {
        control_var = lcg_rand() % 100;
        
        int32x16_t vec1 = *((int32x16_t*)(array1 + iter * 2));
        int32x16_t vec2 = *((int32x16_t*)(array2 + iter * 2 + 8));
        int32x16_t vec3 = *((int32x16_t*)(array3 + iter * 2 + 16));
        
        /* Compute two different masks */
        int32x16_t mask_a = compute_complex_mask(control_var);
        int32x16_t mask_b = compute_alternate_mask(control_var);
        
        /* Conditional selection between two shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(vec1, vec2, mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(vec2, vec3, mask_b);
        
        int32x16_t final_result = (control_var > 50) ? shuffle_a : shuffle_b;
        
        /* Additional shuffle chain */
        int32x16_t mask_c = compute_complex_mask(control_var + 3);
        int32x16_t result = __builtin_shuffle(final_result, vec1, mask_c);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Accumulate checksum */
        int32_t *res_ptr = (int32_t*)&result;
        for (int i = 0; i < 16; i++) {
            checksum += res_ptr[i];
        }
    }
    
    /* Kernel 4: Inline assembly with many vector operands */
#ifdef __x86_64__
    for (int iter = 0; iter < 25; iter++) {
        int32x8_t v1 = *((int32x8_t*)(array1 + iter * 16));
        int32x8_t v2 = *((int32x8_t*)(array2 + iter * 16));
        int32x8_t v3 = *((int32x8_t*)(array3 + iter * 16));
        int32x8_t v4, v5, v6;
        
        /* Inline asm with multiple vector operands */
        asm volatile (
            "vmovdqa %[in1], %%ymm0\n\t"
            "vmovdqa %[in2], %%ymm1\n\t"
            "vmovdqa %[in3], %%ymm2\n\t"
            "vpermq $0x1B, %%ymm0, %%ymm3\n\t"
            "vpermq $0x39, %%ymm1, %%ymm4\n\t"
            "vpblendd $0xF0, %%ymm3, %%ymm4, %%ymm5\n\t"
            "vmovdqa %%ymm5, %[out1]\n\t"
            "vpshufd $0x1B, %%ymm2, %%ymm6\n\t"
            "vmovdqa %%ymm6, %[out2]\n\t"
            : [out1] "=v" (v4), [out2] "=v" (v5)
            : [in1] "v" (v1), [in2] "v" (v2), [in3] "v" (v3)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "memory"
        );
        
        /* Use the results */
        int32_t *ptr4 = (int32_t*)&v4;
        int32_t *ptr5 = (int32_t*)&v5;
        for (int i = 0; i < 8; i++) {
            checksum += ptr4[i] + ptr5[i];
        }
    }
#endif
    
    /* Kernel 5: Large vector operations with shufflevector */
    {
        int64x8_t big_vec1 = *((int64x8_t*)array1);
        int64x8_t big_vec2 = *((int64x8_t*)array2);
        
        /* shufflevector with many indices - may trigger multi-operand expansion */
        int64x8_t shuffled_big = __builtin_shufflevector(
            big_vec1, big_vec2,
            0, 2, 4, 6, 8, 10, 12, 14
        );
        
        /* Another complex shuffle */
        int64x8_t shuffled_big2 = __builtin_shufflevector(
            big_vec2, big_vec1,
            1, 3, 5, 7, 9, 11, 13, 15
        );
        
        /* Combine results */
        int64_t *ptr1 = (int64_t*)&shuffled_big;
        int64_t *ptr2 = (int64_t*)&shuffled_big2;
        for (int i = 0; i < 8; i++) {
            checksum += ptr1[i] + ptr2[i];
        }
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
