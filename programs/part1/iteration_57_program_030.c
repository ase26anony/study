/* Test program to trigger 10/11-operand expansion in optabs.cc */
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
#elif defined(__powerpc64__) || defined(__powerpc__)
    #define TARGET_PPC 1
    #include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* This expression might be combined into a single instruction
       with many operands during optimization */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with exactly 11 operands to trigger the 11-operand case */
    __asm__ volatile (
        /* Simple operation that uses all operands - actual instruction depends on target */
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

/* Atomic built-in with many parameters */
static int atomic_multi_operand(int *ptr, int expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, which might expand
       to a multi-operand instruction on some architectures */
    __atomic_compare_exchange(ptr, &expected, &desired, weak,
                              success_memorder, failure_memorder);
    
    return expected;
}

#if TARGET_X86
/* AVX-512 intrinsics that can use many operands */
static __m512i avx512_multi_operand(__m512i a, __m512i b, __m512i c,
                                    __m512i d, __m512i e, __mmask16 k) {
    /* Chain multiple operations that might be combined */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(t1, t2);
    __m512i t4 = _mm512_add_epi32(t3, e);
    
    /* Masked operation with multiple operands */
    return _mm512_mask_add_epi32(t4, k, t4, _mm512_set1_epi32(1));
}

/* FMA operation with rounding control - can have many operands */
static __m512d avx512_fma_multi_operand(__m512d a, __m512d b, __m512d c,
                                        __mmask8 k, int rounding) {
    /* This intrinsic uses: a, b, c, k, rounding - 5 explicit operands
       but expands to more when including implicit registers */
    return _mm512_maskz_fmadd_round_pd(k, a, b, c, rounding);
}
#endif

#if TARGET_ARM
/* ARM NEON/SVE intrinsics with lane selection */
static int32x4_t neon_multi_lane(int32x4_t a, int32x4_t b, int32x4_t c,
                                 int32x4_t d, int32x4_t e) {
    /* Complex operation that might use multiple lanes */
    int32x4_t t1 = vmlaq_laneq_s32(a, b, c, 0);
    int32x4_t t2 = vmlaq_laneq_s32(t1, d, e, 1);
    int32x4_t t3 = vmlaq_laneq_s32(t2, a, b, 2);
    return vmlaq_laneq_s32(t3, c, d, 3);
}

#ifdef __ARM_FEATURE_SVE
/* SVE2 intrinsics that can have many operands */
static svint32_t sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                   svint32_t d, svint32_t e, svint32_t f,
                                   svbool_t pg) {
    /* SVE dot product with lane - potentially many operands */
    return svdot_lane_s32(a, b, c, 0);
}
#endif
#endif

#if TARGET_PPC
/* PowerPC Altivec/VSX operations */
static vector int ppc_multi_operand(vector int a, vector int b, vector int c,
                                    vector int d, vector int e) {
    /* Complex permute and compute */
    vector int t1 = vec_madd(a, b, c);
    vector int t2 = vec_madd(d, e, t1);
    vector int t3 = vec_perm(t1, t2, (vector unsigned char){0,1,2,3,4,5,6,7,
                                                             8,9,10,11,12,13,14,15});
    return vec_add(t3, a);
}
#endif

/* Bit-field operations that might combine */
static uint64_t bitfield_multi_operand(uint64_t val, uint64_t mask1,
                                       uint64_t mask2, uint64_t mask3,
                                       uint64_t mask4, uint64_t mask5,
                                       int shift1, int shift2, int shift3,
                                       int shift4) {
    /* Multiple bit-field extractions and insertions */
    uint64_t result = 0;
    result |= (val & mask1) << shift1;
    result |= (val & mask2) >> shift2;
    result |= (val & mask3) << shift3;
    result |= (val & mask4) >> shift4;
    result |= (val & mask5);
    return result;
}

int main() {
    uint64_t sum = 0;
    
    /* Test complex expression with 10 operands */
    int expr_result = complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    sum += expr_result;
    
    /* Test inline assembly with 11 operands */
    uint64_t asm_result = multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    sum += asm_result;
    
    /* Test atomic built-in */
    int atomic_var = 42;
    int atomic_result = atomic_multi_operand(&atomic_var, 42, 100);
    sum += atomic_result;
    
    /* Test bit-field operations */
    uint64_t bitfield_result = bitfield_multi_operand(0x123456789ABCDEF0ULL,
                                                      0xFF00000000000000ULL,
                                                      0x00FF000000000000ULL,
                                                      0x0000FF0000000000ULL,
                                                      0x000000FF00000000ULL,
                                                      0x00000000FF000000ULL,
                                                      8, 8, 16, 16);
    sum += bitfield_result;
    
#if TARGET_X86
    /* Test AVX-512 operations if available */
    if (__builtin_cpu_supports("avx512f")) {
        __m512i avx_a = _mm512_set1_epi32(1);
        __m512i avx_b = _mm512_set1_epi32(2);
        __m512i avx_c = _mm512_set1_epi32(3);
        __m512i avx_d = _mm512_set1_epi32(4);
        __m512i avx_e = _mm512_set1_epi32(5);
        __mmask16 mask = 0xFFFF;
        
        __m512i avx_result = avx512_multi_operand(avx_a, avx_b, avx_c,
                                                  avx_d, avx_e, mask);
        
        /* Extract one element to add to sum */
        int avx_array[16];
        _mm512_storeu_si512(avx_array, avx_result);
        sum += avx_array[0];
    }
#endif
    
#if TARGET_ARM
    /* Test NEON operations */
    int32x4_t neon_a = {1, 2, 3, 4};
    int32x4_t neon_b = {5, 6, 7, 8};
    int32x4_t neon_c = {9, 10, 11, 12};
    int32x4_t neon_d = {13, 14, 15, 16};
    int32x4_t neon_e = {17, 18, 19, 20};
    
    int32x4_t neon_result = neon_multi_lane(neon_a, neon_b, neon_c,
                                            neon_d, neon_e);
    
    /* Extract one element */
    int neon_array[4];
    vst1q_s32(neon_array, neon_result);
    sum += neon_array[0];
#endif
    
#if TARGET_PPC
    /* Test PowerPC vector operations */
    vector int ppc_a = {1, 2, 3, 4};
    vector int ppc_b = {5, 6, 7, 8};
    vector int ppc_c = {9, 10, 11, 12};
    vector int ppc_d = {13, 14, 15, 16};
    vector int ppc_e = {17, 18, 19, 20};
    
    vector int ppc_result = ppc_multi_operand(ppc_a, ppc_b, ppc_c,
                                              ppc_d, ppc_e);
    
    /* Extract one element */
    int ppc_array[4];
    vec_st(ppc_result, 0, ppc_array);
    sum += ppc_array[0];
#endif
    
    printf("Result: %lu\n", (unsigned long)sum);
    return (int)(sum % 256); /* Return non-zero to prevent optimization */
}
