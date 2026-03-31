/* test_multi_operand.c - Test program for GCC optabs 10/11-operand expansion */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Generic fallback for architectures without specific intrinsics */
static inline uint64_t generic_multi_operand_expression(uint64_t a, uint64_t b,
                                                        uint64_t c, uint64_t d,
                                                        uint64_t e, uint64_t f,
                                                        uint64_t g, uint64_t h,
                                                        uint64_t i, uint64_t j) {
    /* Complex expression that might combine into multi-operand instruction */
    return a * b + c * d + e * f + g * h + i * j +
           (a & b) | (c & d) | (e & f) | (g & h) | (i & j);
}

/* x86 AVX-512 specific code */
#ifdef __x86_64__
#include <immintrin.h>

static __m512i avx512_multi_operand_test(__m512i a, __m512i b, __m512i c,
                                         __m512i d, __m512i e, __m512i f,
                                         __m512i g, __m512i h, __m512i i,
                                         __m512i j) {
    /* AVX-512 masked operations with many operands */
    __mmask16 mask = 0xAAAA;
    
    /* This could potentially expand to multi-operand instruction */
    __m512i temp1 = _mm512_mask_add_epi32(a, mask, b, c);
    __m512i temp2 = _mm512_mask_mul_epi32(d, mask, e, f);
    __m512i temp3 = _mm512_mask_add_epi32(g, mask, h, i);
    
    /* Complex chain that might combine */
    return _mm512_mask_add_epi32(temp1, mask,
           _mm512_mask_mul_epi32(temp2, mask, temp3, j), a);
}

/* AVX-512 FP multi-operand fused operations */
static __m512d avx512_fma_multi_operand(__m512d a, __m512d b, __m512d c,
                                        __m512d d, __m512d e, __m512d f,
                                        __m512d g, __m512d h, __m512d i,
                                        __m512d j, __mmask8 k) {
    /* FMA with mask and rounding - up to 11 operands */
    return _mm512_mask3_fmadd_round_pd(a, b, c, k,
           _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC) +
           _mm512_mask3_fmadd_round_pd(d, e, f, k,
           _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC) +
           _mm512_mask3_fmadd_round_pd(g, h, i, k,
           _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC) + j;
}
#endif

/* ARM SVE2 specific code */
#ifdef __ARM_ARCH
#if __ARM_ARCH >= 8
#include <arm_sve.h>

static svint32_t sve2_multi_operand_test(svint32_t a, svint32_t b, svint32_t c,
                                         svint32_t d, svint32_t e, svint32_t f,
                                         svint32_t g, svint32_t h, svint32_t i,
                                         svint32_t j, svbool_t pg) {
    /* SVE2 lane operations with many vector operands */
    svint32_t temp1 = svmla_lane_s32(a, b, c, 0);
    svint32_t temp2 = svmla_lane_s32(d, e, f, 1);
    svint32_t temp3 = svmla_lane_s32(g, h, i, 2);
    
    /* Complex expression that might require many operands */
    return svadd_s32_z(pg,
           svadd_s32_z(pg, temp1, temp2),
           svadd_s32_z(pg, temp3, j));
}
#endif
#endif

/* PowerPC VSX specific code */
#ifdef __PPC64__
#include <altivec.h>

static vector signed int vsx_multi_operand_test(vector signed int a,
                                                vector signed int b,
                                                vector signed int c,
                                                vector signed int d,
                                                vector signed int e,
                                                vector signed int f,
                                                vector signed int g,
                                                vector signed int h,
                                                vector signed int i,
                                                vector signed int j) {
    /* VSX complex permute and compute */
    vector signed int temp1 = vec_madd(a, b, c);
    vector signed int temp2 = vec_madd(d, e, f);
    vector signed int temp3 = vec_madd(g, h, i);
    
    /* Permute and combine - could require many operands */
    return vec_add(vec_perm(temp1, temp2, (vector unsigned char){0}),
                   vec_add(temp3, j));
}
#endif

