/* test_multi_operand_rtl.c
 * 
 * This program generates RTL patterns requiring 10-11 operands
 * to trigger uncovered lines in optabs.cc (lines 8254-8263)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* Prevent inlining to ensure RTL expansion happens */
#define NOINLINE __attribute__((noinline, noipa))

/* Test data initialization */
static void init_test_data(double* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = (double)(i * 1.5 + 0.7);
    }
}

static void init_test_data_int(int64_t* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = i * 3 + 1;
    }
}

/* ============================================================
 * PATTERN A: 10 OPERANDS (Case 10 in optabs.cc)
 * ============================================================ */

#ifdef __AVX512F__

/* 10-operand pattern: AVX-512 masked gather with multiple parameters */
NOINLINE static void test_10_operand_avx512(double* result, const double* base, 
                                           const int64_t* indices, 
                                           __mmask8 mask, double src) {
    /* This should generate RTL with 10 operands:
     * 1. Destination vector (result)
     * 2. Mask
     * 3. Base pointer
     * 4-7. Index vector elements (4x64-bit indices)
     * 8. Scale (encoded as immediate)
     * 9. Source value
     * 10. Displacement (0)
     */
    
    __m512d src_vec = _mm512_set1_pd(src);
    __m256i idx_vec = _mm256_loadu_si256((const __m256i*)indices);
    
    /* _mm512_mask_i64gather_pd requires:
     * 1. src (__m512d)
     * 2. mask (__mmask8)
     * 3. vindex (__m256i)
     * 4. base (void const*)
     * 5. scale (int)
     * Total: 5 explicit operands, but RTL expansion may break this into more
     */
    __m512d gathered = _mm512_mask_i64gather_pd(src_vec, mask, idx_vec, base, 8);
    _mm512_storeu_pd(result, gathered);
}

/* Alternative 10-operand pattern using inline assembly */
NOINLINE static void test_10_operand_asm(int64_t* out, int64_t a, int64_t b, 
                                        int64_t c, int64_t d, int64_t e,
                                        int64_t f, int64_t g, int64_t h) {
    /* Inline assembly with 10 operands */
    asm volatile (
        "/* 10-operand dummy operation */\n\t"
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "mov %[b], %[out0]\n\t"
        "mov %[d], %[out1]\n\t"
        "mov %[f], %[out2]\n\t"
        "mov %[h], %[out3]"
        : [out0] "=r" (out[0]), [out1] "=r" (out[1]), 
          [out2] "=r" (out[2]), [out3] "=r" (out[3])
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h)
        : "cc"
    );
}

#endif /* __AVX512F__ */

/* ============================================================
 * PATTERN B: 11 OPERANDS (Case 11 in optabs.cc)
 * ============================================================ */

#ifdef __AVX512F__

/* 11-operand pattern: Complex AVX-512 masked operation with multiple sources */
NOINLINE static void test_11_operand_avx512(double* result1, double* result2,
                                           const double* a, const double* b,
                                           const double* c, __mmask8 mask,
                                           double alpha, double beta) {
    /* Aiming for 11 operands in RTL expansion */
    __m512d vec_a = _mm512_loadu_pd(a);
    __m512d vec_b = _mm512_loadu_pd(b);
    __m512d vec_c = _mm512_loadu_pd(c);
    
    /* Fused multiply-add with mask: 
     * result = mask ? (a * b + c * alpha + beta) : 0
     * This complex operation might expand to many RTL operands
     */
    __m512d mul_ab = _mm512_mul_pd(vec_a, vec_b);
    __m512d mul_ca = _mm512_mul_pd(vec_c, _mm512_set1_pd(alpha));
    __m512d sum = _mm512_add_pd(mul_ab, mul_ca);
    __m512d with_beta = _mm512_add_pd(sum, _mm512_set1_pd(beta));
    __m512d result = _mm512_maskz_mov_pd(mask, with_beta);
    
    _mm512_storeu_pd(result1, result);
    
    /* Second result for additional operand complexity */
    __m512d neg_result = _mm512_sub_pd(_mm512_setzero_pd(), result);
    _mm512_storeu_pd(result2, neg_result);
}

