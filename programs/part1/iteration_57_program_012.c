/* test_multi_operand.c - Test program for 10/11-operand instruction expansion */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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
#elif defined(__powerpc__) || defined(__PPC__)
#define TARGET_PPC 1
#include <altivec.h>
#endif

/* Generic vector types for fallback */
typedef int32_t v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression_10_operands(int a, int b, int c, int d, 
                                                 int e, int f, int g, int h,
                                                 int i, int j) {
    /* This expression might be combined into a single instruction
       with 10 operands during optimization */
    return ((a * b) + (c * d) + (e * f) + (g * h) + (i * j)) * 
           ((a + b) - (c + d) + (e + f) - (g + h) + (i + j));
}

/* Function using inline assembly with exactly 11 operands */
static int inline_asm_11_operands(int a, int b, int c, int d, int e,
                                  int f, int g, int h, int i, int j, int k) {
    int result;
    /* Inline asm with 11 operands to force 11-operand expansion */
    __asm__ volatile (
        /* Template doesn't matter much - we care about operand count */
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

#if TARGET_X86
/* AVX-512 intrinsic that uses many operands */
static __m512d avx512_multi_operand_test(__m512d a, __m512d b, __m512d c,
                                         __m512d d, __m512d e, __mmask8 k) {
    /* _mm512_mask3_fmadd_round_pd has 6 explicit operands + implicit rounding control
       The expansion might need 10+ operands when including all parameters */
    return _mm512_mask3_fmadd_round_pd(a, b, c, k, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}

/* FMA chain that might combine */
static __m512d fma_chain_10_operands(__m512d a, __m512d b, __m512d c,
                                     __m512d d, __m512d e, __m512d f,
                                     __m512d g, __m512d h, __m512d i,
                                     __m512d j) {
    /* Complex FMA expression that might be optimized into multi-operand form */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    __m512d t3 = _mm512_fmadd_pd(g, h, i);
    return _mm512_fmadd_pd(t1, t2, _mm512_fmadd_pd(t3, j, t1));
}
#endif

#if TARGET_ARM
#ifdef __ARM_FEATURE_SVE
/* SVE2 intrinsic with lane selection - potentially many operands */
static svint32_t sve_multi_lane_test(svint32_t a, svint32_t b, svint32_t c,
                                     svint32_t d, svint32_t e, uint64_t lane) {
    /* Complex SVE operation chain */
    svint32_t t1 = svmla_lane_s32(a, b, c, lane);
    svint32_t t2 = svmla_lane_s32(d, e, t1, lane);
    return svadd_s32_z(svptrue_b32(), t1, t2);
}
#endif

/* NEON intrinsic with multiple operands */
static int32x4_t neon_multi_operand_test(int32x4_t a, int32x4_t b, int32x4_t c,
                                         int32x4_t d, int32x4_t e, int32x4_t f,
                                         int32x4_t g, int32x4_t h) {
    /* vfmaq_lane_f32 has 4 operands, chain them */
    int32x4_t t1 = vmlaq_lane_s32(a, b, vget_low_s32(c), 0);
    int32x4_t t2 = vmlaq_lane_s32(d, e, vget_low_s32(f), 1);
    int32x4_t t3 = vmlaq_lane_s32(g, h, vget_high_s32(t1), 0);
    return vaddq_s32(t2, t3);
}
#endif

#if TARGET_PPC
/* PowerPC Altivec with complex permute */
static vector float ppc_multi_operand_test(vector float a, vector float b,
                                           vector float c, vector float d,
                                           vector float e, vector float f) {
    /* Complex Altivec operation with permute */
    vector float t1 = vec_madd(a, b, c);
    vector float t2 = vec_madd(d, e, f);
    vector unsigned char perm = {0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15};
    vector float t3 = vec_perm(t1, t2, perm);
    return vec_add(t1, vec_add(t2, t3));
}
#endif

/* Atomic built-in with many parameters */
static int atomic_multi_operand_test(int64_t *ptr, int64_t *expected, 
                                     int64_t desired) {
    int64_t old_expected = *expected;
    /* __atomic_compare_exchange has 6 parameters, expansion might need more */
    int success = __atomic_compare_exchange(ptr, expected, &desired,
                                            0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return success ? (int)desired : (int)old_expected;
}

/* Decimal float built-in (if available) */
#ifdef __DEC128_MAX__
static _Decimal128 decimal_multi_operand_test(_Decimal128 a, _Decimal128 b,
                                              _Decimal128 c, _Decimal128 d,
                                              _Decimal128 e, _Decimal128 f) {
    /* Chain of decimal operations */
    _Decimal128 t1 = a * b + c;
    _Decimal128 t2 = d * e + f;
    return t1 / t2 + a - b * c;
}
#endif

/* Main test driver */
int main() {
    int result = 0;
    
    /* Test 1: Complex expression with 10 variables */
    int vars[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result += complex_expression_10_operands(vars[0], vars[1], vars[2], vars[3],
                                             vars[4], vars[5], vars[6], vars[7],
                                             vars[8], vars[9]);
    
    /* Test 2: Inline assembly with 11 operands */
    result += inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test 3: Architecture-specific intrinsics */
#if TARGET_X86
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_vec1 = _mm512_set1_pd(1.0);
        __m512d avx_vec2 = _mm512_set1_pd(2.0);
        __m512d avx_vec3 = _mm512_set1_pd(3.0);
        __m512d avx_vec4 = _mm512_set1_pd(4.0);
        __m512d avx_vec5 = _mm512_set1_pd(5.0);
        
        __m512d avx_result = fma_chain_10_operands(avx_vec1, avx_vec2, avx_vec3,
                                                   avx_vec4, avx_vec5, avx_vec1,
                                                   avx_vec2, avx_vec3, avx_vec4,
                                                   avx_vec5);
        double avx_sum = _mm512_reduce_add_pd(avx_result);
        result += (int)avx_sum;
    }
#endif
    
#if TARGET_ARM
    int32x4_t neon1 = {1, 2, 3, 4};
    int32x4_t neon2 = {5, 6, 7, 8};
    int32x4_t neon3 = {9, 10, 11, 12};
    int32x4_t neon4 = {13, 14, 15, 16};
    int32x4_t neon5 = {17, 18, 19, 20};
    int32x4_t neon6 = {21, 22, 23, 24};
    int32x4_t neon7 = {25, 26, 27, 28};
    int32x4_t neon8 = {29, 30, 31, 32};
    
    int32x4_t neon_result = neon_multi_operand_test(neon1, neon2, neon3, neon4,
                                                    neon5, neon6, neon7, neon8);
    result += vgetq_lane_s32(neon_result, 0);
#endif
    
    /* Test 4: Atomic operation with many parameters */
    int64_t atomic_var = 100;
    int64_t expected = 100;
    int64_t desired = 200;
    result += atomic_multi_operand_test(&atomic_var, &expected, desired);
    
    /* Test 5: Vector reduction across multiple registers */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    v4si vec4 = {13, 14, 15, 16};
    v4si vec5 = {17, 18, 19, 20};
    
    /* Complex vector expression that might combine */
    v4si vec_result = (vec1 * vec2) + (vec3 * vec4) + vec5 +
                      (vec1 + vec2) * (vec3 + vec4) - vec5;
    result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* Test 6: Bit-field operations across multiple words */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
        unsigned int d : 16;
    } bitfields = {7, 31, 255, 65535};
    
    /* Complex bitfield expression */
    unsigned int bit_result = (bitfields.a << bitfields.b) |
                              (bitfields.c << 8) |
                              (bitfields.d << 16);
    result += bit_result;
    
    printf("Result: %d\n", result);
    return result != 0;
}
