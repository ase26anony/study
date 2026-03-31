/* test_multi_operand.c - Test program to trigger 10/11-operand expansion in GCC optabs */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define X86_ARCH 1
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#define ARM_ARCH 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc__) || defined(__PPC__)
#define PPC_ARCH 1
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* Fused multiply-add chain that could be combined */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with exactly 11 operands */
static inline uint64_t inline_asm_11_operands(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h, uint64_t i,
                                              uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with 11 operands to force expansion */
    __asm__ volatile (
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

/* Function using inline assembly with exactly 10 operands */
static inline uint64_t inline_asm_10_operands(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h, uint64_t i,
                                              uint64_t j) {
    uint64_t result;
    
    __asm__ volatile (
        "mul %[res], %[a], %[b]\n\t"
        "mla %[res], %[c], %[d], %[res]\n\t"
        "mla %[res], %[e], %[f], %[res]\n\t"
        "mla %[res], %[g], %[h], %[res]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}

#ifdef X86_ARCH
/* AVX-512 masked operation with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __mmask8 k,
                                           int rounding) {
    /* This could expand to a multi-operand instruction with:
       - 3 source vector registers (a, b, c)
       - 1 destination vector register
       - 1 mask register (k)
       - 1 rounding control (rounding)
       - Additional operands for memory addressing modes
    */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, t1);
    
    /* Masked store with many implicit operands */
    __m512d result = _mm512_mask_blend_pd(k, t1, t2);
    
    return result;
}

/* Complex AVX-512 expression */
static inline __m512i avx512_complex_expression(__m512i a, __m512i b, __m512i c,
                                                __m512i d, __m512i e, __m512i f,
                                                __m512i g, __m512i h, __m512i i,
                                                __m512i j) {
    /* Chain of operations that might combine */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(e, f);
    __m512i t4 = _mm512_add_epi32(g, h);
    __m512i t5 = _mm512_add_epi32(i, j);
    
    __m512i sum1 = _mm512_add_epi32(t1, t2);
    __m512i sum2 = _mm512_add_epi32(t3, t4);
    __m512i sum3 = _mm512_add_epi32(sum1, sum2);
    
    return _mm512_add_epi32(sum3, t5);
}
#endif

#ifdef ARM_ARCH
#ifdef __ARM_FEATURE_SVE
/* SVE2 multi-operand intrinsic simulation */
static inline svint32_t sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                          svint32_t d, svint32_t e, svint32_t f,
                                          svint32_t g, svint32_t h, svint32_t i,
                                          svint32_t j) {
    /* Simulate complex SVE operation with many operands */
    svint32_t t1 = svmla_s32(a, b, c);
    svint32_t t2 = svmla_s32(d, e, f);
    svint32_t t3 = svadd_s32(t1, t2);
    svint32_t t4 = svmla_s32(g, h, i);
    
    return svadd_s32(t3, svadd_s32(t4, j));
}
#endif

/* NEON complex expression */
static inline int32x4_t neon_complex_expression(int32x4_t a, int32x4_t b, int32x4_t c,
                                                int32x4_t d, int32x4_t e, int32x4_t f,
                                                int32x4_t g, int32x4_t h, int32x4_t i,
                                                int32x4_t j) {
    /* Multi-operand expression */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    int32x4_t t3 = vaddq_s32(t1, t2);
    int32x4_t t4 = vmlaq_s32(g, h, i);
    
    return vaddq_s32(t3, vaddq_s32(t4, j));
}
#endif

#ifdef PPC_ARCH
/* Altivec/VSX complex permute operation */
static inline vector signed int ppc_complex_operation(vector signed int a,
                                                      vector signed int b,
                                                      vector signed int c,
                                                      vector signed int d,
                                                      vector signed int e,
                                                      vector signed int f,
                                                      vector signed int g,
                                                      vector signed int h,
                                                      vector signed int i,
                                                      vector signed int j) {
    /* Complex permute and compute */
    vector signed int t1 = vec_add(a, b);
    vector signed int t2 = vec_add(c, d);
    vector signed int t3 = vec_add(e, f);
    vector signed int t4 = vec_add(g, h);
    vector signed int t5 = vec_add(i, j);
    
    vector signed int sum1 = vec_add(t1, t2);
    vector signed int sum2 = vec_add(t3, t4);
    vector signed int sum3 = vec_add(sum1, sum2);
    
    return vec_add(sum3, t5);
}
#endif

/* Atomic operation with many parameters */
static inline int atomic_multi_operand(int *ptr, int *expected, int desired) {
    int result;
    /* __atomic_compare_exchange has 6 parameters which might expand further */
    __atomic_compare_exchange(ptr, expected, &desired, 0,
                              __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Additional atomic operations */
    __atomic_fetch_add(ptr, desired, __ATOMIC_SEQ_CST);
    __atomic_fetch_sub(ptr, 1, __ATOMIC_SEQ_CST);
    
    result = __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
    return result;
}

/* Bit-field operations across multiple words */
static inline uint64_t bitfield_multi_operand(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h, uint64_t i,
                                              uint64_t j) {
    /* Complex bit-field manipulation */
    uint64_t t1 = (a & 0xFF) | ((b & 0xFF) << 8);
    uint64_t t2 = (c & 0xFF) | ((d & 0xFF) << 8);
    uint64_t t3 = (e & 0xFF) | ((f & 0xFF) << 8);
    uint64_t t4 = (g & 0xFF) | ((h & 0xFF) << 8);
    uint64_t t5 = (i & 0xFF) | ((j & 0xFF) << 8);
    
    return t1 + t2 + t3 + t4 + t5;
}

int main() {
    int result = 0;
    
    /* Test complex scalar expression */
    int scalar_result = complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += scalar_result;
    
    /* Test inline assembly with many operands */
    uint64_t asm_result1 = inline_asm_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    uint64_t asm_result2 = inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)(asm_result1 + asm_result2);
    
    /* Test bit-field operations */
    uint64_t bitfield_result = bitfield_multi_operand(0xAA, 0xBB, 0xCC, 0xDD,
                                                      0xEE, 0xFF, 0x11, 0x22,
                                                      0x33, 0x44);
    result += (int)bitfield_result;
    
    /* Test atomic operations */
    int atomic_var = 42;
    int expected = 42;
    int atomic_result = atomic_multi_operand(&atomic_var, &expected, 100);
    result += atomic_result;
    
#ifdef X86_ARCH
    /* Test AVX-512 operations if available */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_vec1 = _mm512_set1_pd(1.0);
        __m512d avx_vec2 = _mm512_set1_pd(2.0);
        __m512d avx_vec3 = _mm512_set1_pd(3.0);
        __m512d avx_vec4 = _mm512_set1_pd(4.0);
        __m512d avx_vec5 = _mm512_set1_pd(5.0);
        
        __m512d avx_result = avx512_multi_operand(avx_vec1, avx_vec2, avx_vec3,
                                                  avx_vec4, avx_vec5, 0xFF, 0);
        
        /* Extract a scalar from vector for result */
        double avx_scalar = _mm512_cvtsd_f64(_mm512_castpd512_pd128(avx_result));
        result += (int)avx_scalar;
    }
#endif
    
#ifdef ARM_ARCH
    /* Test NEON operations */
    int32x4_t neon_vec1 = vdupq_n_s32(1);
    int32x4_t neon_vec2 = vdupq_n_s32(2);
    int32x4_t neon_vec3 = vdupq_n_s32(3);
    int32x4_t neon_vec4 = vdupq_n_s32(4);
    int32x4_t neon_vec5 = vdupq_n_s32(5);
    int32x4_t neon_vec6 = vdupq_n_s32(6);
    int32x4_t neon_vec7 = vdupq_n_s32(7);
    int32x4_t neon_vec8 = vdupq_n_s32(8);
    int32x4_t neon_vec9 = vdupq_n_s32(9);
    int32x4_t neon_vec10 = vdupq_n_s32(10);
    
    int32x4_t neon_result = neon_complex_expression(neon_vec1, neon_vec2, neon_vec3,
                                                    neon_vec4, neon_vec5, neon_vec6,
                                                    neon_vec7, neon_vec8, neon_vec9,
                                                    neon_vec10);
    
    /* Extract scalar from vector */
    int32_t neon_scalar = vgetq_lane_s32(neon_result, 0);
    result += neon_scalar;
#endif
    
    printf("Final result: %d\n", result);
    return result == 0 ? 0 : 1;
}
