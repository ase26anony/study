/* Test program to cover 10 and 11-operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Enable target-specific intrinsics based on architecture */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#include <arm_acle.h>
#endif

/* Force inlining for all target functions */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* GCC vector extensions for complex operations */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Complex reduction using GCC vector extensions */
FORCE_INLINE v8sf complex_vector_operation(v8sf a, v8sf b, v8sf c, v8sf d) {
    /* Complex expression that might expand to multi-operand instructions */
    return a * b + c * d + (a + b) * (c - d) + a * c + b * d;
}

/* Test case 1: AVX-512 mask operations (x86 specific) */
#ifdef __AVX512F__
FORCE_INLINE __m512 test_avx512_10_operand(__m512 a, __m512 b, __m512 c, 
                                          __m512 d, __m512 e, __mmask16 k) {
    /* AVX-512 masked FMA with multiple operations - potentially expands to many operands */
    __m512 result;
    
    /* Use multiple masked operations in sequence */
    result = _mm512_mask_add_ps(a, k, b, c);
    result = _mm512_mask_mul_ps(result, k, d, e);
    result = _mm512_mask_fmadd_ps(result, k, a, b, c);
    result = _mm512_mask_fnmadd_ps(result, k, d, e, a);
    
    /* Complex expression that might require many operands during expansion */
    result = _mm512_mask_add_ps(
        _mm512_mask_mul_ps(a, k, b, c),
        k,
        _mm512_mask_sub_ps(d, k, e, a),
        _mm512_mask_mul_ps(b, k, c, d)
    );
    
    return result;
}

FORCE_INLINE __m512i test_avx512_11_operand(__m512i a, __m512i b, __m512i c,
                                           __m512i d, __m512i e, __m512i f,
                                           __mmask16 k1, __mmask16 k2) {
    /* Even more complex operation with multiple masks */
    __m512i temp1, temp2, result;
    
    /* This complex sequence might expand to 11-operand patterns */
    temp1 = _mm512_mask_add_epi32(a, k1, b, c);
    temp2 = _mm512_mask_sub_epi32(d, k2, e, f);
    
    /* Blend with two masks - potentially many operands */
    result = _mm512_mask_blend_epi32(k1, temp1, temp2);
    result = _mm512_mask_add_epi32(result, k2, result, a);
    
    return result;
}
#endif

/* Test case 2: ARM NEON/SVE operations (aarch64 specific) */
#ifdef __aarch64__
FORCE_INLINE float32x4_t test_neon_10_operand(float32x4_t a, float32x4_t b,
                                             float32x4_t c, float32x4_t d,
                                             float32x4_t e) {
    /* Complex NEON operations that might expand to many operands */
    float32x4_t result;
    
    /* Fused multiply-add operations */
    result = vfmaq_f32(a, b, c);
    result = vfmsq_f32(result, d, e);
    result = vmlaq_f32(result, a, b);
    result = vmlsq_f32(result, c, d);
    
    /* Complex expression */
    result = vaddq_f32(
        vmulq_f32(a, b),
        vaddq_f32(
            vmulq_f32(c, d),
            vmulq_f32(vaddq_f32(a, b), vsubq_f32(c, d))
        )
    );
    
    return result;
}

/* Using multiple vector load/store with lane operations */
FORCE_INLINE void test_neon_multi_lane(int8_t* data, int8x16x4_t* result) {
    /* vld4 loads 4 vectors interleaved - might expand to many operands */
    int8x16x4_t loaded = vld4q_s8(data);
    
    /* Perform operations on all 4 vectors */
    loaded.val[0] = vaddq_s8(loaded.val[0], loaded.val[1]);
    loaded.val[1] = vsubq_s8(loaded.val[1], loaded.val[2]);
    loaded.val[2] = vmulq_s8(loaded.val[2], loaded.val[3]);
    loaded.val[3] = vaddq_s8(loaded.val[3], loaded.val[0]);
    
    *result = loaded;
}
#endif

/* Test case 3: Inline assembly with many operands (architecture independent) */
FORCE_INLINE uint64_t test_inline_asm_11_operand(uint64_t a, uint64_t b,
                                                uint64_t c, uint64_t d,
                                                uint64_t e, uint64_t f,
                                                uint64_t g, uint64_t h,
                                                uint64_t i, uint64_t j) {
    uint64_t result1, result2, result3, result4, result5;
    
    /* Inline assembly with 11 operands (10 inputs + 1 output) */
    __asm__ volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "add %[r3], %[e], %[f]\n\t"
        "add %[r4], %[g], %[h]\n\t"
        "mul %[r5], %[i], %[j]\n\t"
        "add %[r1], %[r1], %[r2]\n\t"
        "add %[r3], %[r3], %[r4]\n\t"
        "add %[out], %[r1], %[r3]\n\t"
        "add %[out], %[out], %[r5]"
        : [out] "=r" (result1),
          [r1] "=&r" (result2),
          [r2] "=&r" (result3),
          [r3] "=&r" (result4),
          [r5] "=&r" (result5)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1;
}

