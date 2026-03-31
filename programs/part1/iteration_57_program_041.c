/* Test program to trigger 10/11-operand instruction expansion in GCC optabs */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define HAS_X86 1
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#define HAS_ARM 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc64__) || defined(__PPC64__) || defined(__powerpc__)
#define HAS_PPC 1
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* Fused multiply-add chain that could be combined */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result1, result2, result3;
    
    /* Inline asm with 11 operands to force 11-operand expansion */
    asm volatile (
        /* Template doesn't matter much - we just need 11 operands */
        "mov %[r1], %[a]\n\t"
        "add %[r1], %[r1], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "mov %[r2], %[d]\n\t"
        "add %[r2], %[r2], %[e]\n\t"
        "add %[r2], %[r2], %[f]\n\t"
        "mov %[r3], %[g]\n\t"
        "add %[r3], %[r3], %[h]\n\t"
        "add %[r3], %[r3], %[i]\n\t"
        "add %[r1], %[r1], %[r2]\n\t"
        "add %[r1], %[r1], %[r3]\n\t"
        "add %[r1], %[r1], %[j]\n\t"
        "add %[r1], %[r1], %[k]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1;
}

#ifdef HAS_X86
/* AVX-512 operations with many operands */
static __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                    __m512d d, __m512d e, __mmask8 k) {
    /* FMA with mask and rounding control - up to 6 explicit operands */
    __m512d t1 = _mm512_mask3_fmadd_round_pd(a, b, c, k, _MM_FROUND_TO_NEAREST_INT);
    
    /* Another complex operation */
    __m512d t2 = _mm512_maskz_fmadd_round_pd(k, d, e, t1, _MM_FROUND_TO_ZERO);
    
    /* Permute with multiple sources */
    __m512d t3 = _mm512_permutex2var_pd(t1, _mm512_set1_epi64(0x01), t2);
    
    return _mm512_add_pd(t3, _mm512_set1_pd(1.0));
}

/* AVX-512 integer operation with many operands */
static __m512i avx512_multi_int(__m512i a, __m512i b, __m512i c,
                                __m512i d, __m512i e, __mmask16 k) {
    /* Blend with multiple sources and mask */
    __m512i t1 = _mm512_mask_blend_epi32(k, a, b);
    
    /* Multiply and add with saturation */
    __m512i t2 = _mm512_madd_epi16(t1, c);
    
    /* Alignr with immediate */
    __m512i t3 = _mm512_alignr_epi32(t2, d, 3);
    
    /* Ternary logic with 3 sources and immediate */
    return _mm512_ternarylogic_epi32(t3, e, _mm512_set1_epi32(0xFF), 0xE8);
}
#endif

#ifdef HAS_ARM
/* ARM SVE2 multi-operand intrinsics */
#ifdef __ARM_FEATURE_SVE
static svint32_t sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                   svint32_t d, svint32_t e, svbool_t pg) {
    /* SVE dot product with lane selection - can have many operands */
    svint32_t t1 = svdot_lane_s32(pg, a, b, c, 0);
    
    /* MLA with lane */
    svint32_t t2 = svmla_lane_s32(pg, t1, d, e, 1);
    
    /* Complex permute */
    svint32_t t3 = svtbl_s32(t2, svdup_u32(0x03020100));
    
    return svadd_s32_z(pg, t3, svdup_s32(1));
}
#endif

/* ARM Neon multi-operand operations */
static int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                    int32x4_t d, int32x4_t e) {
    /* MLA with lane - up to 4 explicit vector operands plus lane index */
    int32x4_t t1 = vmlaq_laneq_s32(a, b, c, 1);
    
    /* Dot product */
    int32x4_t t2 = vdotq_s32(t1, d, e);
    
    /* Table lookup */
    const uint8x16_t tbl = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    int32x4_t t3 = vqtbl1q_s8(vreinterpretq_s8_s32(t2), tbl);
    
    return vaddq_s32(t3, vdupq_n_s32(1));
}
#endif

#ifdef HAS_PPC
/* PowerPC Altivec multi-operand operations */
static vector signed int ppc_multi_operand(vector signed int a,
                                           vector signed int b,
                                           vector signed int c,
                                           vector signed int d,
                                           vector signed int e) {
    /* Multiply-add */
    vector signed int t1 = vec_madd(a, b, c);
    
    /* Permute with multiple sources */
    vector signed int t2 = vec_perm(t1, d, (vector unsigned char){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
    
    /* Select with condition */
    vector signed int t3 = vec_sel(t2, e, vec_cmpgt(t2, e));
    
    return vec_add(t3, vec_splats(1));
}
#endif

/* Atomic operation with many parameters */
static int atomic_multi_operand(int *ptr, int expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_ACQUIRE;
    
    /* __atomic_compare_exchange has 6 parameters */
    __atomic_compare_exchange(ptr, &expected, &desired, weak,
                              success_memorder, failure_memorder);
    
    return expected;
}

/* Decimal floating-point built-in (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static _Decimal128 decimal_multi_operand(_Decimal128 a, _Decimal128 b,
                                         _Decimal128 c, _Decimal128 d) {
    /* Complex decimal operation - may expand to multi-operand instruction */
    return __bid128_add(a, __bid128_mul(b, __bid128_add(c, d)));
}
#endif

int main() {
    int result = 0;
    
    /* Test complex expression with 10 operands */
    result += complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test inline assembly with 11 operands */
    result += multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test atomic operation */
    int atomic_var = 42;
    result += atomic_multi_operand(&atomic_var, 42, 100);
    
#ifdef HAS_X86
    /* Test AVX-512 operations if available */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_a = _mm512_set1_pd(1.0);
        __m512d avx_b = _mm512_set1_pd(2.0);
        __m512d avx_c = _mm512_set1_pd(3.0);
        __m512d avx_d = _mm512_set1_pd(4.0);
        __m512d avx_e = _mm512_set1_pd(5.0);
        __mmask8 k = 0xFF;
        
        __m512d avx_result = avx512_multi_operand(avx_a, avx_b, avx_c,
                                                  avx_d, avx_e, k);
        double avx_sum = _mm512_reduce_add_pd(avx_result);
        result += (int)avx_sum;
        
        /* Test integer version */
        __m512i avxi_a = _mm512_set1_epi32(1);
        __m512i avxi_b = _mm512_set1_epi32(2);
        __m512i avxi_c = _mm512_set1_epi32(3);
        __m512i avxi_d = _mm512_set1_epi32(4);
        __m512i avxi_e = _mm512_set1_epi32(5);
        __mmask16 ki = 0xFFFF;
        
        __m512i avxi_result = avx512_multi_int(avxi_a, avxi_b, avxi_c,
                                               avxi_d, avxi_e, ki);
        result += _mm512_reduce_add_epi32(avxi_result);
    }
#endif

#ifdef HAS_ARM
    /* Test ARM Neon operations */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    
    int32x4_t neon_result = neon_multi_operand(neon_a, neon_b, neon_c,
                                               neon_d, neon_e);
    result += vgetq_lane_s32(neon_result, 0);
#endif

#ifdef HAS_PPC
    /* Test PowerPC Altivec operations */
    vector signed int ppc_a = vec_splats(1);
    vector signed int ppc_b = vec_splats(2);
    vector signed int ppc_c = vec_splats(3);
    vector signed int ppc_d = vec_splats(4);
    vector signed int ppc_e = vec_splats(5);
    
    vector signed int ppc_result = ppc_multi_operand(ppc_a, ppc_b, ppc_c,
                                                     ppc_d, ppc_e);
    result += vec_extract(ppc_result, 0);
#endif

    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
