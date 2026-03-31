/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force inline expansion */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* Target-specific includes */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#include <arm_acle.h>
#endif

/* GCC vector extensions for complex operations */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Complex multi-operand intrinsic usage */
#ifdef __AVX512F__
FORCE_INLINE __m512 test_avx512_10_operand(__m512 a, __m512 b, __m512 c, 
                                          __m512 d, __mmask16 k) {
    /* AVX-512 masked FMA with multiple operands */
    /* This should expand to pattern with mask, 3 source vectors, etc. */
    __m512 result = _mm512_mask_fmadd_ps(a, k, b, c);
    
    /* Additional complex operation with many operands */
    result = _mm512_mask_add_ps(result, k, result, d);
    
    /* Ternary operation with immediate */
    result = _mm512_maskz_mov_ps(k, result);
    
    return result;
}

FORCE_INLINE __m512i test_avx512_11_operand(__m512i a, __m512i b, __m512i c,
                                           __m512i d, __m512i e, __mmask16 k) {
    /* Complex permute/shuffle with many operands */
    __m512i temp = _mm512_mask_add_epi32(a, k, b, c);
    temp = _mm512_mask_slli_epi32(temp, k, temp, 3);
    temp = _mm512_mask_add_epi32(temp, k, temp, d);
    temp = _mm512_mask_add_epi32(temp, k, temp, e);
    
    /* Blend with immediate control */
    return _mm512_mask_blend_epi32(k, a, temp);
}
#endif

/* ARM NEON/SVE multi-operand patterns */
#ifdef __aarch64__
FORCE_INLINE int32x4_t test_neon_multi_operand(int32x4_t a, int32x4_t b,
                                              int32x4_t c, int32x4_t d,
                                              int32x4_t e, int32x4_t f) {
    /* Complex sequence that might use table lookup or multi-vector ops */
    int32x4_t r1 = vaddq_s32(a, b);
    int32x4_t r2 = vaddq_s32(c, d);
    int32x4_t r3 = vaddq_s32(e, f);
    
    /* Fused multiply-add style */
    r1 = vmlaq_s32(r1, r2, r3);
    
    /* Lane operations */
    r1 = vsetq_lane_s32(vgetq_lane_s32(r1, 0) + vgetq_lane_s32(r1, 1), r1, 0);
    r1 = vsetq_lane_s32(vgetq_lane_s32(r1, 2) + vgetq_lane_s32(r1, 3), r1, 1);
    
    return r1;
}
#endif

