/* test_multi_operand.c - Test program for GCC optabs 10/11-operand expansion */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Generic fallback for architectures without specific intrinsics */
#ifndef __x86_64__
#ifndef __ARM_ARCH
#ifndef __powerpc64__
#define GENERIC_FALLBACK 1
#endif
#endif
#endif

/* x86 AVX-512 intrinsics */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* ARM SVE intrinsics */
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#include <arm_neon.h>
#endif

/* PowerPC VSX intrinsics */
#ifdef __powerpc64__
#include <altivec.h>
#endif

/* Atomic operations for all architectures */
#include <stdatomic.h>

/* Complex expression that might combine into multi-operand instruction */
static inline int64_t complex_expression(int64_t a, int64_t b, int64_t c,
                                         int64_t d, int64_t e, int64_t f,
                                         int64_t g, int64_t h, int64_t i,
                                         int64_t j, int64_t k) {
    /* Fused multiply-add chain with 11 operands */
    return a * b + c * d + e * f + g * h + i * j + k;
}

/* Function using inline assembly with exactly 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with 11 operands - forces RTL expansion */
    __asm__ volatile (
        "/* Multi-operand test with 11 operands */\n\t"
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
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

#ifdef __x86_64__
/* AVX-512 masked operation with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __mmask8 m) {
    /* This could expand to a multi-operand instruction with:
       - 3 source registers (a, b, c)
       - 1 destination register
       - 1 mask register
       - 1 rounding control
       - 1 sae control
       Total: 7+ operands, compiler might combine further */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, t1);
    return _mm512_mask_mov_pd(t1, m, t2);
}

