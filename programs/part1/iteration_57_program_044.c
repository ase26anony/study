/* test_multi_operand_expansion.c
 * Compile with: gcc -O3 -march=native -ftree-vectorize -funsafe-math-optimizations -fdump-rtl-expand -fdump-rtl-combine -o test test_multi_operand_expansion.c
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* For atomic operations */
#include <stdatomic.h>

/* Architecture-specific headers */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#endif

#ifdef __ARM_ARCH
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#endif

#ifdef __PPC64__
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* This expression might be combined into a single multi-operand
     * instruction pattern during optimization */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline void inline_asm_11_operands(uint64_t *out, uint64_t a, uint64_t b,
                                          uint64_t c, uint64_t d, uint64_t e,
                                          uint64_t f, uint64_t g, uint64_t h,
                                          uint64_t i, uint64_t j) {
    /* Inline assembly with exactly 11 operands to force expansion
     * into the 11-operand case in optabs.cc */
    asm volatile (
        "/* 11-operand assembly block */\n\t"
        "mov %[out], %[a]\n\t"
        "add %[out], %[b]\n\t"
        "add %[out], %[c]\n\t"
        "add %[out], %[d]\n\t"
        "add %[out], %[e]\n\t"
        "add %[out], %[f]\n\t"
        "add %[out], %[g]\n\t"
        "add %[out], %[h]\n\t"
        "add %[out], %[i]\n\t"
        "add %[out], %[j]"
        : [out] "=r" (*out)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
}

/* Function using inline assembly with 10 operands */
static inline void inline_asm_10_operands(uint64_t *out, uint64_t a, uint64_t b,
                                          uint64_t c, uint64_t d, uint64_t e,
                                          uint64_t f, uint64_t g, uint64_t h,
                                          uint64_t i) {
    asm volatile (
        "/* 10-operand assembly block */\n\t"
        "mov %[out], %[a]\n\t"
        "add %[out], %[b]\n\t"
        "add %[out], %[c]\n\t"
        "add %[out], %[d]\n\t"
        "add %[out], %[e]\n\t"
        "add %[out], %[f]\n\t"
        "add %[out], %[g]\n\t"
        "add %[out], %[h]\n\t"
        "add %[out], %[i]"
        : [out] "=r" (*out)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i)
        : "cc"
    );
}

#ifdef __x86_64__
/* AVX-512 example that could use many operands */
static inline __m512d avx512_complex_fma(__m512d a, __m512d b, __m512d c,
                                         __m512d d, __m512d e, __m512d f,
                                         __m512d g, __m512d h, __m512d i,
                                         __m512d j, __mmask8 k) {
    /* Chain of FMA operations that might be combined */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    __m512d t3 = _mm512_fmadd_pd(g, h, i);
    
    /* Masked operation with many parameters */
    __m512d result = _mm512_mask_fmadd_pd(t1, k, t2, t3, j);
    
    return result;
}

/* AVX-512 masked operation with 10+ parameters */
static inline __m512i avx512_masked_blend(__m512i a, __m512i b, __m512i c,
                                          __m512i d, __m512i e, __m512i f,
                                          __mmask16 m1, __mmask16 m2,
                                          __mmask16 m3, int imm) {
    /* Complex sequence that might expand to multi-operand pattern */
    __m512i t1 = _mm512_mask_blend_epi32(m1, a, b);
    __m512i t2 = _mm512_mask_blend_epi32(m2, c, d);
    __m512i t3 = _mm512_mask_blend_epi32(m3, e, f);
    
    /* Permute with immediate - adds another operand */
    __m512i result = _mm512_permutexvar_epi32(t1, t2);
    result = _mm512_add_epi32(result, t3);
    
    return result;
}
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
/* SVE2 example with lane operations */
static inline svint32_t sve2_complex_operation(svint32_t a, svint32_t b,
                                               svint32_t c, svint32_t d,
                                               svint32_t e, svint32_t f,
                                               svint32_t g, svint32_t h,
                                               uint64_t lane1, uint64_t lane2,
                                               svbool_t pg) {
    /* Complex SVE2 operation that might use many operands */
    svint32_t t1 = svmla_lane_s32(pg, a, b, lane1);
    svint32_t t2 = svmla_lane_s32(pg, c, d, lane2);
    svint32_t t3 = svadd_s32_z(pg, e, f);
    svint32_t t4 = svadd_s32_z(pg, g, h);
    
    return svadd_s32_z(pg, svadd_s32_z(pg, t1, t2), svadd_s32_z(pg, t3, t4));
}
#endif

/* NEON example with many operands */
static inline int32x4_t neon_complex_mla(int32x4_t a, int32x4_t b, int32x4_t c,
                                         int32x4_t d, int32x4_t e, int32x4_t f,
                                         int32x4_t g, int32x4_t h, int32x4_t i,
                                         int32x4_t j, int lane) {
    /* Chain of MLA operations */
    int32x4_t t1 = vmlaq_lane_s32(a, b, c, lane);
    int32x4_t t2 = vmlaq_lane_s32(d, e, f, lane);
    int32x4_t t3 = vmlaq_lane_s32(g, h, i, lane);
    
    return vaddq_s32(t1, vaddq_s32(t2, t3));
}
#endif

#ifdef __PPC64__
/* PowerPC VSX/Altivec example */
static inline vector signed int ppc_complex_permute(vector signed int a,
                                                    vector signed int b,
                                                    vector signed int c,
                                                    vector signed int d,
                                                    vector signed int e,
                                                    vector signed int f,
                                                    vector unsigned char perm1,
                                                    vector unsigned char perm2,
                                                    vector unsigned char perm3,
                                                    vector unsigned char perm4) {
    /* Complex permute and compute sequence */
    vector signed int t1 = vec_perm(a, b, perm1);
    vector signed int t2 = vec_perm(c, d, perm2);
    vector signed int t3 = vec_perm(e, f, perm3);
    
    vector signed int r1 = vec_add(t1, t2);
    vector signed int r2 = vec_perm(r1, t3, perm4);
    
    return vec_add(r2, vec_add(a, b));
}
#endif

