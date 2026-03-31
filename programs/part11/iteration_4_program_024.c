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
static void init_arrays(int32_t *arr1, int32_t *arr2, float *farr, double *darr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr1[i] = (int32_t)lcg_rand();
        arr2[i] = (int32_t)lcg_rand();
        farr[i] = (float)lcg_rand() / (float)UINT32_MAX;
        darr[i] = (double)lcg_rand() / (double)UINT32_MAX;
    }
}

/* Complex mask calculation preventing constant propagation */
static int32x16_t compute_dynamic_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask calculation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i * 3) % 32;
        if (control & 1) {
            mask_data[i] = (mask_data[i] + 7) % 32;
        }
        /* Add more complexity to prevent optimization */
        mask_data[i] ^= (control >> (i & 3)) & 0xF;
    }
    
    return mask;
}

/* Another complex mask with different pattern */
static int32x16_t compute_alternate_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t *mask_data = (int32_t*)&mask;
    
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (i * 5 + control * 11) % 48;
        /* Conditional modification based on control */
        if ((control ^ i) & 1) {
            mask_data[i] = (mask_data[i] + 16) % 48;
        }
        /* More arithmetic to prevent simplification */
        mask_data[i] = (mask_data[i] * 3 + 1) & 0x1F;
    }
    
    return mask;
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    int32_t *array1 = malloc(ARRAY_SIZE * sizeof(int32_t));
    int32_t *array2 = malloc(ARRAY_SIZE * sizeof(int32_t));
    float *farray = malloc(ARRAY_SIZE * sizeof(float));
    double *darray = malloc(ARRAY_SIZE * sizeof(double));
    
    if (!array1 || !array2 || !farray || !darray) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array1, array2, farray, darray, ARRAY_SIZE);
    
    volatile int control_var = 42; /* Volatile to prevent constant propagation */
    int64_t checksum = 0;
    
    /* KERNEL 1: Complex shuffle with computed mask */
    for (int iter = 0; iter < 100; iter++) {
        /* Load vectors from different parts of arrays */
        int32x16_t vec_a = *((int32x16_t*)&array1[iter * 8]);
        int32x16_t vec_b = *((int32x16_t*)&array2[iter * 8]);
        
        /* Compute dynamic mask - prevents compile-time simplification */
        int32x16_t mask = compute_dynamic_mask(control_var + iter);
        
        /* Complex shuffle operation that may expand to many operands */
        int32x16_t result1 = __builtin_shuffle(vec_a, vec_b, mask);
        
        /* Store result back */
        *((int32x16_t*)&array1[iter * 8]) = result1;
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
        
        /* Update control variable */
        control_var ^= result1[0];
    }
    
    /* KERNEL 2: Chain of shuffles accumulating operand count */
    for (int iter = 0; iter < 50; iter++) {
        /* Load multiple vector types */
        int32x8_t vec1 = *((int32x8_t*)&array1[iter * 16]);
        int32x8_t vec2 = *((int32x8_t*)&array2[iter * 16]);
        float32x8_t fvec1 = *((float32x8_t*)&farray[iter * 8]);
        float32x8_t fvec2 = *((float32x8_t*)&farray[iter * 8 + 64]);
        
        /* First shuffle */
        int32_t mask1_data[16];
        for (int i = 0; i < 16; i++) {
            mask1_data[i] = (control_var + i * 7) % 16;
        }
        int32x16_t mask1 = *((int32x16_t*)mask1_data);
        
        /* Cast and shuffle - creates complex operand graph */
        int32x16_t wide_vec1 = __builtin_shufflevector(vec1, vec2, 
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
        
        /* Second shuffle with different mask */
        int32x16_t mask2 = compute_alternate_mask(control_var + iter * 2);
        int32x16_t shuffled1 = __builtin_shuffle(wide_vec1, wide_vec1, mask2);
        
        /* Third shuffle mixing float and int vectors */
        int32x8_t int_from_float = __builtin_convertvector(fvec1, int32x8_t);
        int32x16_t mixed_vec = __builtin_shufflevector(int_from_float, vec2,
            0, 1, 2, 3, 8, 9, 10, 11, 4, 5, 6, 7, 12, 13, 14, 15);
        
        /* Final complex shuffle chain */
        int32x16_t mask3 = compute_dynamic_mask(control_var ^ shuffled1[0]);
        int32x16_t final_result = __builtin_shuffle(shuffled1, mixed_vec, mask3);
        
        /* Store and update checksum */
        *((int32x16_t*)&array2[iter * 8]) = final_result;
        
        /* Horizontal add for checksum */
        for (int i = 0; i < 16; i++) {
            checksum += final_result[i];
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 3: Conditional vector permutation */
    for (int iter = 0; iter < 25; iter++) {
        int64x8_t vec64_a = *((int64x8_t*)&array1[iter * 32]);
        int64x8_t vec64_b = *((int64x8_t*)&array2[iter * 32]);
        
        /* Complex mask calculation */
        int64_t mask_data[16];
        for (int i = 0; i < 16; i++) {
            mask_data[i] = (control_var * i + iter) % 16;
            /* Data-dependent modification */
            if (checksum & (1 << (i & 7))) {
                mask_data[i] = (mask_data[i] + 8) % 16;
            }
        }
        
        /* Conditional shuffle selection */
        int64x8_t mask_vec = *((int64x8_t*)mask_data);
        int64x8_t shuffle_a = __builtin_shuffle(vec64_a, vec64_b, mask_vec);
        
        /* Alternate mask for conditional path */
        for (int i = 0; i < 16; i++) {
            mask_data[i] = (mask_data[i] * 5 + 1) % 16;
        }
        int64x8_t mask_vec2 = *((int64x8_t*)mask_data);
        int64x8_t shuffle_b = __builtin_shuffle(vec64_b, vec64_a, mask_vec2);
        
        /* Conditional selection between two shuffle results */
        int64x8_t cond_result = (control_var & (1 << (iter & 3))) 
            ? shuffle_a 
            : shuffle_b;
        
        /* Store result */
        *((int64x8_t*)&array1[iter * 32]) = cond_result;
        
        /* Update checksum */
        for (int i = 0; i < 8; i++) {
            checksum += cond_result[i];
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 4: Inline assembly with many vector operands */
#ifdef __x86_64__
    for (int iter = 0; iter < 10; iter++) {
        float64x4_t dvec1 = *((float64x4_t*)&darray[iter * 16]);
        float64x4_t dvec2 = *((float64x4_t*)&darray[iter * 16 + 8]);
        float64x4_t dvec3 = *((float64x4_t*)&darray[iter * 16 + 16]);
        float64x4_t dvec4 = *((float64x4_t*)&darray[iter * 16 + 24]);
        
        float64x4_t result1, result2, result3, result4;
        
        /* Inline assembly with multiple vector operands */
        asm volatile(
            /* Hypothetical multi-operand vector operation */
            "vmovapd %[vec1], %%ymm0\n\t"
            "vmovapd %[vec2], %%ymm1\n\t"
            "vmovapd %[vec3], %%ymm2\n\t"
            "vmovapd %[vec4], %%ymm3\n\t"
            "vperm2f128 $0x21, %%ymm0, %%ymm1, %%ymm4\n\t"
            "vperm2f128 $0x21, %%ymm2, %%ymm3, %%ymm5\n\t"
            "vblendvpd %%ymm4, %%ymm5, %%ymm0, %%ymm6\n\t"
            "vmovapd %%ymm6, %[out1]\n\t"
            "vpermilpd $0x5, %%ymm6, %%ymm7\n\t"
            "vmovapd %%ymm7, %[out2]\n\t"
            : [out1] "=m" (result1), [out2] "=m" (result2)
            : [vec1] "m" (dvec1), [vec2] "m" (dvec2), 
              [vec3] "m" (dvec3), [vec4] "m" (dvec4)
            : "ymm0", "ymm1", "ymm2", "ymm3", 
              "ymm4", "ymm5", "ymm6", "ymm7", "memory"
        );
        
        /* Use results to prevent elimination */
        *((float64x4_t*)&darray[iter * 16]) = result1;
        *((float64x4_t*)&darray[iter * 16 + 8]) = result2;
        
        checksum += (int64_t)result1[0] + (int64_t)result2[0];
        
        asm volatile("" ::: "memory");
    }
#endif
    
    /* Target-specific builtins for different architectures */
#ifdef __ARM_NEON
    /* ARM NEON specific operations */
    for (int iter = 0; iter < 10; iter++) {
        int32x4_t neon_vec1 = *((int32x4_t*)&array1[iter * 4]);
        int32x4_t neon_vec2 = *((int32x4_t*)&array2[iter * 4]);
        
        /* Complex NEON permutation chain */
        int32x4_t rev1 = __builtin_neon_vrev64q_s32(neon_vec1);
        int32x4_t rev2 = __builtin_neon_vrev64q_s32(neon_vec2);
        
        /* More operations to increase operand count */
        int32x4_t blended = __builtin_neon_vbslq_s32(
            __builtin_neon_vceqq_s32(neon_vec1, neon_vec2),
            rev1, rev2);
        
        *((int32x4_t*)&array1[iter * 4]) = blended;
        
        for (int i = 0; i < 4; i++) {
            checksum += blended[i];
        }
    }
#endif
    
    /* Final checksum computation */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array1[i] + array2[i];
        checksum += (int64_t)farray[i];
        checksum += (int64_t)darray[i];
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    free(array1);
    free(array2);
    free(farray);
    free(darray);
    
    return 0;
}
