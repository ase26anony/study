/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Generic fallback for architectures without specific intrinsics */
static inline uint64_t generic_10_operand_op(uint64_t a, uint64_t b, uint64_t c,
                                            uint64_t d, uint64_t e, uint64_t f,
                                            uint64_t g, uint64_t h, uint64_t i,
                                            uint64_t j) {
    /* Complex expression that might combine into multi-operand instruction */
    return a * b + c * d + e * f + g * h + i * j;
}

/* x86 AVX-512 specific code */
#ifdef __x86_64__
#include <immintrin.h>

static __m512i avx512_10_operand_test(__m512i a, __m512i b, __m512i c,
                                      __m512i d, __m512i e, __m512i f,
                                      __m512i g, __m512i h, __m512i i,
                                      __m512i j) {
    /* AVX-512 masked operations with multiple operands */
    __mmask16 mask = 0xAAAA;
    
    /* Complex chain of operations that might combine */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_mullo_epi32(c, d);
    __m512i t3 = _mm512_mask_add_epi32(t1, mask, e, f);
    __m512i t4 = _mm512_mask_mullo_epi32(t2, mask, g, h);
    __m512i result = _mm512_add_epi32(t3, t4);
    result = _mm512_add_epi32(result, i);
    result = _mm512_add_epi32(result, j);
    
    return result;
}

static double avx512_fma_11_operand_test(__m512d a, __m512d b, __m512d c,
                                        __m512d d, __m512d e, __m512d f,
                                        __m512d g, __m512d h, __m512d i,
                                        __m512d j, __m512d k) {
    /* FMA operations with multiple operands - might combine */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    __m512d t3 = _mm512_fmadd_pd(g, h, i);
    __m512d result = _mm512_add_pd(t1, t2);
    result = _mm512_add_pd(result, t3);
    result = _mm512_add_pd(result, j);
    result = _mm512_add_pd(result, k);
    
    /* Reduce to scalar */
    return _mm512_reduce_add_pd(result);
}
#endif

/* ARM SVE2 specific code */
#ifdef __ARM_ARCH
#if __ARM_ARCH >= 8 && defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>

static svint32_t sve2_10_operand_test(svint32_t a, svint32_t b, svint32_t c,
                                     svint32_t d, svint32_t e, svint32_t f,
                                     svint32_t g, svint32_t h, svint32_t i,
                                     svint32_t j) {
    /* SVE2 operations with lane selection - potentially many operands */
    svbool_t pg = svptrue_b32();
    
    /* Complex expression that might use multi-operand instructions */
    svint32_t t1 = svmla_s32_z(pg, a, b, c);
    svint32_t t2 = svmla_s32_z(pg, d, e, f);
    svint32_t t3 = svadd_s32_z(pg, g, h);
    svint32_t result = svadd_s32_z(pg, t1, t2);
    result = svadd_s32_z(pg, result, t3);
    result = svadd_s32_z(pg, result, i);
    result = svadd_s32_z(pg, result, j);
    
    return result;
}
#endif
#endif

/* PowerPC VSX specific code */
#ifdef __PPC64__
#include <altivec.h>

static vector signed int vsx_10_operand_test(vector signed int a,
                                            vector signed int b,
                                            vector signed int c,
                                            vector signed int d,
                                            vector signed int e,
                                            vector signed int f,
                                            vector signed int g,
                                            vector signed int h,
                                            vector signed int i,
                                            vector signed int j) {
    /* VSX permute and compute operations */
    vector signed int t1 = vec_madd(a, b, c);
    vector signed int t2 = vec_madd(d, e, f);
    vector signed int t3 = vec_add(g, h);
    vector signed int result = vec_add(t1, t2);
    result = vec_add(result, t3);
    result = vec_add(result, i);
    result = vec_add(result, j);
    
    return result;
}
#endif

/* Function using inline assembly with exactly 11 operands */
static uint64_t inline_asm_11_operand_test(uint64_t a, uint64_t b, uint64_t c,
                                          uint64_t d, uint64_t e, uint64_t f,
                                          uint64_t g, uint64_t h, uint64_t i,
                                          uint64_t j, uint64_t k) {
    uint64_t result1, result2, result3;
    
    /* Inline assembly with 11 operands to force 11-operand expansion */
    asm volatile (
        /* Template doesn't matter much - we care about operand count */
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result1), "=r" (result2), "=r" (result3)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    return result1 + result2 + result3 + k;
}