/* Complex inline assembly with many operands */
FORCE_INLINE uint64_t test_asm_11_operand(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t out1, out2, out3, out4, out5;
    
    /* Extended asm with 11 operands (5 outputs, 6 inputs) */
    asm volatile (
        "mov %[o1], %[a]\n\t"
        "add %[o1], %[o1], %[b]\n\t"
        "mov %[o2], %[c]\n\t"
        "add %[o2], %[o2], %[d]\n\t"
        "mov %[o3], %[e]\n\t"
        "add %[o3], %[o3], %[f]\n\t"
        "mov %[o4], %[g]\n\t"
        "add %[o4], %[o4], %[h]\n\t"
        "mov %[o5], %[i]\n\t"
        "add %[o5], %[o5], %[j]\n\t"
        "imul %[o1], %[o5]\n\t"
        "add %[o1], %[k]"
        : [o1] "=&r" (out1), [o2] "=&r" (out2), [o3] "=&r" (out3),
          [o4] "=&r" (out4), [o5] "=&r" (out5)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return out1 + out2 + out3 + out4 + out5;
}

/* GCC built-in complex math with many operands */
FORCE_INLINE double test_builtin_multi_operand(double a, double b, double c,
                                              double d, double e, double f,
                                              double g, double h) {
    /* Nested FMA operations - each FMA takes 3 operands */
    double r1 = __builtin_fma(a, b, c);
    double r2 = __builtin_fma(d, e, f);
    double r3 = __builtin_fma(g, h, r1);
    
    /* Complex expression preventing early folding */
    return __builtin_fma(r1, r2, r3) + 
           __builtin_fma(a, d, g) + 
           __builtin_fma(b, e, h);
}

/* Vector extension complex operations */
FORCE_INLINE v8sf test_vector_extensions(v8sf a, v8sf b, v8sf c, v8sf d,
                                        v8sf e, v8sf f) {
    /* Complex expression with many vector operands */
    v8sf r1 = a + b * c;
    v8sf r2 = d - e / f;
    v8sf r3 = r1 * r2 + a;
    v8sf r4 = b * c - d * e;
    
    /* Ternary-like operation using conditional */
    v8sf mask = a > b;
    v8sf result = (mask & r3) | (~mask & r4);
    
    return result + f;
}

/* OpenMP SIMD reduction with vector types */
void test_omp_reduction(float* restrict a, float* restrict b, 
                       float* restrict c, int n) {
    #pragma omp simd reduction(+:a[:n]) aligned(a:32) simdlen(8)
    for (int i = 0; i < n; i++) {
        /* Complex expression that might use FMA */
        a[i] = a[i] * b[i] + c[i];
        a[i] = a[i] * a[i] - b[i] * c[i];
    }
}

/* Hot function to encourage complex instruction patterns */
__attribute__((hot, noinline))
float complex_hot_function(float a, float b, float c, float d,
                          float e, float f, float g, float h,
                          float i, float j, float k) {
    /* Many parameter function that might use multi-operand patterns */
    float r1 = __builtin_fmaf(a, b, c);
    float r2 = __builtin_fmaf(d, e, f);
    float r3 = __builtin_fmaf(g, h, i);
    float r4 = __builtin_fmaf(j, k, r1);
    
    /* Complex reduction */
    return r1 + r2 + r3 + r4 + 
           __builtin_fmaf(r1, r2, r3) +
           __builtin_fmaf(a, d, g) +
           __builtin_fmaf(b, e, h);
}

/* Main test driver */
int main() {
    volatile float result = 0.0f;
    
    /* Test 1: Complex built-in math */
    result += test_builtin_multi_operand(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8);
    
    /* Test 2: Hot function with many parameters */
    result += complex_hot_function(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 
                                  7.0, 8.0, 9.0, 10.0, 11.0);
    
    /* Test 3: Inline assembly with many operands */
    result += test_asm_11_operand(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test 4: Vector extensions */
    v8sf va = {1, 2, 3, 4, 5, 6, 7, 8};
    v8sf vb = {2, 3, 4, 5, 6, 7, 8, 9};
    v8sf vc = {3, 4, 5, 6, 7, 8, 9, 10};
    v8sf vd = {4, 5, 6, 7, 8, 9, 10, 11};
    v8sf ve = {5, 6, 7, 8, 9, 10, 11, 12};
    v8sf vf = {6, 7, 8, 9, 10, 11, 12, 13};
    
    v8sf vresult = test_vector_extensions(va, vb, vc, vd, ve, vf);
    
    /* Extract result to prevent elimination */
    for (int i = 0; i < 8; i++) {
        result += vresult[i];
    }
    
    /* Test 5: OpenMP reduction */
    float arr_a[64], arr_b[64], arr_c[64];
    for (int i = 0; i < 64; i++) {
        arr_a[i] = i * 0.1f;
        arr_b[i] = i * 0.2f;
        arr_c[i] = i * 0.3f;
    }
    
    test_omp_reduction(arr_a, arr_b, arr_c, 64);
    result += arr_a[0] + arr_a[63];
    
    /* Target-specific tests */
#ifdef __AVX512F__
    /* Test AVX-512 multi-operand intrinsics */
    __m512 avx_a = _mm512_set1_ps(1.0f);
    __m512 avx_b = _mm512_set1_ps(2.0f);
    __m512 avx_c = _mm512_set1_ps(3.0f);
    __m512 avx_d = _mm512_set1_ps(4.0f);
    __mmask16 mask = 0xAAAA;
    
    __m512 avx_result = test_avx512_10_operand(avx_a, avx_b, avx_c, avx_d, mask);
    
    __m512i avxi_a = _mm512_set1_epi32(1);
    __m512i avxi_b = _mm512_set1_epi32(2);
    __m512i avxi_c = _mm512_set1_epi32(3);
    __m512i avxi_d = _mm512_set1_epi32(4);
    __m512i avxi_e = _mm512_set1_epi32(5);
    
    __m512i avxi_result = test_avx512_11_operand(avxi_a, avxi_b, avxi_c, 
                                                avxi_d, avxi_e, mask);
    
    /* Extract some values to prevent elimination */
    float temp[16];
    _mm512_storeu_ps(temp, avx_result);
    result += temp[0] + temp[15];
#endif
    
#ifdef __aarch64__
    /* Test ARM NEON multi-operand */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    int32x4_t neon_f = vdupq_n_s32(6);
    
    int32x4_t neon_result = test_neon_multi_operand(neon_a, neon_b, neon_c,
                                                   neon_d, neon_e, neon_f);
    
    int32_t temp_neon[4];
    vst1q_s32(temp_neon, neon_result);
    result += temp_neon[0] + temp_neon[3];
#endif
    
    printf("Result: %f\n", result);
    return (result > 0) ? 0 : 1;
}