/* AVX-512 ternary operation with explicit rounding */
static inline __m512d avx512_ternary_round(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __m512d f,
                                           __mmask8 m1, __mmask8 m2) {
    /* Chain operations that might combine */
    __m512d t1 = _mm512_fmadd_round_pd(a, b, c, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    __m512d t2 = _mm512_fmadd_round_pd(d, e, f, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    return _mm512_mask_add_pd(t1, m1, t1, t2);
}
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
/* SVE2 multi-lane operation - can have many operands */
static inline svint32_t sve_multi_lane(svint32_t a, svint32_t b, svint32_t c,
                                       svint32_t d, svint32_t e, svint32_t f,
                                       uint64_t lane1, uint64_t lane2) {
    /* Simulate complex SVE pattern with multiple lane selections */
    svint32_t t1 = svmla_lane_s32(a, b, c, lane1);
    svint32_t t2 = svmla_lane_s32(d, e, f, lane2);
    return svadd_s32(t1, t2);
}
#endif

/* NEON multi-register operation */
static inline int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                           int32x4_t d, int32x4_t e, int32x4_t f,
                                           int32x4_t g, int32x4_t h) {
    /* Chain of operations that might combine */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    int32x4_t t3 = vaddq_s32(t1, t2);
    return vmlaq_s32(t3, g, h);
}
#endif

#ifdef __powerpc64__
/* VSX complex permute and compute */
static inline vector double vsx_multi_operand(vector double a, vector double b,
                                              vector double c, vector double d,
                                              vector double e, vector double f) {
    /* Multiple operations that PowerPC might combine */
    vector double t1 = vec_madd(a, b, c);
    vector double t2 = vec_madd(d, e, f);
    return vec_add(t1, t2);
}
#endif

/* Atomic operation with many parameters */
static inline int atomic_multi_operand(_Atomic int* ptr, int expected, int desired) {
    int result = 0;
    /* __atomic_compare_exchange has 6 parameters, which might expand further */
    __atomic_compare_exchange(ptr, &expected, &desired, 0,
                              __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}

/* Bit-field manipulation across multiple words */
static inline uint64_t bitfield_ops(uint64_t a, uint64_t b, uint64_t c,
                                    uint64_t d, uint64_t e, uint64_t f,
                                    uint64_t g, uint64_t h, uint64_t i,
                                    uint64_t j) {
    /* Complex bitfield expression with 10 operands */
    return ((a & 0xFF) << 56) |
           ((b & 0xFF) << 48) |
           ((c & 0xFF) << 40) |
           ((d & 0xFF) << 32) |
           ((e & 0xFF) << 24) |
           ((f & 0xFF) << 16) |
           ((g & 0xFF) << 8) |
           ((h & 0xFF)) |
           ((i & 0xF) << 60) |
           ((j & 0xF) << 4);
}

int main() {
    uint64_t result = 0;
    
    /* Test 1: Complex arithmetic expression with 11 operands */
    result += complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test 2: Inline assembly with exactly 11 operands */
    result += multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test 3: Bit-field operations with 10 operands */
    result += bitfield_ops(0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
                           0x11, 0x22, 0x33, 0x44);
    
#ifdef __x86_64__
    /* Test 4: AVX-512 operations */
    __m512d avx_a = _mm512_set1_pd(1.0);
    __m512d avx_b = _mm512_set1_pd(2.0);
    __m512d avx_c = _mm512_set1_pd(3.0);
    __m512d avx_d = _mm512_set1_pd(4.0);
    __m512d avx_e = _mm512_set1_pd(5.0);
    __m512d avx_f = _mm512_set1_pd(6.0);
    
    __m512d avx_res = avx512_multi_operand(avx_a, avx_b, avx_c, avx_d, avx_e, 0xFF);
    __m512d avx_res2 = avx512_ternary_round(avx_a, avx_b, avx_c, avx_d, avx_e, avx_f, 0xFF, 0xFF);
    
    /* Extract some result to prevent optimization */
    double avx_sum = _mm512_reduce_add_pd(avx_res) + _mm512_reduce_add_pd(avx_res2);
    result += (uint64_t)avx_sum;
#endif

#ifdef __ARM_ARCH
    /* Test 5: NEON operations */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    int32x4_t neon_f = vdupq_n_s32(6);
    int32x4_t neon_g = vdupq_n_s32(7);
    int32x4_t neon_h = vdupq_n_s32(8);
    
    int32x4_t neon_res = neon_multi_operand(neon_a, neon_b, neon_c, neon_d,
                                            neon_e, neon_f, neon_g, neon_h);
    
    /* Extract result */
    int32_t neon_sum = vgetq_lane_s32(neon_res, 0) +
                       vgetq_lane_s32(neon_res, 1) +
                       vgetq_lane_s32(neon_res, 2) +
                       vgetq_lane_s32(neon_res, 3);
    result += neon_sum;
#endif

#ifdef __powerpc64__
    /* Test 6: VSX operations */
    vector double vsx_a = (vector double){1.0, 2.0};
    vector double vsx_b = (vector double){3.0, 4.0};
    vector double vsx_c = (vector double){5.0, 6.0};
    vector double vsx_d = (vector double){7.0, 8.0};
    vector double vsx_e = (vector double){9.0, 10.0};
    vector double vsx_f = (vector double){11.0, 12.0};
    
    vector double vsx_res = vsx_multi_operand(vsx_a, vsx_b, vsx_c, vsx_d, vsx_e, vsx_f);
    
    /* Extract result */
    double vsx_sum = ((double*)&vsx_res)[0] + ((double*)&vsx_res)[1];
    result += (uint64_t)vsx_sum;
#endif

    /* Test 7: Atomic operation with multiple parameters */
    _Atomic int atomic_var = 42;
    result += atomic_multi_operand(&atomic_var, 42, 100);
    
    /* Additional test: Mixed operations that might combine */
    {
        uint64_t a = 1, b = 2, c = 3, d = 4, e = 5;
        uint64_t f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
        
        /* Expression with 11 variables - might trigger combine pass */
        uint64_t mixed = (a * b) + (c * d) + (e * f) + (g * h) + (i * j) + k;
        result += mixed;
        
        /* Another expression with different operations */
        mixed = ((a & b) | (c & d)) ^ ((e & f) | (g & h)) + (i ^ j) * k;
        result += mixed;
    }
    
    printf("Result: %lu\n", result);
    return (int)(result % 256);
}
