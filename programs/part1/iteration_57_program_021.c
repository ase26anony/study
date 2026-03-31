/* Test program to trigger 10/11-operand instruction expansion in GCC's optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Enable architecture-specific intrinsics */
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__ARM_ARCH) || defined(__aarch64__)
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc__) || defined(__PPC__)
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* Fused multiply-add chain with 10 operands */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with exactly 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with 11 operands to force 11-operand expansion */
    __asm__ volatile (
        /* Simple operation - the template doesn't matter, operand count does */
        "add %[res], %[a], %[b] \n\t"
        "add %[res], %[res], %[c] \n\t"
        "add %[res], %[res], %[d] \n\t"
        "add %[res], %[res], %[e] \n\t"
        "add %[res], %[res], %[f] \n\t"
        "add %[res], %[res], %[g] \n\t"
        "add %[res], %[res], %[h] \n\t"
        "add %[res], %[res], %[i] \n\t"
        "add %[res], %[res], %[j] \n\t"
        "add %[res], %[res], %[k]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

#if defined(__x86_64__) || defined(__i386__)
/* x86 AVX-512 multi-operand intrinsic usage */
static __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                    __m512d d, __m512d e, __m512d f,
                                    __mmask8 m, int r) {
    /* AVX-512 masked fused multiply-add with rounding control */
    /* This can expand to instructions with many operands */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    
    /* Masked operation with multiple source registers */
    return _mm512_mask_add_pd(t1, m, t1, t2);
}

/* Another AVX-512 pattern with many operands */
static __m512i avx512_ternary_logic(__m512i a, __m512i b, __m512i c,
                                    __m512i d, __m512i e, __m512i f,
                                    __m512i g, __m512i h) {
    /* Chain of operations that might combine */
    __m512i t1 = _mm512_and_si512(a, b);
    __m512i t2 = _mm512_or_si512(c, d);
    __m512i t3 = _mm512_xor_si512(e, f);
    __m512i t4 = _mm512_add_epi64(g, h);
    
    /* Complex expression with many operands */
    return _mm512_add_epi64(_mm512_add_epi64(t1, t2),
                           _mm512_add_epi64(t3, t4));
}
#endif

#if defined(__ARM_ARCH) || defined(__aarch64__)
/* ARM NEON/SVE multi-operand patterns */
#ifdef __ARM_FEATURE_SVE
/* SVE2 multi-operand lane operations */
static svint32_t sve_multi_lane(svint32_t a, svint32_t b, svint32_t c,
                                svint32_t d, svint32_t e, svint32_t f,
                                svint32_t g, svint32_t h, svint32_t i,
                                svint32_t j) {
    /* SVE operations with lane selection can have many operands */
    svint32_t t1 = svmla_lane_s32(a, b, c, 0);
    svint32_t t2 = svmla_lane_s32(d, e, f, 1);
    svint32_t t3 = svmla_lane_s32(g, h, i, 2);
    
    return svadd_s32_z(svptrue_b32(), t1, svadd_s32_z(svptrue_b32(), t2, t3));
}
#endif

/* ARM NEON multi-register operations */
static int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                    int32x4_t d, int32x4_t e, int32x4_t f,
                                    int32x4_t g, int32x4_t h) {
    /* Chain of operations that might combine */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    int32x4_t t3 = vaddq_s32(g, h);
    
    /* Complex expression */
    return vaddq_s32(vaddq_s32(t1, t2), t3);
}
#endif

/* Atomic operation with many parameters */
static int atomic_multi_operand(int *ptr, int expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange with 6 parameters */
    __atomic_compare_exchange(ptr, &expected, &desired, weak,
                              success_memorder, failure_memorder);
    
    return expected;
}

/* Bit-field operations across multiple words */
static uint64_t bitfield_operations(uint64_t a, uint64_t b, uint64_t c,
                                    uint64_t d, uint64_t e, uint64_t f,
                                    uint64_t g, uint64_t h, uint64_t i,
                                    uint64_t j) {
    /* Complex bit-field manipulation that might combine */
    uint64_t result = 0;
    
    /* Extract and insert bits from 10 different sources */
    result |= (a & 0xFF) << 0;
    result |= (b & 0xFF) << 8;
    result |= (c & 0xFF) << 16;
    result |= (d & 0xFF) << 24;
    result |= (e & 0xFF) << 32;
    result |= (f & 0xFF) << 40;
    result |= (g & 0xFF) << 48;
    result |= (h & 0xFF) << 56;
    
    /* Additional operations */
    result ^= (i & 0xFFFF);
    result += (j & 0xFFFF);
    
    return result;
}

int main() {
    int result = 0;
    
    /* Test complex expression with 10 operands */
    result += complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test inline assembly with 11 operands */
    result += multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test atomic operation */
    int atomic_var = 42;
    result += atomic_multi_operand(&atomic_var, 42, 100);
    
    /* Test bitfield operations */
    result += bitfield_operations(0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
                                  0xFF, 0x11, 0x22, 0x33, 0x44);
    
#if defined(__x86_64__) || defined(__i386__)
    /* Test AVX-512 patterns if available */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_vec1 = _mm512_set1_pd(1.0);
        __m512d avx_vec2 = _mm512_set1_pd(2.0);
        __m512d avx_vec3 = _mm512_set1_pd(3.0);
        __m512d avx_vec4 = _mm512_set1_pd(4.0);
        __m512d avx_vec5 = _mm512_set1_pd(5.0);
        __m512d avx_vec6 = _mm512_set1_pd(6.0);
        
        __m512d avx_result = avx512_multi_operand(avx_vec1, avx_vec2, avx_vec3,
                                                  avx_vec4, avx_vec5, avx_vec6,
                                                  0xFF, _MM_FROUND_TO_NEAREST_INT);
        
        /* Extract a value to prevent optimization */
        double avx_val = _mm512_cvtsd_f64(_mm512_castpd512_pd128(avx_result));
        result += (int)avx_val;
    }
#endif
    
#if defined(__ARM_ARCH) || defined(__aarch64__)
    /* Test NEON patterns */
    int32x4_t neon_vec1 = vdupq_n_s32(1);
    int32x4_t neon_vec2 = vdupq_n_s32(2);
    int32x4_t neon_vec3 = vdupq_n_s32(3);
    int32x4_t neon_vec4 = vdupq_n_s32(4);
    int32x4_t neon_vec5 = vdupq_n_s32(5);
    int32x4_t neon_vec6 = vdupq_n_s32(6);
    int32x4_t neon_vec7 = vdupq_n_s32(7);
    int32x4_t neon_vec8 = vdupq_n_s32(8);
    
    int32x4_t neon_result = neon_multi_operand(neon_vec1, neon_vec2, neon_vec3,
                                               neon_vec4, neon_vec5, neon_vec6,
                                               neon_vec7, neon_vec8);
    
    /* Extract a value */
    result += vgetq_lane_s32(neon_result, 0);
#endif
    
    /* Additional complex expression with exactly 10 variables */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    
    /* Expression that might be combined into a multi-operand instruction */
    int complex_result = v1 * v2 + v3 * v4 + v5 * v6 + v7 * v8 + v9 * v10;
    result += complex_result;
    
    /* Another pattern: mixed operations */
    complex_result = (v1 & v2) | (v3 ^ v4) + (v5 << v6) - (v7 >> v8) * (v9 % v10);
    result += complex_result;
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