/* 11-operand inline assembly */
NOINLINE static void test_11_operand_asm(int64_t* out, int64_t a, int64_t b,
                                        int64_t c, int64_t d, int64_t e,
                                        int64_t f, int64_t g, int64_t h,
                                        int64_t i) {
    /* 11 operands: 2 outputs + 9 inputs */
    int64_t tmp1, tmp2, tmp3;
    
    asm volatile (
        "/* 11-operand complex sequence */\n\t"
        "imul %[a], %[b]\n\t"
        "add %[c], %[b]\n\t"
        "imul %[d], %[e]\n\t"
        "add %[f], %[e]\n\t"
        "imul %[g], %[h]\n\t"
        "add %[i], %[h]\n\t"
        "mov %[b], %[tmp1]\n\t"
        "mov %[e], %[tmp2]\n\t"
        "mov %[h], %[tmp3]\n\t"
        "add %[tmp1], %[tmp2]\n\t"
        "add %[tmp2], %[tmp3]\n\t"
        "mov %[tmp3], %[out0]"
        : [out0] "=r" (out[0]), [tmp1] "=&r" (tmp1), 
          [tmp2] "=&r" (tmp2), [tmp3] "=&r" (tmp3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i)
        : "cc"
    );
}

#endif /* __AVX512F__ */

/* ============================================================
 * ARM SVE PATTERNS (if supported)
 * ============================================================ */

#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* 10-operand SVE gather pattern */
NOINLINE static void test_10_operand_sve(double* result, const double* base,
                                        const int64_t* offsets, svbool_t pg) {
    /* SVE gather with predicate, base, offset vector - may expand to many operands */
    svint64_t offset_vec = svld1_s64(pg, offsets);
    svfloat64_t gathered = svld1_gather_index(pg, base, offset_vec);
    svst1(pg, result, gathered);
}

/* 11-operand SVE complex operation */
NOINLINE static void test_11_operand_sve(double* result1, double* result2,
                                        const double* a, const double* b,
                                        svbool_t pg, double alpha) {
    svfloat64_t vec_a = svld1(pg, a);
    svfloat64_t vec_b = svld1(pg, b);
    
    /* Complex operation: result1 = a * b + alpha, result2 = a - b * alpha */
    svfloat64_t mul = svmul_m(pg, vec_a, vec_b);
    svfloat64_t result1_vec = svadd_m(pg, mul, svdup_f64(alpha));
    svfloat64_t scaled_b = svmul_m(pg, vec_b, svdup_f64(alpha));
    svfloat64_t result2_vec = svsub_m(pg, vec_a, scaled_b);
    
    svst1(pg, result1, result1_vec);
    svst1(pg, result2, result2_vec);
}

#endif /* __ARM_FEATURE_SVE */

/* ============================================================
 * POWERPC ALTIVEC/VSX PATTERNS
 * ============================================================ */

#ifdef __ALTIVEC__

#include <altivec.h>

/* 10-operand PowerPC pattern */
NOINLINE static void test_10_operand_powerpc(float* result, const float* a,
                                            const float* b, const float* c,
                                            const float* d) {
    /* Complex vector operation with multiple inputs */
    vector float vec_a = vec_load(a);
    vector float vec_b = vec_load(b);
    vector float vec_c = vec_load(c);
    vector float vec_d = vec_load(d);
    
    /* Multiple operations that might combine into one RTL pattern */
    vector float mul1 = vec_mul(vec_a, vec_b);
    vector float mul2 = vec_mul(vec_c, vec_d);
    vector float add1 = vec_add(mul1, mul2);
    vector float result_vec = vec_madd(vec_a, vec_b, add1); /* FMA */
    
    vec_store(result_vec, result);
}

#endif /* __ALTIVEC__ */

/* ============================================================
 * RISC-V VECTOR EXTENSION PATTERNS
 * ============================================================ */

#ifdef __riscv_v

/* 10-operand RISC-V vector pattern */
NOINLINE static void test_10_operand_riscv(double* result, const double* a,
                                          const double* b, long vl) {
    /* Using vector length and mask - may expand to many operands */
    asm volatile (
        "vsetvli zero, %[vl], e64, m8, ta, ma\n\t"
        "vle64.v v0, (%[a])\n\t"
        "vle64.v v8, (%[b])\n\t"
        "vfadd.vv v16, v0, v8\n\t"
        "vse64.v v16, (%[result])"
        : 
        : [result] "r" (result), [a] "r" (a), [b] "r" (b), [vl] "r" (vl)
        : "v0", "v8", "v16", "memory"
    );
}

