/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define HAS_X86 1
#include <immintrin.h>
#include <x86intrin.h>
#endif

#if defined(__aarch64__) || defined(__arm__)
#define HAS_ARM 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#endif

#if defined(__powerpc__) || defined(__PPC__)
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
    uint64_t result;
    
    /* Inline assembly with exactly 11 operands to force 11-operand expansion */
    asm volatile (
        /* Simple template - the important part is the operand count */
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

/* Atomic operation with many parameters */
static inline int atomic_multi_op(int *ptr, int expected, int desired) {
    int weak = 0;
    /* __atomic_compare_exchange has 6 parameters, which when expanded
       with memory ordering parameters could reach 10+ operands */
    __atomic_compare_exchange(ptr, &expected, &desired, weak,
                              __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}

#if HAS_X86
/* AVX-512 operations with many operands */
static inline __m512i avx512_multi_operand(__m512i a, __m512i b, __m512i c,
                                           __m512i d, __m512i e, __m512i f,
                                           __m512i g, __m512i h, __m512i i,
                                           __m512i j) {
    /* Chain of operations that might be combined */
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

/* AVX-512 masked operation with many parameters */
static inline __m512d avx512_masked_fma(__m512d a, __m512d b, __m512d c,
                                        __m512d d, __m512d e, __m512d f,
                                        __m512d g, __m512d h, __m512d i,
                                        __m512d j, __mmask8 k) {
    /* Complex expression that might use masked FMA */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    __m512d t3 = _mm512_fmadd_pd(g, h, i);
    
    /* Masked operation with many operands */
    __m512d result = _mm512_mask_add_pd(t1, k, t2, t3);
    result = _mm512_mask_add_pd(result, k, result, j);
    
    return result;
}
#endif

#if HAS_ARM
/* ARM NEON/SVE operations with many operands */
static inline int32x4_t neon_multi_lane(int32x4_t a, int32x4_t b, int32x4_t c,
                                        int32x4_t d, int32x4_t e, int32x4_t f,
                                        int32x4_t g, int32x4_t h, int32x4_t i,
                                        int32x4_t j) {
    /* Multiple lane operations */
    int32x4_t t1 = vmlaq_laneq_s32(a, b, c, 0);
    int32x4_t t2 = vmlaq_laneq_s32(d, e, f, 1);
    int32x4_t t3 = vmlaq_laneq_s32(g, h, i, 2);
    
    /* Combine results */
    int32x4_t sum1 = vaddq_s32(t1, t2);
    int32x4_t sum2 = vaddq_s32(sum1, t3);
    
    return vaddq_s32(sum2, j);
}

#ifdef __ARM_FEATURE_SVE
/* SVE2 operation with many parameters (simulated - actual SVE has fewer) */
static inline svint32_t sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                          svint32_t d, svint32_t e, svint32_t f,
                                          svint32_t g, svint32_t h, svint32_t i,
                                          svint32_t j, svbool_t pg) {
    /* Chain of SVE operations */
    svint32_t t1 = svadd_s32_z(pg, a, b);
    svint32_t t2 = svadd_s32_z(pg, c, d);
    svint32_t t3 = svadd_s32_z(pg, e, f);
    svint32_t t4 = svadd_s32_z(pg, g, h);
    svint32_t t5 = svadd_s32_z(pg, i, j);
    
    svint32_t sum1 = svadd_s32_z(pg, t1, t2);
    svint32_t sum2 = svadd_s32_z(pg, t3, t4);
    svint32_t sum3 = svadd_s32_z(pg, sum1, sum2);
    
    return svadd_s32_z(pg, sum3, t5);
}
#endif
#endif

#if HAS_PPC
/* PowerPC Altivec operations */
static inline vector signed int altivec_multi_operand(vector signed int a,
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

/* Bit-field operations across multiple words */
static inline uint64_t bitfield_multi_op(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j) {
    /* Complex bit manipulation that might combine */
    uint64_t result = 0;
    
    /* Extract and insert bits from 10 different values */
    result |= (a & 0xFF) << 0;
    result |= (b & 0xFF) << 8;
    result |= (c & 0xFF) << 16;
    result |= (d & 0xFF) << 24;
    result |= (e & 0xFF) << 32;
    result |= (f & 0xFF) << 40;
    result |= (g & 0xFF) << 48;
    result |= (h & 0xFF) << 56;
    
    /* Use remaining values for arithmetic */
    result += i * j;
    
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
    result += atomic_multi_op(&atomic_var, 42, 100);
    
    /* Test bitfield operations */
    result += bitfield_multi_op(0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
                                0x11, 0x22, 0x33, 0x44);
    
#if HAS_X86
    /* Test AVX-512 operations if available */
    if (__builtin_cpu_supports("avx512f")) {
        __m512i vec_a = _mm512_set1_epi32(1);
        __m512i vec_b = _mm512_set1_epi32(2);
        __m512i vec_c = _mm512_set1_epi32(3);
        __m512i vec_d = _mm512_set1_epi32(4);
        __m512i vec_e = _mm512_set1_epi32(5);
        __m512i vec_f = _mm512_set1_epi32(6);
        __m512i vec_g = _mm512_set1_epi32(7);
        __m512i vec_h = _mm512_set1_epi32(8);
        __m512i vec_i = _mm512_set1_epi32(9);
        __m512i vec_j = _mm512_set1_epi32(10);
        
        __m512i vec_result = avx512_multi_operand(vec_a, vec_b, vec_c, vec_d,
                                                  vec_e, vec_f, vec_g, vec_h,
                                                  vec_i, vec_j);
        
        /* Extract one element to prevent dead code elimination */
        int avx_result = _mm512_extract_epi32(vec_result, 0);
        result += avx_result;
    }
#endif
    
#if HAS_ARM
    /* Test ARM NEON operations */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    int32x4_t neon_f = vdupq_n_s32(6);
    int32x4_t neon_g = vdupq_n_s32(7);
    int32x4_t neon_h = vdupq_n_s32(8);
    int32x4_t neon_i = vdupq_n_s32(9);
    int32x4_t neon_j = vdupq_n_s32(10);
    
    int32x4_t neon_result = neon_multi_lane(neon_a, neon_b, neon_c, neon_d,
                                            neon_e, neon_f, neon_g, neon_h,
                                            neon_i, neon_j);
    
    /* Extract result */
    int neon_sum = vgetq_lane_s32(neon_result, 0);
    result += neon_sum;
#endif
    
#if HAS_PPC
    /* Test PowerPC Altivec operations */
    vector signed int ppc_a = {1, 1, 1, 1};
    vector signed int ppc_b = {2, 2, 2, 2};
    vector signed int ppc_c = {3, 3, 3, 3};
    vector signed int ppc_d = {4, 4, 4, 4};
    vector signed int ppc_e = {5, 5, 5, 5};
    vector signed int ppc_f = {6, 6, 6, 6};
    vector signed int ppc_g = {7, 7, 7, 7};
    vector signed int ppc_h = {8, 8, 8, 8};
    vector signed int ppc_i = {9, 9, 9, 9};
    vector signed int ppc_j = {10, 10, 10, 10};
    
    vector signed int ppc_result = altivec_multi_operand(ppc_a, ppc_b, ppc_c,
                                                         ppc_d, ppc_e, ppc_f,
                                                         ppc_g, ppc_h, ppc_i,
                                                         ppc_j);
    
    /* Extract result */
    int ppc_sum = ((int*)&ppc_result)[0];
    result += ppc_sum;
#endif
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
