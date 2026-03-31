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

/* Initialize arrays with pseudo-random data */
static void init_arrays(int32_t* arr_int32, size_t size_int32,
                       float* arr_float, size_t size_float,
                       double* arr_double, size_t size_double) {
    for (size_t i = 0; i < size_int32; i++) {
        arr_int32[i] = (int32_t)lcg_rand();
    }
    for (size_t i = 0; i < size_float; i++) {
        arr_float[i] = (float)lcg_rand() / 1000.0f;
    }
    for (size_t i = 0; i < size_double; i++) {
        arr_double[i] = (double)lcg_rand() / 1000.0;
    }
}

/* Complex mask computation that prevents constant propagation */
static int32x16_t compute_complex_mask(volatile int control) {
    int32x16_t mask = {0};
    int32_t* mask_data = (int32_t*)&mask;
    
    /* Data-dependent mask computation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (control + i * 3) % 32;
        if (i % 4 == 0) mask_data[i] ^= control;
        if (i % 3 == 0) mask_data[i] += i * 2;
    }
    
    /* Additional non-linear transformation */
    for (int i = 0; i < 16; i++) {
        mask_data[i] = (mask_data[i] * 13 + 7) & 31;
    }
    
    return mask;
}

/* Horizontal sum for checksum computation */
static int32_t horizontal_sum_int32x16(int32x16_t v) {
    int32_t* data = (int32_t*)&v;
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += data[i];
    }
    return sum;
}

static float horizontal_sum_float32x16(float32x16_t v) {
    float* data = (float*)&v;
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += data[i];
    }
    return sum;
}

