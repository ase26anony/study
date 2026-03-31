/* Test program to trigger 10/11-operand instruction expansion in GCC optabs */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define TARGET_X86 1
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#define TARGET_ARM 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc64__) || defined(__powerpc__)
#define TARGET_PPC 1
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression_10_operands(int a, int b, int c, int d, 
                                                  int e, int f, int g, int h,
                                                  int i, int j) {
    /* Expression designed to potentially combine into a single instruction */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Another complex expression with 11 operands */
static inline int complex_expression_11_operands(int a, int b, int c, int d,
                                                  int e, int f, int g, int h,
                                                  int i, int j, int k) {
    return a * b + c * d + e * f + g * h + i * j + k;
}

/* Function using inline assembly with exactly 11 operands */
static inline uint64_t inline_asm_11_operands(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h, uint64_t i,
                                              uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with 11 operands to force optabs expansion */
    __asm__ volatile (
        /* Template doesn't matter much - we just need 11 operands */
        "add %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]\n\t"
        "add %[res], %[res], %[k]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

/* Function using atomic built-in with many parameters */
static inline int atomic_compare_exchange_many_args(int *ptr, int *expected, 
                                                     int desired, int weak,
                                                     int success_memorder,
                                                     int failure_memorder) {
    /* __atomic_compare_exchange has 6 parameters, which might expand further */
    return __atomic_compare_exchange(ptr, expected, &desired, weak,
                                     success_memorder, failure_memorder);
}

#if TARGET_X86
/* x86 AVX-512 intrinsics that use many operands */
static inline __m512i avx512_multi_operand_intrinsic(__m512i a, __m512i b,
                                                     __m512i c, __m512i d,
                                                     __m512i e, __m512i f,
                                                     __mmask16 k) {
    /* Chain multiple operations that might combine */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(e, f);
    
    /* Use masked operation with multiple operands */
    return _mm512_mask_add_epi32(t1, k, t2, t3);
}

/* AVX-512 FMA with rounding control - up to 6 operands */
static inline __m512d avx512_fma_multi_operand(__m512d a, __m512d b, __m512d c,
                                               __m512d d, __m512d e, __m512d f,
                                               __mmask8 k) {
    /* Chain FMA operations */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    
    /* Masked operation */
    return _mm512_mask_mul_pd(t1, k, t2, _mm512_set1_pd(2.0));
}
#endif

#if TARGET_ARM
/* ARM NEON/SVE intrinsics with many operands */
#ifdef __ARM_FEATURE_SVE
/* SVE2 multi-operand intrinsic simulation */
static inline svint32_t sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                          svint32_t d, svint32_t e, svint32_t f,
                                          svint32_t g, svint32_t h) {
    /* Complex expression with SVE intrinsics */
    svint32_t t1 = svmla_s32_x(svptrue_b32(), a, b, c);
    svint32_t t2 = svmla_s32_x(svptrue_b32(), d, e, f);
    return svadd_s32_x(svptrue_b32(), t1, svadd_s32_x(svptrue_b32(), t2, g));
}
#endif

/* ARM NEON multi-operand operations */
static inline int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                           int32x4_t d, int32x4_t e, int32x4_t f) {
    /* vmlaq_lane can use multiple operands */
    int32x4_t t1 = vmlaq_laneq_s32(a, b, c, 0);
    int32x4_t t2 = vmlaq_laneq_s32(d, e, f, 1);
    return vaddq_s32(t1, t2);
}
#endif

#if TARGET_PPC
/* PowerPC Altivec/VSX multi-operand operations */
static inline vector signed int ppc_multi_operand(vector signed int a,
                                                  vector signed int b,
                                                  vector signed int c,
                                                  vector signed int d,
                                                  vector signed int e,
                                                  vector signed int f) {
    /* Complex permute and compute */
    vector signed int t1 = vec_madd(a, b, c);
    vector signed int t2 = vec_madd(d, e, f);
    return vec_add(t1, t2);
}
#endif

/* Main driver function */
int main() {
    int result = 0;
    
    /* Test complex expressions with 10 and 11 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    
    result += complex_expression_10_operands(a, b, c, d, e, f, g, h, i, j);
    result += complex_expression_11_operands(a, b, c, d, e, f, g, h, i, j, k);
    
    /* Test inline assembly with 11 operands */
    uint64_t asm_result = inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    
    /* Test atomic operation with many parameters */
    int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    if (atomic_compare_exchange_many_args(&atomic_var, &expected, desired,
                                          0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        result += atomic_var;
    }
    
#if TARGET_X86
    /* Test AVX-512 intrinsics */
    if (__builtin_cpu_supports("avx512f")) {
        __m512i avx_vec1 = _mm512_set1_epi32(1);
        __m512i avx_vec2 = _mm512_set1_epi32(2);
        __m512i avx_vec3 = _mm512_set1_epi32(3);
        __m512i avx_vec4 = _mm512_set1_epi32(4);
        __m512i avx_vec5 = _mm512_set1_epi32(5);
        __m512i avx_vec6 = _mm512_set1_epi32(6);
        
        __m512i avx_result = avx512_multi_operand_intrinsic(avx_vec1, avx_vec2,
                                                            avx_vec3, avx_vec4,
                                                            avx_vec5, avx_vec6,
                                                            0xFFFF);
        
        /* Extract one element to prevent dead code elimination */
        result += _mm512_extract_epi32(avx_result, 0);
    }
#endif
    
#if TARGET_ARM
    /* Test NEON intrinsics */
    int32x4_t neon_vec1 = vdupq_n_s32(1);
    int32x4_t neon_vec2 = vdupq_n_s32(2);
    int32x4_t neon_vec3 = vdupq_n_s32(3);
    int32x4_t neon_vec4 = vdupq_n_s32(4);
    int32x4_t neon_vec5 = vdupq_n_s32(5);
    int32x4_t neon_vec6 = vdupq_n_s32(6);
    
    int32x4_t neon_result = neon_multi_operand(neon_vec1, neon_vec2, neon_vec3,
                                               neon_vec4, neon_vec5, neon_vec6);
    
    result += vgetq_lane_s32(neon_result, 0);
#endif
    
    /* Additional complex expression that might trigger combine pass */
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5, x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    int complex_result = x1 * x2 + x3 * x4 + x5 * x6 + x7 * x8 + x9 * x10;
    result += complex_result;
    
    /* Another expression with 11 variables */
    int y1 = 1, y2 = 2, y3 = 3, y4 = 4, y5 = 5, y6 = 6, y7 = 7, y8 = 8, y9 = 9, y10 = 10, y11 = 11;
    int complex_result2 = y1 + y2 * y3 - y4 / y5 + y6 % y7 + y8 & y9 | y10 ^ y11;
    result += complex_result2;
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
