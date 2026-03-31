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
#elif defined(__powerpc__) || defined(__PPC__)
#define TARGET_PPC 1
#include <altivec.h>
#endif

/* Atomic operations header */
#include <stdatomic.h>

/* Complex expression to force operand combining */
static inline int complex_expression_10_operands(int a, int b, int c, int d, 
                                                  int e, int f, int g, int h,
                                                  int i, int j) {
    /* This expression might be combined into a multi-operand instruction */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with exactly 11 operands */
static int inline_asm_11_operands(int a, int b, int c, int d, int e,
                                  int f, int g, int h, int i, int j, int k) {
    int result;
    
    /* Inline assembly with 11 operands to force 11-operand expansion */
    __asm__ volatile (
        /* Simple template - the goal is operand count, not the instruction */
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

/* Function using atomic built-in with many parameters */
static int atomic_multi_operand(void) {
    _Atomic int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, which might expand further */
    int result = __atomic_compare_exchange_n(&atomic_var, &expected, desired,
                                             weak, success_memorder, 
                                             failure_memorder);
    return result ? atomic_var : expected;
}

#if TARGET_X86
/* AVX-512 intrinsics that can use many operands */
static __m512i avx512_multi_operand_intrinsic(__m512i a, __m512i b, __m512i c,
                                              __m512i d, __m512i e, __m512i f,
                                              __mmask16 mask) {
    /* Chain multiple operations that might combine */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(e, f);
    
    /* Masked operation with multiple operands */
    __m512i result = _mm512_mask_add_epi32(t1, mask, t2, t3);
    
    /* FMA with rounding control - up to 5 operands */
    #ifdef __AVX512F__
    __m512d da = _mm512_set1_pd(1.0);
    __m512d db = _mm512_set1_pd(2.0);
    __m512d dc = _mm512_set1_pd(3.0);
    __m512d fma_result = _mm512_fmadd_round_pd(da, db, dc, _MM_FROUND_TO_NEAREST_INT);
    result = _mm512_castpd_si512(fma_result);
    #endif
    
    return result;
}
#endif

#if TARGET_ARM
/* ARM SVE/NEON intrinsics with lane operations */
static int32x4_t neon_multi_lane_ops(int32x4_t a, int32x4_t b, int32x4_t c,
                                     int32x4_t d, int32x4_t e) {
    /* Multi-lane operations */
    int32x4_t t1 = vmlaq_laneq_s32(a, b, c, 0);
    int32x4_t t2 = vmlaq_laneq_s32(t1, d, e, 1);
    
    /* Additional operations to increase operand count */
    int32x4_t t3 = vqdmulhq_s32(t2, a);
    int32x4_t result = vrhaddq_s32(t3, b);
    
    return result;
}

#ifdef __ARM_FEATURE_SVE
/* SVE2 intrinsics with many operands */
static svint32_t sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                   svint32_t d, svint32_t e, svint32_t f,
                                   svbool_t pg) {
    /* SVE dot product with lane - potentially many operands */
    svint32_t t1 = svdot_lane_s32_z(pg, a, b, c, 0);
    svint32_t t2 = svmla_lane_s32_z(pg, t1, d, e, 1);
    svint32_t result = svmad_s32_z(pg, t2, f, a);
    
    return result;
}
#endif
#endif

#if TARGET_PPC
/* PowerPC Altivec/VSX operations */
static vector int ppc_multi_vec_ops(vector int a, vector int b, vector int c,
                                    vector int d, vector int e, vector int f) {
    /* Complex permute and compute */
    vector int t1 = vec_add(a, b);
    vector int t2 = vec_add(c, d);
    vector int t3 = vec_add(e, f);
    
    /* Multiply-add with permute */
    vector int t4 = vec_madd(a, b, t1);
    vector int result = vec_add(t4, vec_perm(t2, t3, vec_gbb));
    
    return result;
}
#endif

/* Main driver function */
int main(void) {
    int result = 0;
    
    /* 1. Complex expression with 10+ operands */
    int expr_result = complex_expression_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += expr_result;
    
    /* 2. Inline assembly with 11 operands */
    int asm_result = inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += asm_result;
    
    /* 3. Atomic operation with multiple parameters */
    int atomic_result = atomic_multi_operand();
    result += atomic_result;
    
    /* 4. Architecture-specific vector intrinsics */
    #if TARGET_X86
    __m512i vec_a = _mm512_set1_epi32(1);
    __m512i vec_b = _mm512_set1_epi32(2);
    __m512i vec_c = _mm512_set1_epi32(3);
    __m512i vec_d = _mm512_set1_epi32(4);
    __m512i vec_e = _mm512_set1_epi32(5);
    __m512i vec_f = _mm512_set1_epi32(6);
    __mmask16 mask = 0xFFFF;
    
    __m512i avx_result = avx512_multi_operand_intrinsic(vec_a, vec_b, vec_c,
                                                        vec_d, vec_e, vec_f,
                                                        mask);
    int avx_sum = _mm512_reduce_add_epi32(avx_result);
    result += avx_sum;
    #endif
    
    #if TARGET_ARM
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    
    int32x4_t neon_result = neon_multi_lane_ops(neon_a, neon_b, neon_c,
                                                neon_d, neon_e);
    int neon_sum = vaddvq_s32(neon_result);
    result += neon_sum;
    #endif
    
    #if TARGET_PPC
    vector int ppc_a = {1, 2, 3, 4};
    vector int ppc_b = {5, 6, 7, 8};
    vector int ppc_c = {9, 10, 11, 12};
    vector int ppc_d = {13, 14, 15, 16};
    vector int ppc_e = {17, 18, 19, 20};
    vector int ppc_f = {21, 22, 23, 24};
    
    vector int ppc_result = ppc_multi_vec_ops(ppc_a, ppc_b, ppc_c,
                                              ppc_d, ppc_e, ppc_f);
    int ppc_sum = ppc_result[0] + ppc_result[1] + ppc_result[2] + ppc_result[3];
    result += ppc_sum;
    #endif
    
    /* 5. Additional complex expression that might chain FMA operations */
    double fma_chain = 0.0;
    double v1 = 1.1, v2 = 2.2, v3 = 3.3, v4 = 4.4, v5 = 5.5;
    double v6 = 6.6, v7 = 7.7, v8 = 8.8, v9 = 9.9, v10 = 10.10;
    
    /* This expression might be optimized into multi-operand FMA instructions */
    fma_chain = v1 * v2 + v3 * v4 + v5 * v6 + v7 * v8 + v9 * v10;
    result += (int)fma_chain;
    
    /* 6. Bit-field operations across multiple words */
    unsigned int bf1 = 0x12345678;
    unsigned int bf2 = 0x9ABCDEF0;
    unsigned int bf3 = 0xFEDCBA98;
    unsigned int bf4 = 0x76543210;
    
    /* Complex bit-field extraction/insertion */
    unsigned int bf_result = ((bf1 & 0xFF) << 24) |
                             ((bf2 & 0xFF00) << 8) |
                             ((bf3 & 0xFF0000) >> 8) |
                             ((bf4 & 0xFF000000) >> 24);
    result += bf_result;
    
    printf("Final result: %d\n", result);
    
    /* Return non-zero to indicate execution */
    return result != 0 ? 0 : 1;
}
