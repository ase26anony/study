#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define various vector types */
typedef int32_t int32x16_t __attribute__((vector_size(64)));
typedef int32_t int32x8_t __attribute__((vector_size(32)));
typedef int64_t int64x8_t __attribute__((vector_size(64)));
typedef float float32x16_t __attribute__((vector_size(64)));
typedef double float64x8_t __attribute__((vector_size(64)));

/* Simple LCG for pseudo-random data */
static uint32_t lcg_state = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random data */
static void init_array(void *arr, size_t size) {
    uint32_t *ptr = (uint32_t *)arr;
    size_t count = size / sizeof(uint32_t);
    for (size_t i = 0; i < count; i++) {
        ptr[i] = lcg_rand();
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

static double horizontal_sum_float64x8(float64x8_t v) {
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += v[i];
    }
    return sum;
}

int main(void) {
    /* Large arrays for vector operations */
    int32_t arr_int32[256];
    float arr_float[256];
    double arr_double[128];
    
    /* Initialize arrays */
    init_array(arr_int32, sizeof(arr_int32));
    init_array(arr_float, sizeof(arr_float));
    init_array(arr_double, sizeof(arr_double));
    
    /* Volatile variables to prevent constant propagation */
    volatile int shuffle_mode = lcg_rand() % 4;
    volatile int mask_offset = lcg_rand() % 8;
    
    int32_t checksum = 0;
    
    /* Kernel 1: Complex shuffle with computed mask vector */
    {
        /* Load vectors from arrays */
        int32x16_t vec_a = *(int32x16_t *)&arr_int32[0];
        int32x16_t vec_b = *(int32x16_t *)&arr_int32[16];
        
        /* Create a computed mask vector - non-constant to force runtime expansion */
        int32x16_t mask;
        for (int i = 0; i < 16; i++) {
            mask[i] = (i + mask_offset + shuffle_mode) % 32;
        }
        
        /* Complex shuffle operation that may require many operands during expansion */
        int32x16_t result1 = __builtin_shuffle(vec_a, vec_b, mask);
        
        /* Chain another shuffle using the result */
        int32x16_t mask2;
        for (int i = 0; i < 16; i++) {
            mask2[i] = (i * 3 + mask_offset) % 32;
        }
        int32x16_t result2 = __builtin_shuffle(result1, vec_b, mask2);
        
        checksum += horizontal_sum_int32x16(result2);
        
        /* Compiler barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 2: Mixed type permutations */
    {
        float32x16_t fvec_a = *(float32x16_t *)&arr_float[0];
        float32x16_t fvec_b = *(float32x16_t *)&arr_float[16];
        
        /* Create a complex mask using arithmetic */
        int32x16_t fmask;
        for (int i = 0; i < 16; i++) {
            fmask[i] = (i * 5 + mask_offset * 2) % 32;
        }
        
        /* Shuffle with float vectors using int mask */
        float32x16_t fresult = __builtin_shuffle(fvec_a, fvec_b, fmask);
        
        /* Convert and mix with integer vectors */
        int32x16_t iconv = *(int32x16_t *)&fresult;
        int32x16_t ivec = *(int32x16_t *)&arr_int32[32];
        
        /* Another shuffle chain */
        int32x16_t mask3;
        for (int i = 0; i < 16; i++) {
            mask3[i] = (i + shuffle_mode * 7) % 32;
        }
        
        int32x16_t mixed_result = __builtin_shuffle(iconv, ivec, mask3);
        checksum += horizontal_sum_int32x16(mixed_result);
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 3: Conditional vector permutations */
    {
        int64x8_t vec64_a = *(int64x8_t *)&arr_int32[0];  /* Reinterpret as 64-bit */
        int64x8_t vec64_b = *(int64x8_t *)&arr_int32[8];
        
        /* Create two different masks conditionally */
        int64x8_t mask_a, mask_b;
        for (int i = 0; i < 8; i++) {
            mask_a[i] = (i + mask_offset) % 16;
            mask_b[i] = (i * 2 + mask_offset) % 16;
        }
        
        /* Conditional selection of shuffle result */
        int64x8_t shuffle_a = __builtin_shuffle(vec64_a, vec64_b, mask_a);
        int64x8_t shuffle_b = __builtin_shuffle(vec64_a, vec64_b, mask_b);
        
        /* Use volatile condition to prevent optimization */
        volatile int cond = shuffle_mode > 2;
        int64x8_t selected = cond ? shuffle_a : shuffle_b;
        
        /* Chain more operations */
        int64x8_t mask_c;
        for (int i = 0; i < 8; i++) {
            mask_c[i] = (i * 3 + 1) % 16;
        }
        
        int64x8_t final_result = __builtin_shuffle(selected, vec64_a, mask_c);
        
        /* Add to checksum */
        for (int i = 0; i < 8; i++) {
            checksum += (int32_t)final_result[i];
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 4: Inline assembly with many vector operands */
    {
        float64x8_t dvec1 = *(float64x8_t *)&arr_double[0];
        float64x8_t dvec2 = *(float64x8_t *)&arr_double[8];
        float64x8_t dvec3 = *(float64x8_t *)&arr_double[16];
        
        /* Complex inline assembly that may require many operand handling */
        float64x8_t asm_result1, asm_result2;
        
        /* This inline assembly uses multiple vector inputs/outputs */
        asm volatile(
            /* Hypothetical multi-operand vector operation */
            "# Complex vector operation requiring many operands\n"
            "vmovdqa %[vec1], %%ymm0\n"
            "vmovdqa %[vec2], %%ymm1\n"
            "vmovdqa %[vec3], %%ymm2\n"
            "# Additional processing would go here\n"
            "vmovdqa %%ymm0, %[out1]\n"
            "vmovdqa %%ymm1, %[out2]\n"
            : [out1] "=v" (asm_result1),
              [out2] "=v" (asm_result2)
            : [vec1] "v" (dvec1),
              [vec2] "v" (dvec2),
              [vec3] "v" (dvec3)
            : "ymm0", "ymm1", "ymm2", "memory"
        );
        
        checksum += (int32_t)horizontal_sum_float64x8(asm_result1);
        checksum += (int32_t)horizontal_sum_float64x8(asm_result2);
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 5: Architecture-specific builtins (conditional compilation) */
    {
        int32x8_t vec_sse1 = *(int32x8_t *)&arr_int32[64];
        int32x8_t vec_sse2 = *(int32x8_t *)&arr_int32[72];
        
#ifdef __x86_64__
        /* Use x86-specific shuffle intrinsics */
        int32x8_t sse_result;
        
        /* This builtin typically takes multiple operands */
        asm volatile(
            "pshufd $0x1B, %[in1], %[out]\n"
            : [out] "=x" (sse_result)
            : [in1] "x" (vec_sse1)
        );
        
        /* Chain with another operation */
        int32x8_t final_sse;
        asm volatile(
            "paddd %[in1], %[in2], %[out]\n"
            : [out] "=x" (final_sse)
            : [in1] "x" (sse_result),
              [in2] "x" (vec_sse2)
        );
        
        for (int i = 0; i < 8; i++) {
            checksum += final_sse[i];
        }
#elif defined(__aarch64__)
        /* ARM NEON builtins */
        int32x8_t neon_result;
        
        /* Use ARM-specific shuffle/reverse */
        asm volatile(
            "rev64 %[out].16b, %[in].16b\n"
            : [out] "=w" (neon_result)
            : [in] "w" (vec_sse1)
        );
        
        for (int i = 0; i < 8; i++) {
            checksum += neon_result[i];
        }
#endif
        
        asm volatile("" ::: "memory");
    }
    
    /* Kernel 6: Loop-dependent vector operations */
    {
        int32x16_t accum = {0};
        
        /* Loop with data-dependent shuffles */
        for (int iter = 0; iter < 4; iter++) {
            int32x16_t loop_vec_a = *(int32x16_t *)&arr_int32[iter * 16];
            int32x16_t loop_vec_b = *(int32x16_t *)&arr_int32[iter * 16 + 16];
            
            /* Create mask based on loop iteration and volatile variable */
            int32x16_t loop_mask;
            for (int i = 0; i < 16; i++) {
                loop_mask[i] = (i + iter + mask_offset) % 32;
            }
            
            /* Data-dependent shuffle */
            int32x16_t loop_result = __builtin_shuffle(loop_vec_a, loop_vec_b, loop_mask);
            
            /* Accumulate results */
            accum += loop_result;
        }
        
        checksum += horizontal_sum_int32x16(accum);
    }
    
    /* Final checksum output to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
