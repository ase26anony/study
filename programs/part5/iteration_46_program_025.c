#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Simple PRNG for deterministic testing */
static uint32_t seed = 123456789;
static inline uint32_t prng_u32(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with pseudo-random data */
#define ARRAY_SIZE 1024
#define VEC_SIZE 8  /* For AVX2 256-bit vectors (8 floats) */

static float input1[ARRAY_SIZE] __attribute__((aligned(32)));
static float input2[ARRAY_SIZE] __attribute__((aligned(32)));
static float input3[ARRAY_SIZE] __attribute__((aligned(32)));
static float input4[ARRAY_SIZE] __attribute__((aligned(32)));
static float output[ARRAY_SIZE] __attribute__((aligned(32)));

/* Volatile counter to prevent loop unrolling */
static volatile int volatile_counter = 0;

/* Function with many arguments - designed to trigger optab expansion */
__attribute__((noinline, target("avx2,avx512f")))
static void test_many_args(float* out, const float* in1, const float* in2, 
                          const float* in3, const float* in4, int count) {
    
    /* Force many intermediate temporaries */
    for (int i = 0; i < count; i += VEC_SIZE) {
        volatile_counter++;
        
        /* Load multiple vectors */
        __m256 v1 = _mm256_load_ps(&in1[i]);
        __m256 v2 = _mm256_load_ps(&in2[i]);
        __m256 v3 = _mm256_load_ps(&in3[i]);
        __m256 v4 = _mm256_load_ps(&in4[i]);
        
        /* Complex expression with many temporaries */
        __m256 t1 = _mm256_add_ps(v1, v2);
        __m256 t2 = _mm256_sub_ps(v3, v4);
        __m256 t3 = _mm256_mul_ps(t1, t2);
        __m256 t4 = _mm256_fmadd_ps(v1, v2, v3);
        __m256 t5 = _mm256_fnmadd_ps(v2, v3, v4);
        
        /* Create a complex mask using multiple operations */
        __m256 mask1 = _mm256_cmp_ps(t1, t2, _CMP_GT_OQ);
        __m256 mask2 = _mm256_cmp_ps(t3, t4, _CMP_LT_OQ);
        __m256 mask3 = _mm256_and_ps(mask1, mask2);
        
        /* Blend with many arguments - could expand to optab with many args */
        __m256 blended = _mm256_blendv_ps(t1, t2, mask3);
        
        /* Additional operations to create more temporaries */
        __m256 t6 = _mm256_sqrt_ps(_mm256_add_ps(blended, t3));
        __m256 t7 = _mm256_rcp_ps(_mm256_add_ps(t4, t5));
        
        /* Final blend with many control inputs */
        __m256 result = _mm256_blendv_ps(t6, t7, mask3);
        
        /* Store result */
        _mm256_store_ps(&out[i], result);
    }
}

/* Alternative approach using inline asm with many operands */
__attribute__((noinline, target("avx2")))
static void test_many_args_asm(float* out, const float* in1, const float* in2,
                              const float* in3, const float* in4, int count) {
    
    for (int i = 0; i < count; i += VEC_SIZE) {
        volatile_counter++;
        
        /* Load vectors */
        __m256 v1, v2, v3, v4;
        __m256 result;
        
        /* Complex inline asm with many operands - designed to trigger optab */
        asm volatile (
            "vmovaps %1, %%ymm0\n\t"
            "vmovaps %2, %%ymm1\n\t"
            "vmovaps %3, %%ymm2\n\t"
            "vmovaps %4, %%ymm3\n\t"
            "vaddps %%ymm0, %%ymm1, %%ymm4\n\t"
            "vsubps %%ymm2, %%ymm3, %%ymm5\n\t"
            "vmulps %%ymm4, %%ymm5, %%ymm6\n\t"
            "vfmadd231ps %%ymm0, %%ymm1, %%ymm2\n\t"
            "vfnmadd231ps %%ymm1, %%ymm2, %%ymm3\n\t"
            "vcmpps $1, %%ymm4, %%ymm5, %%ymm7\n\t"
            "vblendvps %%ymm7, %%ymm4, %%ymm5, %%ymm0\n\t"
            "vsqrtps %%ymm0, %%ymm1\n\t"
            "vmovaps %%ymm1, %0\n\t"
            : "=m" (result)
            : "m" (in1[i]), "m" (in2[i]), "m" (in3[i]), "m" (in4[i]),
              "r" (i), "r" (count), "r" (volatile_counter)
            : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", 
              "ymm6", "ymm7", "memory"
        );
        
        _mm256_store_ps(&out[i], result);
    }
}

/* Function using GCC vector builtins with many arguments */
typedef float v8sf __attribute__((vector_size(32)));

__attribute__((noinline, target("avx2")))
static void test_vector_builtins(float* out, const float* in1, const float* in2,
                                const float* in3, const float* in4, int count) {
    
    for (int i = 0; i < count; i += VEC_SIZE) {
        volatile_counter++;
        
        /* Load as vector types */
        v8sf v1 = *(const v8sf*)&in1[i];
        v8sf v2 = *(const v8sf*)&in2[i];
        v8sf v3 = *(const v8sf*)&in3[i];
        v8sf v4 = *(const v8sf*)&in4[i];
        
        /* Complex expression tree with many arguments */
        v8sf t1 = v1 + v2;
        v8sf t2 = v3 - v4;
        v8sf t3 = t1 * t2;
        v8sf t4 = __builtin_ia32_vfmaddps256(v1, v2, v3);
        v8sf t5 = __builtin_ia32_vfnmaddps256(v2, v3, v4);
        
        /* Create comparison results */
        v8sf cmp1 = t1 > t2;
        v8sf cmp2 = t3 < t4;
        v8sf mask = cmp1 & cmp2;
        
        /* Use __builtin_shuffle with complex pattern - could use many args */
        int indices[8] = {0, 2, 4, 6, 1, 3, 5, 7};
        v8sf shuffled = __builtin_shuffle(t1, t2, 
            indices[0], indices[1], indices[2], indices[3],
            indices[4], indices[5], indices[6], indices[7]);
        
        /* Final blend operation */
        v8sf result = __builtin_ia32_blendvps256(shuffled, t3, mask);
        
        /* Store result */
        *(v8sf*)&out[i] = result;
    }
}

/* Complex multi-statement expression with many temporaries */
__attribute__((noinline))
static float complex_expression(float a, float b, float c, float d,
                               float e, float f, float g, float h,
                               float i, float j, float k) {
    /* Force many intermediate calculations */
    float t1 = a + b;
    float t2 = c - d;
    float t3 = e * f;
    float t4 = g / h;
    float t5 = t1 * t2;
    float t6 = t3 + t4;
    float t7 = t5 - t6;
    float t8 = i * j;
    float t9 = k + t7;
    float t10 = t8 / t9;
    
    /* Use inline asm to prevent optimization */
    asm volatile("" : "+r" (t1), "+r" (t2), "+r" (t3), "+r" (t4));
    
    return t10 + t1 + t2 + t3 + t4;
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input1[i] = (float)prng_u32() / (float)UINT32_MAX;
        input2[i] = (float)prng_u32() / (float)UINT32_MAX;
        input3[i] = (float)prng_u32() / (float)UINT32_MAX;
        input4[i] = (float)prng_u32() / (float)UINT32_MAX;
    }
    
    /* Clear output */
    memset(output, 0, sizeof(output));
    
    /* Test different approaches */
    printf("Testing many-argument optab expansion...\n");
    
    /* Test 1: Vector operations with many arguments */
    test_many_args(output, input1, input2, input3, input4, ARRAY_SIZE);
    
    /* Test 2: Inline asm with many operands */
    test_many_args_asm(output, input1, input2, input3, input4, ARRAY_SIZE);
    
    /* Test 3: GCC vector builtins */
    test_vector_builtins(output, input1, input2, input3, input4, ARRAY_SIZE);
    
    /* Test 4: Complex scalar expression with 11 arguments */
    float checksum = 0.0f;
    for (int i = 0; i < 11; i++) {
        checksum += complex_expression(
            input1[i], input2[i], input3[i], input4[i],
            input1[i+1], input2[i+1], input3[i+1], input4[i+1],
            input1[i+2], input2[i+2], input3[i+2]
        );
    }
    
    /* Compute final checksum */
    float final_sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += output[i];
    }
    
    printf("Final checksum: %f\n", final_sum + checksum);
    printf("Volatile counter: %d\n", volatile_counter);
    
    return 0;
}
