/* Test program to trigger 10/11-operand expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(_M_X64)
#define TARGET_X86 1
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#define TARGET_ARM 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc64__) || defined(__PPC64__)
#define TARGET_PPC 1
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* Fused multiply-add chain that could be combined */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with exactly 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result1, result2;
    
    /* Inline assembly with 11 operands - forces expansion */
    __asm__ volatile (
        "/* 11-operand asm block */\n\t"
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "add %[r1], %[r1], %[e]\n\t"
        "add %[r2], %[r2], %[f]\n\t"
        "add %[r1], %[r1], %[g]\n\t"
        "add %[r2], %[r2], %[h]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "add %[r2], %[r2], %[j]\n\t"
        "add %[r1], %[r1], %[k]\n\t"
        "add %[r1], %[r1], %[r2]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1;
}

/* Atomic operation with many parameters */
static int atomic_multi_operand(int *ptr, int expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, which when expanded
       with memory orders might need 10+ operands */
    __atomic_compare_exchange(ptr, &expected, &desired, weak,
                              success_memorder, failure_memorder);
    return expected;
}

#if TARGET_X86
/* AVX-512 intrinsics that use many operands */
static __m512i avx512_multi_operand(__m512i a, __m512i b, __m512i c,
                                    __m512i d, __m512i e, __m512i f,
                                    __mmask16 k1, __mmask16 k2) {
    /* Complex sequence that might combine */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(e, f);
    
    /* Masked operation with multiple operands */
    __m512i result = _mm512_mask_add_epi32(t1, k1, t2, t3);
    
    /* Another masked operation */
    result = _mm512_mask_mul_epi32(result, k2, result, t1);
    
    return result;
}

/* FMA with rounding control - up to 11 operands in expansion */
static __m512d avx512_fma_multi_op(__m512d a, __m512d b, __m512d c,
                                   __m512d d, __m512d e, __mmask8 k) {
    /* Chain of FMA operations */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, t1);
    
    /* Masked version with explicit rounding */
    __m512d result = _mm512_mask_fmadd_round_pd(t1, k, t2, a, 
                                                _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    return result;
}
#endif

#if TARGET_ARM
#ifdef __ARM_FEATURE_SVE
/* SVE2 intrinsics with lane selection - can use many operands */
static svint32_t sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                   svint32_t d, svint32_t e, svint32_t f,
                                   svint32_t g, svint32_t h) {
    /* Complex SVE operation chain */
    svint32_t t1 = svmla_s32(a, b, c);
    svint32_t t2 = svmla_s32(d, e, f);
    svint32_t t3 = svadd_s32(t1, t2);
    svint32_t result = svmla_lane_s32(t3, g, h, 3);
    return result;
}
#endif

/* ARM NEON with multiple vector operations */
static int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                    int32x4_t d, int32x4_t e, int32x4_t f) {
    /* Vector multiply-add chain */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    int32x4_t result = vaddq_s32(t1, t2);
    
    /* Lane extraction and insertion */
    int32_t lane0 = vgetq_lane_s32(result, 0);
    int32_t lane1 = vgetq_lane_s32(result, 1);
    int32_t lane2 = vgetq_lane_s32(result, 2);
    int32_t lane3 = vgetq_lane_s32(result, 3);
    
    result = vsetq_lane_s32(lane0 + lane1, result, 0);
    result = vsetq_lane_s32(lane2 + lane3, result, 1);
    
    return result;
}
#endif

#if TARGET_PPC
/* PowerPC Altivec with complex permute */
static vector signed int ppc_multi_operand(vector signed int a,
                                           vector signed int b,
                                           vector signed int c,
                                           vector signed int d) {
    /* Complex Altivec operation */
    vector signed int t1 = vec_add(a, b);
    vector signed int t2 = vec_add(c, d);
    vector signed int t3 = vec_madd(a, b, t1);
    vector signed int result = vec_madd(c, d, t2);
    result = vec_add(t3, result);
    
    /* Permute with many operands */
    result = vec_perm(result, t1, (vector unsigned char){0,1,2,3,4,5,6,7,
                                                         8,9,10,11,12,13,14,15});
    return result;
}
#endif