/* Function using atomic built-in with many parameters */
static int atomic_10_operand_test(int *ptr, int *expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, but expands to complex RTL */
    return __atomic_compare_exchange(ptr, expected, &desired,
                                     weak, success_memorder,
                                     failure_memorder);
}

/* Complex expression that might combine into multi-operand instruction */
static int complex_expression_10_operand(int a, int b, int c, int d, int e,
                                        int f, int g, int h, int i, int j) {
    /* Expression designed to potentially match multi-operand pattern */
    return ((a * b) + (c * d) + (e * f) + (g * h) + (i * j)) *
           ((a + b) * (c + d) * (e + f) * (g + h) * (i + j));
}

int main() {
    uint64_t result = 0;
    
    /* Test 1: Generic 10-operand operation */
    result += generic_10_operand_op(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test 2: Inline assembly with 11 operands */
    result += inline_asm_11_operand_test(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test 3: Complex expression */
    result += complex_expression_10_operand(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test 4: Atomic operation */
    int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    result += atomic_10_operand_test(&atomic_var, &expected, desired);
    
    /* Architecture-specific tests */
#ifdef __x86_64__
    /* Initialize AVX-512 vectors */
    __m512i vec1 = _mm512_set1_epi32(1);
    __m512i vec2 = _mm512_set1_epi32(2);
    __m512i vec3 = _mm512_set1_epi32(3);
    __m512i vec4 = _mm512_set1_epi32(4);
    __m512i vec5 = _mm512_set1_epi32(5);
    __m512i vec6 = _mm512_set1_epi32(6);
    __m512i vec7 = _mm512_set1_epi32(7);
    __m512i vec8 = _mm512_set1_epi32(8);
    __m512i vec9 = _mm512_set1_epi32(9);
    __m512i vec10 = _mm512_set1_epi32(10);
    
    __m512i avx_result = avx512_10_operand_test(vec1, vec2, vec3, vec4, vec5,
                                               vec6, vec7, vec8, vec9, vec10);
    
    /* Extract and add to result */
    int avx_sum[16];
    _mm512_storeu_si512(avx_sum, avx_result);
    for (int idx = 0; idx < 16; idx++) {
        result += avx_sum[idx];
    }
    
    /* Test FMA with 11 operands */
    __m512d dvec1 = _mm512_set1_pd(1.0);
    __m512d dvec2 = _mm512_set1_pd(2.0);
    __m512d dvec3 = _mm512_set1_pd(3.0);
    __m512d dvec4 = _mm512_set1_pd(4.0);
    __m512d dvec5 = _mm512_set1_pd(5.0);
    __m512d dvec6 = _mm512_set1_pd(6.0);
    __m512d dvec7 = _mm512_set1_pd(7.0);
    __m512d dvec8 = _mm512_set1_pd(8.0);
    __m512d dvec9 = _mm512_set1_pd(9.0);
    __m512d dvec10 = _mm512_set1_pd(10.0);
    __m512d dvec11 = _mm512_set1_pd(11.0);
    
    double fma_result = avx512_fma_11_operand_test(dvec1, dvec2, dvec3, dvec4,
                                                  dvec5, dvec6, dvec7, dvec8,
                                                  dvec9, dvec10, dvec11);
    result += (uint64_t)fma_result;
#endif

#ifdef __ARM_ARCH
#if __ARM_ARCH >= 8 && defined(__ARM_FEATURE_SVE)
    /* ARM SVE2 test would go here */
    /* Note: Actual SVE code requires runtime length queries */
#endif
#endif

#ifdef __PPC64__
    /* PowerPC VSX test */
    vector signed int v1 = {1, 2, 3, 4};
    vector signed int v2 = {5, 6, 7, 8};
    vector signed int v3 = {9, 10, 11, 12};
    vector signed int v4 = {13, 14, 15, 16};
    vector signed int v5 = {17, 18, 19, 20};
    vector signed int v6 = {21, 22, 23, 24};
    vector signed int v7 = {25, 26, 27, 28};
    vector signed int v8 = {29, 30, 31, 32};
    vector signed int v9 = {33, 34, 35, 36};
    vector signed int v10 = {37, 38, 39, 40};
    
    vector signed int vsx_result = vsx_10_operand_test(v1, v2, v3, v4, v5,
                                                      v6, v7, v8, v9, v10);
    
    /* Extract results */
    int vsx_array[4];
    vec_store(vsx_result, vsx_array);
    for (int idx = 0; idx < 4; idx++) {
        result += vsx_array[idx];
    }
#endif

    printf("Final result: %lu\n", (unsigned long)result);
    return (result > 0) ? 0 : 1;
}
