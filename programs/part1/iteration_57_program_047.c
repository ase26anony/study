/* test_multi_operand_expansion.c
 * Designed to trigger GCC's 10/11-operand instruction expansion
 * Compile with: gcc -O3 -march=native -ftree-vectorize -funsafe-math-optimizations -fdump-rtl-expand -fdump-rtl-combine -o test test_multi_operand_expansion.c
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* For atomic operations */
#include <stdatomic.h>

/* For vector intrinsics */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__ARM_ARCH)
#include <arm_neon.h>
#include <arm_acle.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression_10_operands(int a, int b, int c, int d, 
                                                 int e, int f, int g, int h,
                                                 int i, int j) {
    /* This expression might be combined into a single instruction
     * with many operands during optimization */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Another complex expression with 11 operands */
static inline float complex_float_expression_11(float a, float b, float c, float d,
                                                float e, float f, float g, float h,
                                                float i, float j, float k) {
    /* FMA chain that might be optimized into multi-operand instruction */
    return a * b + c * d + e * f + g * h + i * j + k;
}

/* Function using inline assembly with exactly 11 operands */
static inline uint64_t inline_asm_11_operands(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h, uint64_t i,
                                              uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with 11 operands to force RTL expansion */
    __asm__ volatile (
        /* Simple operation that uses all operands */
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

#ifdef __x86_64__
/* x86-64 specific: AVX-512 operations with many operands */
static __m512i avx512_multi_operand_test(__m512i a, __m512i b, __m512i c,
                                         __m512i d, __m512i e, __m512i f,
                                         __mmask16 mask) {
    /* AVX-512 masked operations can have many operands */
    __m512i temp1 = _mm512_maskz_mullo_epi32(mask, a, b);
    __m512i temp2 = _mm512_mask_mullo_epi32(c, mask, d, e);
    
    /* Complex expression that might use many operands */
    return _mm512_add_epi32(temp1, temp2);
}

/* FMA with rounding control - potentially many operands */
static __m512d avx512_fma_multi_operand(__m512d a, __m512d b, __m512d c,
                                        __m512d d, __m512d e, __mmask8 mask) {
    /* Chain of FMA operations */
    __m512d result = _mm512_fmadd_pd(a, b, c);
    result = _mm512_fmadd_pd(result, d, e);
    
    /* Masked operation adds more operands */
    return _mm512_mask_mov_pd(a, mask, result);
}
#endif

#ifdef __ARM_ARCH
/* ARM NEON/SVE specific multi-operand operations */
#ifdef __ARM_FEATURE_SVE
/* SVE2 has instructions with many operands like svmla_lane */
static svint32_t sve_multi_operand_test(svint32_t a, svint32_t b, svint32_t c,
                                        svint32_t d, svint32_t e, svint32_t f,
                                        svint32_t g, svint32_t h) {
    /* Complex SVE operation chain */
    svint32_t temp1 = svmla_s32_x(svptrue_b32(), a, b, c);
    svint32_t temp2 = svmla_s32_x(svptrue_b32(), d, e, f);
    return svadd_s32_x(svptrue_b32(), temp1, temp2);
}
#endif

/* ARM NEON multi-operand test */
static int32x4_t neon_multi_operand_test(int32x4_t a, int32x4_t b, int32x4_t c,
                                         int32x4_t d, int32x4_t e, int32x4_t f) {
    /* vmla_lane has multiple operands */
    int32x4_t result = vmlaq_lane_s32(a, b, vget_low_s32(c), 0);
    result = vmlaq_lane_s32(result, d, vget_low_s32(e), 1);
    return vaddq_s32(result, f);
}
#endif

/* Atomic operation with many parameters */
static int atomic_multi_operand_test(_Atomic int* ptr, int expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, which might expand
     * to a multi-operand instruction on some architectures */
    __atomic_compare_exchange(ptr, &expected, &desired, weak,
                              success_memorder, failure_memorder);
    
    return expected;
}

/* Bit-field operations that might combine */
static uint64_t bitfield_multi_operand(uint64_t a, uint64_t b, uint64_t c,
                                       uint64_t d, uint64_t e, uint64_t f,
                                       uint64_t g, uint64_t h, uint64_t i,
                                       uint64_t j) {
    /* Complex bitfield manipulation */
    uint64_t result = a;
    result = (result & b) | (c & ~d);
    result = (result << e) | (result >> f);
    result ^= g;
    result = (result + h) * i;
    result &= j;
    
    return result;
}

int main() {
    int result = 0;
    
    /* Test 1: Complex expression with 10 operands */
    int expr_result = complex_expression_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += expr_result;
    
    /* Test 2: Complex float expression with 11 operands */
    float float_result = complex_float_expression_11(1.0f, 2.0f, 3.0f, 4.0f,
                                                     5.0f, 6.0f, 7.0f, 8.0f,
                                                     9.0f, 10.0f, 11.0f);
    result += (int)float_result;
    
    /* Test 3: Inline assembly with 11 operands */
    uint64_t asm_result = inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    
    /* Test 4: Bit-field operations */
    uint64_t bitfield_result = bitfield_multi_operand(0x12345678, 0x87654321,
                                                      0xABCDEF01, 0xFEDCBA98,
                                                      2, 30, 0x55555555,
                                                      0xAAAAAAAA, 3, 0xFFFFFFFF);
    result += (int)bitfield_result;
    
#ifdef __x86_64__
    /* Test 5: AVX-512 operations */
    __m512i avx_a = _mm512_set1_epi32(1);
    __m512i avx_b = _mm512_set1_epi32(2);
    __m512i avx_c = _mm512_set1_epi32(3);
    __m512i avx_d = _mm512_set1_epi32(4);
    __m512i avx_e = _mm512_set1_epi32(5);
    __m512i avx_f = _mm512_set1_epi32(6);
    __mmask16 mask = 0xFFFF;
    
    __m512i avx_result = avx512_multi_operand_test(avx_a, avx_b, avx_c,
                                                   avx_d, avx_e, avx_f, mask);
    
    /* Extract one element to prevent optimization away */
    int avx_sum = _mm512_reduce_add_epi32(avx_result);
    result += avx_sum;
#endif
    
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
    /* Test 6: SVE operations (if available) */
    svint32_t sve_a = svdup_s32(1);
    svint32_t sve_b = svdup_s32(2);
    svint32_t sve_c = svdup_s32(3);
    svint32_t sve_d = svdup_s32(4);
    svint32_t sve_e = svdup_s32(5);
    svint32_t sve_f = svdup_s32(6);
    svint32_t sve_g = svdup_s32(7);
    svint32_t sve_h = svdup_s32(8);
    
    svint32_t sve_result = sve_multi_operand_test(sve_a, sve_b, sve_c, sve_d,
                                                  sve_e, sve_f, sve_g, sve_h);
    
    /* Extract result */
    int32_t sve_extracted[4];
    svst1_s32(svptrue_b32(), sve_extracted, sve_result);
    result += sve_extracted[0];
#endif
    
    /* Test 7: NEON operations */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    int32x4_t neon_f = vdupq_n_s32(6);
    
    int32x4_t neon_result = neon_multi_operand_test(neon_a, neon_b, neon_c,
                                                    neon_d, neon_e, neon_f);
    
    /* Extract result */
    result += vgetq_lane_s32(neon_result, 0);
#endif
    
    /* Test 8: Atomic operation with many parameters */
    _Atomic int atomic_var = 42;
    int atomic_result = atomic_multi_operand_test(&atomic_var, 42, 100);
    result += atomic_result;
    
    /* Test 9: Another complex expression that might trigger combining */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int combined = a * b + c * d + e * f + g * h + i * j + k;
    result += combined;
    
    /* Test 10: Memory operations that might combine */
    int arr[20];
    for (int idx = 0; idx < 20; idx++) {
        arr[idx] = idx;
    }
    
    /* Complex memory access pattern */
    int mem_result = arr[0] * arr[1] + arr[2] * arr[3] + arr[4] * arr[5] +
                     arr[6] * arr[7] + arr[8] * arr[9] + arr[10];
    result += mem_result;
    
    printf("Final result: %d\n", result);
    return result;
}