/* Main driver function */
int main() {
    int result = 0;
    
    /* Test complex expression with 10 variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    result += complex_expression(a, b, c, d, e, f, g, h, i, j);
    
    /* Test inline assembly with 11 operands */
    uint64_t asm_result = multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    
    /* Test atomic operation */
    int atomic_var = 42;
    result += atomic_multi_operand(&atomic_var, 42, 100);
    
#if TARGET_X86
    /* Test AVX-512 intrinsics if available */
    if (__builtin_cpu_supports("avx512f")) {
        __m512i vec1 = _mm512_set1_epi32(1);
        __m512i vec2 = _mm512_set1_epi32(2);
        __m512i vec3 = _mm512_set1_epi32(3);
        __m512i vec4 = _mm512_set1_epi32(4);
        __m512i vec5 = _mm512_set1_epi32(5);
        __m512i vec6 = _mm512_set1_epi32(6);
        
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = 0x5555;
        
        __m512i avx_result = avx512_multi_operand(vec1, vec2, vec3, vec4, vec5, vec6, mask1, mask2);
        int sum_arr[16];
        _mm512_storeu_si512(sum_arr, avx_result);
        result += sum_arr[0];
        
        /* Test FMA with rounding */
        __m512d dvec1 = _mm512_set1_pd(1.0);
        __m512d dvec2 = _mm512_set1_pd(2.0);
        __m512d dvec3 = _mm512_set1_pd(3.0);
        __m512d dvec4 = _mm512_set1_pd(4.0);
        __m512d dvec5 = _mm512_set1_pd(5.0);
        __mmask8 dmask = 0xFF;
        
        __m512d fma_result = avx512_fma_multi_op(dvec1, dvec2, dvec3, dvec4, dvec5, dmask);
        double dsum_arr[8];
        _mm512_storeu_pd(dsum_arr, fma_result);
        result += (int)dsum_arr[0];
    }
#endif

#if TARGET_ARM
    /* Test ARM NEON operations */
    int32x4_t neon1 = vdupq_n_s32(1);
    int32x4_t neon2 = vdupq_n_s32(2);
    int32x4_t neon3 = vdupq_n_s32(3);
    int32x4_t neon4 = vdupq_n_s32(4);
    int32x4_t neon5 = vdupq_n_s32(5);
    int32x4_t neon6 = vdupq_n_s32(6);
    
    int32x4_t neon_result = neon_multi_operand(neon1, neon2, neon3, neon4, neon5, neon6);
    int32_t neon_sum = vgetq_lane_s32(neon_result, 0);
    result += neon_sum;
    
#ifdef __ARM_FEATURE_SVE
    /* Test SVE if available */
    svint32_t sve1 = svdup_s32(1);
    svint32_t sve2 = svdup_s32(2);
    svint32_t sve3 = svdup_s32(3);
    svint32_t sve4 = svdup_s32(4);
    svint32_t sve5 = svdup_s32(5);
    svint32_t sve6 = svdup_s32(6);
    svint32_t sve7 = svdup_s32(7);
    svint32_t sve8 = svdup_s32(8);
    
    svint32_t sve_result = sve_multi_operand(sve1, sve2, sve3, sve4, sve5, sve6, sve7, sve8);
    int32_t sve_sum = 0;
    svbool_t pg = svptrue_b32();
    sve_sum = svaddv_s32(pg, sve_result);
    result += sve_sum;
#endif
#endif

#if TARGET_PPC
    /* Test PowerPC Altivec */
    vector signed int ppc1 = (vector signed int){1, 2, 3, 4};
    vector signed int ppc2 = (vector signed int){5, 6, 7, 8};
    vector signed int ppc3 = (vector signed int){9, 10, 11, 12};
    vector signed int ppc4 = (vector signed int){13, 14, 15, 16};
    
    vector signed int ppc_result = ppc_multi_operand(ppc1, ppc2, ppc3, ppc4);
    int ppc_sum = ((int*)&ppc_result)[0];
    result += ppc_sum;
#endif

    /* Additional complex expression to encourage combining */
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10, x11 = 11;
    
    /* Expression with 11 variables that might combine */
    int combined = x1 * x2 + x3 * x4 + x5 * x6 + x7 * x8 + x9 * x10 + x11;
    result += combined;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