/* Test case 4: Built-in math functions with complex expressions */
FORCE_INLINE double test_builtin_fma_chain(double a, double b, double c,
                                          double d, double e, double f,
                                          double g, double h, double i,
                                          double j, double k) {
    /* Chain of FMA operations - each expands to a 3-operand instruction */
    double result;
    
    /* Complex expression that might be optimized into multi-operand pattern */
    result = __builtin_fma(a, b, c);
    result = __builtin_fma(result, d, e);
    result = __builtin_fma(f, g, result);
    result = __builtin_fma(h, i, result);
    result = __builtin_fma(j, k, result);
    
    /* Nested FMA calls that might be combined */
    result = __builtin_fma(
        __builtin_fma(a, b, c),
        __builtin_fma(d, e, f),
        __builtin_fma(g, h, __builtin_fma(i, j, k))
    );
    
    return result;
}

/* Test case 5: OpenMP SIMD reduction with vector types */
void test_omp_simd_reduction(float* array, int n, float* result) {
    v8sf sum = {0, 0, 0, 0, 0, 0, 0, 0};
    
    #pragma omp simd reduction(+:sum) aligned(array:32)
    for (int i = 0; i < n; i += 8) {
        v8sf chunk = *(v8sf*)(array + i);
        sum = sum + chunk * chunk + (chunk + chunk) * (chunk - chunk);
    }
    
    /* Store result */
    *(v8sf*)result = sum;
}

/* Main test function */
int main() {
    volatile int test_result = 0;
    
    /* Test 1: GCC vector extensions */
    {
        v8sf a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
        v8sf b = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
        v8sf c = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
        v8sf d = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
        
        v8sf result = complex_vector_operation(a, b, c, d);
        test_result += (int)result[0];
    }
    
    /* Test 2: Inline assembly with many operands */
    {
        uint64_t asm_result = test_inline_asm_11_operand(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        test_result += (int)asm_result;
    }
    
    /* Test 3: Built-in FMA chain */
    {
        double fma_result = test_builtin_fma_chain(1.0, 2.0, 3.0, 4.0, 5.0,
                                                  6.0, 7.0, 8.0, 9.0, 10.0, 11.0);
        test_result += (int)fma_result;
    }
    
    /* Test 4: OpenMP SIMD reduction */
    {
        float array[64];
        float result[8];
        
        for (int i = 0; i < 64; i++) {
            array[i] = (float)i;
        }
        
        test_omp_simd_reduction(array, 64, result);
        test_result += (int)result[0];
    }
    
    /* Target-specific tests */
    #ifdef __AVX512F__
    {
        __m512 avx_a = _mm512_set1_ps(1.0f);
        __m512 avx_b = _mm512_set1_ps(2.0f);
        __m512 avx_c = _mm512_set1_ps(3.0f);
        __m512 avx_d = _mm512_set1_ps(4.0f);
        __m512 avx_e = _mm512_set1_ps(5.0f);
        __mmask16 mask = 0xFFFF;
        
        __m512 avx_result = test_avx512_10_operand(avx_a, avx_b, avx_c, avx_d, avx_e, mask);
        float temp[16];
        _mm512_storeu_ps(temp, avx_result);
        test_result += (int)temp[0];
    }
    #endif
    
    #ifdef __aarch64__
    {
        float32x4_t neon_a = {1.0f, 2.0f, 3.0f, 4.0f};
        float32x4_t neon_b = {2.0f, 3.0f, 4.0f, 5.0f};
        float32x4_t neon_c = {3.0f, 4.0f, 5.0f, 6.0f};
        float32x4_t neon_d = {4.0f, 5.0f, 6.0f, 7.0f};
        float32x4_t neon_e = {5.0f, 6.0f, 7.0f, 8.0f};
        
        float32x4_t neon_result = test_neon_10_operand(neon_a, neon_b, neon_c, neon_d, neon_e);
        test_result += (int)neon_result[0];
    }
    #endif
    
    printf("Test result: %d\n", test_result);
    return test_result == 0 ? 0 : 1;
}