/* Atomic operation with many parameters */
static inline int atomic_complex_exchange(_Atomic(int64_t) *atom,
                                          int64_t *expected,
                                          int64_t desired,
                                          int success_memorder,
                                          int failure_memorder) {
    /* __atomic_compare_exchange with 6 parameters total */
    return __atomic_compare_exchange(atom, expected, &desired,
                                     0, /* weak */
                                     success_memorder,
                                     failure_memorder);
}

/* Decimal floating point built-in (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static inline _Decimal128 decimal_complex_operation(_Decimal128 a,
                                                    _Decimal128 b,
                                                    _Decimal128 c,
                                                    _Decimal128 d,
                                                    _Decimal128 e,
                                                    _Decimal128 f,
                                                    _Decimal128 g,
                                                    _Decimal128 h,
                                                    _Decimal128 i,
                                                    _Decimal128 j) {
    /* Chain of decimal operations */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(c, d);
    _Decimal128 t3 = __bid128_add(e, f);
    _Decimal128 t4 = __bid128_mul(g, h);
    _Decimal128 t5 = __bid128_add(i, j);
    
    _Decimal128 r1 = __bid128_add(t1, t2);
    _Decimal128 r2 = __bid128_add(t3, t4);
    
    return __bid128_add(r1, __bid128_add(r2, t5));
}
#endif

int main() {
    int result = 0;
    
    /* Test 1: Complex expression with 10+ variables */
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5;
        int f = 6, g = 7, h = 8, i = 9, j = 10;
        
        result += complex_expression(a, b, c, d, e, f, g, h, i, j);
    }
    
    /* Test 2: Inline assembly with 11 operands */
    {
        uint64_t out;
        inline_asm_11_operands(&out, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        result += (int)out;
    }
    
    /* Test 3: Inline assembly with 10 operands */
    {
        uint64_t out;
        inline_asm_10_operands(&out, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        result += (int)out;
    }
    
    /* Test 4: Atomic operation with many parameters */
    {
        _Atomic(int64_t) atom = 100;
        int64_t expected = 100;
        int64_t desired = 200;
        
        if (atomic_complex_exchange(&atom, &expected, desired,
                                    __ATOMIC_SEQ_CST,
                                    __ATOMIC_SEQ_CST)) {
            result += 1;
        }
    }
    
    /* Test 5: Architecture-specific vector operations */
#ifdef __x86_64__
    {
        __m512d v1 = _mm512_set1_pd(1.0);
        __m512d v2 = _mm512_set1_pd(2.0);
        __m512d v3 = _mm512_set1_pd(3.0);
        __m512d v4 = _mm512_set1_pd(4.0);
        __m512d v5 = _mm512_set1_pd(5.0);
        __m512d v6 = _mm512_set1_pd(6.0);
        __m512d v7 = _mm512_set1_pd(7.0);
        __m512d v8 = _mm512_set1_pd(8.0);
        __m512d v9 = _mm512_set1_pd(9.0);
        __m512d v10 = _mm512_set1_pd(10.0);
        
        __m512d r = avx512_complex_fma(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, 0xFF);
        
        double sum[8];
        _mm512_storeu_pd(sum, r);
        result += (int)sum[0];
    }
#endif
    
#ifdef __ARM_ARCH
    {
        int32x4_t v1 = vdupq_n_s32(1);
        int32x4_t v2 = vdupq_n_s32(2);
        int32x4_t v3 = vdupq_n_s32(3);
        int32x4_t v4 = vdupq_n_s32(4);
        int32x4_t v5 = vdupq_n_s32(5);
        int32x4_t v6 = vdupq_n_s32(6);
        int32x4_t v7 = vdupq_n_s32(7);
        int32x4_t v8 = vdupq_n_s32(8);
        int32x4_t v9 = vdupq_n_s32(9);
        int32x4_t v10 = vdupq_n_s32(10);
        
        int32x4_t r = neon_complex_mla(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, 0);
        
        result += vgetq_lane_s32(r, 0);
    }
#endif
    
#ifdef __PPC64__
    {
        vector signed int v1 = {1, 2, 3, 4};
        vector signed int v2 = {5, 6, 7, 8};
        vector signed int v3 = {9, 10, 11, 12};
        vector signed int v4 = {13, 14, 15, 16};
        vector signed int v5 = {17, 18, 19, 20};
        vector signed int v6 = {21, 22, 23, 24};
        
        vector unsigned char perm1 = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        vector unsigned char perm2 = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
        vector unsigned char perm3 = {4,5,6,7,0,1,2,3,12,13,14,15,8,9,10,11};
        vector unsigned char perm4 = {7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8};
        
        vector signed int r = ppc_complex_permute(v1, v2, v3, v4, v5, v6,
                                                  perm1, perm2, perm3, perm4);
        
        result += vec_extract(r, 0);
    }
#endif
    
    /* Test 6: Fused multiply-add chain */
    {
        double a = 1.1, b = 2.2, c = 3.3, d = 4.4, e = 5.5;
        double f = 6.6, g = 7.7, h = 8.8, i = 9.9, j = 10.10;
        
        /* Complex expression that might be combined */
        double chain_result = a * b + c * d + e * f + g * h + i * j;
        result += (int)chain_result;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