/* Inline assembly with exactly 11 operands */
static uint64_t inline_asm_11_operands(uint64_t a, uint64_t b, uint64_t c,
                                       uint64_t d, uint64_t e, uint64_t f,
                                       uint64_t g, uint64_t h, uint64_t i,
                                       uint64_t j, uint64_t k) {
    uint64_t result1, result2, result3, result4, result5;
    
    /* Inline asm with 11 operands to force optabs expansion */
    asm volatile (
        /* Template doesn't matter much - we care about operand count */
        "mov %[r1], %[a]\n\t"
        "add %[r1], %[b]\n\t"
        "add %[r1], %[c]\n\t"
        "add %[r1], %[d]\n\t"
        "add %[r1], %[e]\n\t"
        "mov %[r2], %[f]\n\t"
        "add %[r2], %[g]\n\t"
        "add %[r2], %[h]\n\t"
        "add %[r2], %[i]\n\t"
        "add %[r2], %[j]\n\t"
        "add %[r1], %[r2]\n\t"
        "mov %[r3], %[k]"
        : [r1] "=&r" (result1),
          [r2] "=&r" (result2),
          [r3] "=r" (result3),
          [r4] "=r" (result4),
          [r5] "=r" (result5)
        : [a] "r" (a),
          [b] "r" (b),
          [c] "r" (c),
          [d] "r" (d),
          [e] "r" (e),
          [f] "r" (f),
          [g] "r" (g),
          [h] "r" (h),
          [i] "r" (i),
          [j] "r" (j),
          [k] "r" (k)
        : "cc"
    );
    
    return result1 + result2 + result3 + result4 + result5;
}

/* Atomic built-in with many parameters */
static int atomic_multi_operand_test(int64_t *ptr, int64_t *expected,
                                     int64_t desired, int memorder_success,
                                     int memorder_failure) {
    int64_t expected_val = *expected;
    
    /* __atomic_compare_exchange with 6 parameters */
    return __atomic_compare_exchange(ptr, expected, &desired, 0,
                                     memorder_success, memorder_failure);
}

/* Complex expression that might combine into multi-operand instruction */
static uint64_t complex_expression_10_operands(uint64_t v0, uint64_t v1,
                                               uint64_t v2, uint64_t v3,
                                               uint64_t v4, uint64_t v5,
                                               uint64_t v6, uint64_t v7,
                                               uint64_t v8, uint64_t v9) {
    /* Expression designed to potentially match multi-operand pattern */
    return ((v0 * v1 + v2 * v3) |
            (v4 * v5 + v6 * v7) |
            (v8 * v9 + v0 & v1) |
            (v2 & v3 + v4 & v5) |
            (v6 & v7 + v8 & v9)) +
           ((v0 | v1) & (v2 | v3) & (v4 | v5) & (v6 | v7) & (v8 | v9));
}

int main() {
    uint64_t result = 0;
    
    /* Test generic multi-operand expression */
    result += generic_multi_operand_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test inline assembly with 11 operands */
    result += inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test complex expression with 10 operands */
    result += complex_expression_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test atomic operation */
    int64_t atomic_var = 42;
    int64_t expected = 42;
    int64_t desired = 100;
    result += atomic_multi_operand_test(&atomic_var, &expected, desired,
                                       __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Architecture-specific tests */
#ifdef __x86_64__
    /* Initialize AVX-512 vectors */
    __m512i avx_vecs[10];
    for (int i = 0; i < 10; i++) {
        avx_vecs[i] = _mm512_set1_epi32(i);
    }
    
    /* Call AVX-512 multi-operand function */
    __m512i avx_result = avx512_multi_operand_test(
        avx_vecs[0], avx_vecs[1], avx_vecs[2], avx_vecs[3], avx_vecs[4],
        avx_vecs[5], avx_vecs[6], avx_vecs[7], avx_vecs[8], avx_vecs[9]);
    
    /* Extract some value from result */
    int avx_extract = _mm512_extract_epi32(avx_result, 0);
    result += avx_extract;
#endif

#ifdef __ARM_ARCH
#if __ARM_ARCH >= 8
    /* ARM SVE would require runtime length detection - simplified */
    result += 123; /* Placeholder */
#endif
#endif

#ifdef __PPC64__
    /* PowerPC VSX test */
    vector signed int vsx_vecs[10];
    for (int i = 0; i < 10; i++) {
        vsx_vecs[i] = (vector signed int){i, i, i, i};
    }
    
    /* Call would go here - simplified for portability */
    result += 456; /* Placeholder */
#endif

    printf("Result: %lu\n", (unsigned long)result);
    return (int)(result % 256);
}