int main(void) {
    /* Allocate and initialize data arrays */
    const size_t ARRAY_SIZE = 1024;
    int32_t* int_data = malloc(ARRAY_SIZE * sizeof(int32_t));
    float* float_data = malloc(ARRAY_SIZE * sizeof(float));
    double* double_data = malloc(ARRAY_SIZE * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(int_data, ARRAY_SIZE, float_data, ARRAY_SIZE, double_data, ARRAY_SIZE);
    
    /* Volatile control variable to prevent compile-time optimization */
    volatile int control_var = 42;
    
    /* Result accumulators */
    int32_t int_checksum = 0;
    float float_checksum = 0.0f;
    
    /* KERNEL 1: Complex shuffle with computed mask vector */
    for (int iter = 0; iter < 100; iter++) {
        control_var = iter % 64;
        
        /* Load vectors from different parts of the array */
        int32x16_t* vec_a = (int32x16_t*)(int_data + iter * 4);
        int32x16_t* vec_b = (int32x16_t*)(int_data + iter * 4 + 16);
        int32x16_t* vec_c = (int32x16_t*)(int_data + iter * 4 + 32);
        
        /* Compute complex mask that can't be optimized away */
        int32x16_t mask = compute_complex_mask(control_var);
        
        /* Complex shuffle operation that may require many operands during expansion */
        int32x16_t result1 = __builtin_shuffle(*vec_a, *vec_b, mask);
        
        /* Another shuffle with different sources */
        int32x16_t mask2 = mask;
        for (int i = 0; i < 16; i++) {
            ((int32_t*)&mask2)[i] = (((int32_t*)&mask)[i] + 8) % 32;
        }
        int32x16_t result2 = __builtin_shuffle(*vec_c, result1, mask2);
        
        /* Store result back */
        int32x16_t* dest = (int32x16_t*)(int_data + iter * 4 + 48);
        *dest = result2;
        
        int_checksum += horizontal_sum_int32x16(result2);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 2: Chain of shuffles with mixed types */
    for (int iter = 0; iter < 50; iter++) {
        control_var = iter % 32;
        
        /* Mixed float/int vectors */
        float32x16_t* fvec_a = (float32x16_t*)(float_data + iter * 8);
        float32x16_t* fvec_b = (float32x16_t*)(float_data + iter * 8 + 16);
        int32x16_t* ivec_c = (int32x16_t*)(int_data + iter * 8 + 64);
        
        /* Compute mask based on control variable */
        int32x16_t mask = compute_complex_mask(control_var * 2);
        
        /* First shuffle with float vectors */
        float32x16_t fresult1 = __builtin_shuffle(*fvec_a, *fvec_b, mask);
        
        /* Convert and shuffle with int vector */
        int32x16_t iconverted = *(int32x16_t*)&fresult1;
        int32x16_t mask3 = mask;
        for (int i = 0; i < 16; i++) {
            ((int32_t*)&mask3)[i] = (((int32_t*)&mask)[i] * 2) % 32;
        }
        int32x16_t result3 = __builtin_shuffle(iconverted, *ivec_c, mask3);
        
        /* Another shuffle chain */
        int32x16_t mask4;
        for (int i = 0; i < 16; i++) {
            ((int32_t*)&mask4)[i] = (control_var + i * 5) % 32;
        }
        int32x16_t result4 = __builtin_shuffle(result3, iconverted, mask4);
        
        float_checksum += horizontal_sum_float32x16(*(float32x16_t*)&result4);
        
        asm volatile("" ::: "memory");
    }
    
    /* KERNEL 3: Conditional vector permutation */
    for (int iter = 0; iter < 25; iter++) {
        control_var = iter % 16;
        
        int32x16_t* vec1 = (int32x16_t*)(int_data + iter * 16);
        int32x16_t* vec2 = (int32x16_t*)(int_data + iter * 16 + 32);
        int32x16_t* vec3 = (int32x16_t*)(int_data + iter * 16 + 64);
        
        /* Two different masks */
        int32x16_t mask_a = compute_complex_mask(control_var);
        int32x16_t mask_b = compute_complex_mask(control_var + 7);
        
        /* Two different shuffle results */
        int32x16_t shuffle_a = __builtin_shuffle(*vec1, *vec2, mask_a);
        int32x16_t shuffle_b = __builtin_shuffle(*vec2, *vec3, mask_b);
        
        /* Conditional selection between shuffle results */
        int32x16_t selector;
        for (int i = 0; i < 16; i++) {
            ((int32_t*)&selector)[i] = (control_var > (i * 2)) ? -1 : 0;
        }
        
        /* Conditional permutation - may require many operands */
        int32x16_t result5 = (selector != 0) ? shuffle_a : shuffle_b;
        
        /* Additional shuffle with the conditional result */
        int32x16_t mask_c;
        for (int i = 0; i < 16; i++) {
            ((int32_t*)&mask_c)[i] = (i + control_var * 3) % 32;
        }
        int32x16_t result6 = __builtin_shuffle(result5, *vec1, mask_c);
        
        int_checksum += horizontal_sum_int32x16(result6);
        
        asm volatile("" ::: "memory");
    }
    
#ifdef __x86_64__
    /* KERNEL 4: Target-specific builtins and inline assembly for x86 */
    for (int iter = 0; iter < 10; iter++) {
        int32x8_t xmm_vec1 = *(int32x8_t*)(int_data + iter * 32);
        int32x8_t xmm_vec2 = *(int32x8_t*)(int_data + iter * 32 + 8);
        int32x8_t xmm_vec3 = *(int32x8_t*)(int_data + iter * 32 + 16);
        int32x8_t xmm_vec4 = *(int32x8_t*)(int_data + iter * 32 + 24);
        
        /* Use x86-specific shuffle intrinsic if available */
#ifdef __SSE4_1__
        /* This intrinsic typically takes multiple operands */
        __m128i v1 = *(const __m128i*)&xmm_vec1;
        __m128i v2 = *(const __m128i*)&xmm_vec2;
        __m128i v3 = *(const __m128i*)&xmm_vec3;
        __m128i v4 = *(const __m128i*)&xmm_vec4;
        
        /* Complex inline assembly with many operands */
        __m128i asm_result1, asm_result2;
        asm volatile (
            "vmovdqa %[v1], %%xmm0\n\t"
            "vmovdqa %[v2], %%xmm1\n\t"
            "vmovdqa %[v3], %%xmm2\n\t"
            "vmovdqa %[v4], %%xmm3\n\t"
            "vpshufd $0x1B, %%xmm0, %%xmm4\n\t"
            "vpshufd $0x39, %%xmm1, %%xmm5\n\t"
            "vpblendw $0xAA, %%xmm4, %%xmm5, %%xmm6\n\t"
            "vpshufd $0x93, %%xmm2, %%xmm7\n\t"
            "vpblendw $0x55, %%xmm6, %%xmm7, %%xmm0\n\t"
            "vmovdqa %%xmm0, %[out1]\n\t"
            "vpermq $0x4E, %%ymm2, %%ymm1\n\t"
            "vmovdqa %%xmm1, %[out2]\n\t"
            : [out1] "=x" (asm_result1), [out2] "=x" (asm_result2)
            : [v1] "x" (v1), [v2] "x" (v2), [v3] "x" (v3), [v4] "x" (v4)
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory"
        );
        
        int32x8_t result_vec1 = *(int32x8_t*)&asm_result1;
        int32x8_t result_vec2 = *(int32x8_t*)&asm_result2;
        
        int_checksum += horizontal_sum_int32x16(*(int32x16_t*)&result_vec1);
        int_checksum += horizontal_sum_int32x16(*(int32x16_t*)&result_vec2);
#endif
        asm volatile("" ::: "memory");
    }
#endif
    
    /* KERNEL 5: Large vector_size operations with __builtin_shufflevector */
    for (int iter = 0; iter < 5; iter++) {
        /* Create very large vectors using array sections */
        int32_t large_array[64];
        for (int i = 0; i < 64; i++) {
            large_array[i] = int_data[iter * 64 + i];
        }
        
        /* Cast to various vector sizes */
        typedef int32_t int32x32_t __attribute__((vector_size(128)));
        int32x32_t* large_vec1 = (int32x32_t*)large_array;
        int32x32_t* large_vec2 = (int32x32_t*)(large_array + 32);
        
        /* Complex operation that may require many operands during expansion */
        /* Note: __builtin_shufflevector can take many arguments */
        int32x32_t large_shuffle = __builtin_shufflevector(*large_vec1, *large_vec2,
            0, 32, 2, 34, 4, 36, 6, 38, 8, 40, 10, 42, 12, 44, 14, 46,
            16, 48, 18, 50, 20, 52, 22, 54, 24, 56, 26, 58, 28, 60, 30, 62);
        
        /* Store result */
        memcpy(large_array, &large_shuffle, sizeof(large_shuffle));
        
        /* Compute partial checksum */
        for (int i = 0; i < 32; i++) {
            int_checksum += ((int32_t*)&large_shuffle)[i];
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* Final checksum output to prevent dead code elimination */
    printf("Integer checksum: %d\n", int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
