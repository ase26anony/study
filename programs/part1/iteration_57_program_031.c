/* test_multi_operand_expansion.c
 * Compile with: gcc -O3 -march=native -ftree-vectorize -funsafe-math-optimizations -fdump-rtl-expand -fdump-rtl-combine test_multi_operand_expansion.c -o test
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Generic vector types for fallback */
typedef int32_t v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex expression that might combine into multi-operand instruction */
static inline v4sf complex_fma_chain(v4sf a, v4sf b, v4sf c, v4sf d, 
                                     v4sf e, v4sf f, v4sf g, v4sf h,
                                     v4sf i, v4sf j) {
    /* FMA chain with 10 operands */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Inline assembly with exactly 11 operands */
static inline uint64_t asm_11_operands(uint64_t a, uint64_t b, uint64_t c,
                                       uint64_t d, uint64_t e, uint64_t f,
                                       uint64_t g, uint64_t h, uint64_t i,
                                       uint64_t j, uint64_t k) {
    uint64_t result;
    /* 11 operands: 1 output + 10 inputs = 11 total */
    __asm__ volatile (
        "mov %[out], %[in1] \n\t"
        "add %[out], %[in2] \n\t"
        "add %[out], %[in3] \n\t"
        "add %[out], %[in4] \n\t"
        "add %[out], %[in5] \n\t"
        "add %[out], %[in6] \n\t"
        "add %[out], %[in7] \n\t"
        "add %[out], %[in8] \n\t"
        "add %[out], %[in9] \n\t"
        "add %[out], %[in10]"
        : [out] "=r" (result)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
          [in4] "r" (d), [in5] "r" (e), [in6] "r" (f),
          [in7] "r" (g), [in8] "r" (h), [in9] "r" (i),
          [in10] "r" (j)
        : "cc"
    );
    return result + k; /* k is 11th operand used outside asm */
}

/* Atomic built-in with many parameters */
static int atomic_compare_exchange_many_args(int *ptr, int *expected, 
                                             int desired, int weak,
                                             int success_memorder, 
                                             int failure_memorder) {
    return __atomic_compare_exchange(ptr, expected, &desired, weak,
                                     success_memorder, failure_memorder);
}

#ifdef __x86_64__
#include <immintrin.h>

/* AVX-512 masked operation with many operands */
static __m512d avx512_masked_fma(__m512d a, __m512d b, __m512d c,
                                 __m512d d, __m512d e, __mmask8 k) {
    /* Chain multiple masked operations */
    __m512d t1 = _mm512_mask_fmadd_pd(a, k, b, c);
    __m512d t2 = _mm512_mask_fmadd_pd(d, k, e, t1);
    return _mm512_mask_fmadd_pd(t2, k, a, b);
}

/* AVX-512 with rounding control (potentially 11 operands) */
static __m512 avx512_rounding_fma(__m512 a, __m512 b, __m512 c,
                                  __m512 d, __m512 e, __m512 f,
                                  __m512 g, __m512 h, __mmask16 k) {
    /* Complex expression that might combine */
    __m512 t1 = _mm512_fmadd_round_ps(a, b, c, _MM_FROUND_TO_NEAREST_INT);
    __m512 t2 = _mm512_fmadd_round_ps(d, e, f, _MM_FROUND_TO_NEAREST_INT);
    __m512 t3 = _mm512_fmadd_round_ps(g, h, t1, _MM_FROUND_TO_NEAREST_INT);
    return _mm512_mask_add_ps(t2, k, t3, t2);
}
#endif

#ifdef __ARM_ARCH
#include <arm_neon.h>
#include <arm_acle.h>

/* ARM SVE2-style pattern (simulated with NEON) */
static int32x4_t neon_multi_lane_ops(int32x4_t a, int32x4_t b, int32x4_t c,
                                     int32x4_t d, int32x4_t e, int32x4_t f,
                                     int32x4_t g, int32x4_t h) {
    /* Complex sequence that might expand to multi-operand pattern */
    int32x4_t t1 = vmlaq_laneq_s32(a, b, c, 0);
    int32x4_t t2 = vmlaq_laneq_s32(d, e, f, 1);
    int32x4_t t3 = vmlaq_laneq_s32(g, h, t1, 2);
    return vaddq_s32(t2, t3);
}

/* ARM bitfield operations with many operands */
static uint64_t arm_bfi_chain(uint64_t a, uint64_t b, uint64_t c,
                              uint64_t d, uint64_t e) {
    /* Chain of bitfield insert/extract */
    uint64_t t1 = __builtin_arm_bfi(a, b, 8, 16);
    uint64_t t2 = __builtin_arm_bfi(t1, c, 24, 32);
    uint64_t t3 = __builtin_arm_bfi(t2, d, 40, 48);
    return __builtin_arm_bfi(t3, e, 56, 64);
}
#endif

#ifdef __PPC64__
#include <altivec.h>

/* PowerPC VSX complex permute */
static vector double vsx_permute_fma(vector double a, vector double b,
                                     vector double c, vector double d,
                                     vector double e, vector double f) {
    /* Multiple permute and FMA operations */
    vector double t1 = vec_madd(a, b, c);
    vector double t2 = vec_perm(t1, d, (vector unsigned char){0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23});
    vector double t3 = vec_madd(e, f, t2);
    vector double t4 = vec_perm(t3, a, (vector unsigned char){16,17,18,19,20,21,22,23,0,1,2,3,4,5,6,7});
    return vec_madd(t4, b, c);
}
#endif

/* Bit-field extraction across multiple words */
static uint64_t bitfield_extraction_chain(uint64_t a, uint64_t b, uint64_t c,
                                          uint64_t d, uint64_t e, uint64_t f,
                                          uint64_t g, uint64_t h, uint64_t i,
                                          uint64_t j) {
    /* Complex bitfield expression with 10 operands */
    return ((a >> 0) & 0xFF) |
           ((b >> 8) & 0xFF00) |
           ((c >> 16) & 0xFF0000) |
           ((d >> 24) & 0xFF000000) |
           ((e >> 32) & 0xFF00000000) |
           ((f >> 40) & 0xFF0000000000) |
           ((g >> 48) & 0xFF000000000000) |
           ((h >> 56) & 0xFF00000000000000) |
           ((i << 8) & 0xFF00) |
           ((j << 16) & 0xFF0000);
}

int main() {
    uint64_t result = 0;
    
    /* Test 1: Complex FMA chain with 10 operands */
    {
        v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
        v4sf c = {9.0f, 10.0f, 11.0f, 12.0f};
        v4sf d = {13.0f, 14.0f, 15.0f, 16.0f};
        v4sf e = {17.0f, 18.0f, 19.0f, 20.0f};
        v4sf f = {21.0f, 22.0f, 23.0f, 24.0f};
        v4sf g = {25.0f, 26.0f, 27.0f, 28.0f};
        v4sf h = {29.0f, 30.0f, 31.0f, 32.0f};
        v4sf i = {33.0f, 34.0f, 35.0f, 36.0f};
        v4sf j = {37.0f, 38.0f, 39.0f, 40.0f};
        
        v4sf res = complex_fma_chain(a, b, c, d, e, f, g, h, i, j);
        result += (uint64_t)res[0];
    }
    
    /* Test 2: Inline assembly with 11 operands */
    {
        uint64_t asm_res = asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        result += asm_res;
    }
    
    /* Test 3: Atomic built-in with 6 parameters */
    {
        int atomic_var = 42;
        int expected = 42;
        int desired = 100;
        int atomic_res = atomic_compare_exchange_many_args(&atomic_var, &expected, 
                                                          desired, 0, 5, 5);
        result += atomic_res;
    }
    
    /* Test 4: Bitfield extraction chain with 10 operands */
    {
        uint64_t bf_res = bitfield_extraction_chain(0xAA, 0xBB, 0xCC, 0xDD,
                                                   0xEE, 0xFF, 0x11, 0x22,
                                                   0x33, 0x44);
        result += bf_res;
    }
    
#ifdef __x86_64__
    /* Test 5: AVX-512 masked operations */
    {
        __m512d avx_a = _mm512_set1_pd(1.0);
        __m512d avx_b = _mm512_set1_pd(2.0);
        __m512d avx_c = _mm512_set1_pd(3.0);
        __m512d avx_d = _mm512_set1_pd(4.0);
        __m512d avx_e = _mm512_set1_pd(5.0);
        __mmask8 mask = 0xFF;
        
        __m512d avx_res = avx512_masked_fma(avx_a, avx_b, avx_c, avx_d, avx_e, mask);
        double avx_sum = _mm512_reduce_add_pd(avx_res);
        result += (uint64_t)avx_sum;
    }
#endif
    
#ifdef __ARM_ARCH
    /* Test 6: ARM NEON multi-lane operations */
    {
        int32x4_t neon_a = {1, 2, 3, 4};
        int32x4_t neon_b = {5, 6, 7, 8};
        int32x4_t neon_c = {9, 10, 11, 12};
        int32x4_t neon_d = {13, 14, 15, 16};
        int32x4_t neon_e = {17, 18, 19, 20};
        int32x4_t neon_f = {21, 22, 23, 24};
        int32x4_t neon_g = {25, 26, 27, 28};
        int32x4_t neon_h = {29, 30, 31, 32};
        
        int32x4_t neon_res = neon_multi_lane_ops(neon_a, neon_b, neon_c,
                                                neon_d, neon_e, neon_f,
                                                neon_g, neon_h);
        result += vgetq_lane_s32(neon_res, 0);
    }
#endif
    
    printf("Result: %lu\n", result);
    return (int)(result % 256);
}