#endif /* __riscv_v */

/* ============================================================
 * TEST EXECUTION AND VALIDATION
 * ============================================================ */

int main() {
    int passed_tests = 0;
    int total_tests = 0;
    
    printf("Testing multi-operand RTL expansion patterns...\n");
    
    /* Test data */
    double data[64];
    int64_t indices[8];
    double result1[8], result2[8];
    int64_t int_results[4];
    
    init_test_data(data, 64);
    init_test_data_int(indices, 8);
    
    /* ===== AVX-512 TESTS ===== */
#ifdef __AVX512F__
    printf("\n[AVX-512 Tests]\n");
    total_tests += 4;
    
    /* Test 10-operand pattern */
    __mmask8 mask = 0xFF;
    test_10_operand_avx512(result1, data, indices, mask, 2.5);
    
    /* Simple validation */
    double sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result1[i];
    }
    if (sum != 0) {  /* Non-zero indicates something was computed */
        printf("  ✓ 10-operand AVX-512 pattern executed\n");
        passed_tests++;
    }
    
    /* Test 10-operand inline assembly */
    test_10_operand_asm(int_results, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    if (int_results[0] != 0) {
        printf("  ✓ 10-operand inline assembly executed\n");
        passed_tests++;
    }
    
    /* Test 11-operand pattern */
    test_11_operand_avx512(result1, result2, data, data + 8, data + 16, mask, 1.5, 0.5);
    
    sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result1[i] + result2[i];
    }
    if (sum != 0) {
        printf("  ✓ 11-operand AVX-512 pattern executed\n");
        passed_tests++;
    }
    
    /* Test 11-operand inline assembly */
    test_11_operand_asm(int_results, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    if (int_results[0] != 0) {
        printf("  ✓ 11-operand inline assembly executed\n");
        passed_tests++;
    }
#endif
    
    /* ===== ARM SVE TESTS ===== */
#ifdef __ARM_FEATURE_SVE
    printf("\n[ARM SVE Tests]\n");
    total_tests += 2;
    
    svbool_t pg = svptrue_b64();
    test_10_operand_sve(result1, data, indices, pg);
    
    sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result1[i];
    }
    if (sum != 0) {
        printf("  ✓ 10-operand SVE pattern executed\n");
        passed_tests++;
    }
    
    test_11_operand_sve(result1, result2, data, data + 8, pg, 2.0);
    
    sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result1[i] + result2[i];
    }
    if (sum != 0) {
        printf("  ✓ 11-operand SVE pattern executed\n");
        passed_tests++;
    }
#endif
    
    /* ===== POWERPC TESTS ===== */
#ifdef __ALTIVEC__
    printf("\n[PowerPC Altivec/VSX Tests]\n");
    total_tests += 1;
    
    float float_data[16];
    float float_result[4];
    for (int i = 0; i < 16; i++) float_data[i] = i * 1.1f;
    
    test_10_operand_powerpc(float_result, float_data, float_data + 4, 
                           float_data + 8, float_data + 12);
    
    float float_sum = 0;
    for (int i = 0; i < 4; i++) float_sum += float_result[i];
    if (float_sum != 0) {
        printf("  ✓ 10-operand PowerPC pattern executed\n");
        passed_tests++;
    }
#endif
    
    /* ===== RISC-V TESTS ===== */
#ifdef __riscv_v
    printf("\n[RISC-V Vector Tests]\n");
    total_tests += 1;
    
    test_10_operand_riscv(result1, data, data + 8, 8);
    
    sum = 0;
    for (int i = 0; i < 8; i++) sum += result1[i];
    if (sum != 0) {
        printf("  ✓ 10-operand RISC-V pattern executed\n");
        passed_tests++;
    }
#endif
    
    /* Summary */
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total architecture-specific tests: %d\n", total_tests);
    printf("  Passed: %d\n", passed_tests);
    
    if (total_tests == 0) {
        printf("\nNo architecture-specific tests were compiled.\n");
        printf("Try compiling with appropriate flags:\n");
        printf("  x86 AVX-512: -mavx512f -mavx512vl\n");
        printf("  ARM SVE: -march=armv8-a+sve\n");
        printf("  PowerPC: -maltivec -mvsx\n");
        printf("  RISC-V: -march=rv64gcv\n");
    }
    
    return (passed_tests == total_tests && total_tests > 0) ? 0 : 1;
}
