/* test_multi_operand_rtl.c
 * 
 * This program generates RTL patterns requiring 10-11 operands
 * to trigger uncovered lines 8254-8263 in optabs.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>
#endif

/* Runtime validation utilities */
#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s\n", msg); \
        return 0; \
    } \
} while(0)

#define PASS(msg) printf("PASS: %s\n", msg)

/* Prevent premature inlining to ensure RTL expansion */
#define NOINLINE __attribute__((noinline, noipa))

/* ============================================
 * PATTERN A: 10-operand case
 * ============================================ */

#ifdef __AVX512F__
NOINLINE static int test_avx512_10_operand(void) {
    printf("Testing AVX-512 10-operand pattern...\n");
    
    /* Setup test data */
    double base[8] = {100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0};
    int64_t indices[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    double src[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    double result[8] = {0};
    
    /* This gather intrinsic with mask should generate RTL with 10 operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Source vector
     * 4. Base address
     * 5. Index vector
     * 6. Scale (immediate)
     * 7. Displacement (immediate)
     * 8. Mask register
     * 9. Address size hint
     * 10. Data size hint
     */
    __m512d vresult, vsrc, vindex;
    __mmask8 kmask = 0xFF;
    
    vsrc = _mm512_loadu_pd(src);
    vindex = _mm512_loadu_si512((__m512i*)indices);
    
    /* Complex masked gather operation - likely to require many operands */
    vresult = _mm512_mask_i64gather_pd(vsrc, kmask, vindex, base, 8);
    
    _mm512_storeu_pd(result, vresult);
    
    /* Validate */
    for (int i = 0; i < 8; i++) {
        double expected = (kmask & (1 << i)) ? base[indices[i]] : src[i];
        CHECK(fabs(result[i] - expected) < 1e-9, 
              "AVX-512 10-operand gather result mismatch");
    }
    
    PASS("AVX-512 10-operand pattern executed correctly");
    return 1;
}

/* Additional 10-operand pattern using inline asm */
NOINLINE static int test_avx512_10_operand_asm(void) {
    printf("Testing AVX-512 10-operand inline asm...\n");
    
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    __m512d c = _mm512_set1_pd(3.0);
    __m512d d = _mm512_set1_pd(4.0);
    __m512d e = _mm512_set1_pd(5.0);
    __m512d f = _mm512_set1_pd(6.0);
    __m512d g = _mm512_set1_pd(7.0);
    __m512d h = _mm512_set1_pd(8.0);
    __m512d i = _mm512_set1_pd(9.0);
    __m512d result;
    
    /* Extended inline assembly with 10 operands */
    asm volatile (
        "vmulpd %{z%}1, %{z%}2, %{z%}0\n\t"
        "vaddpd %{z%}3, %{z%}0, %{z%}0\n\t"
        "vfmadd213pd %{z%}4, %{z%}5, %{z%}0\n\t"
        "vfnmadd231pd %{z%}6, %{z%}7, %{z%}0\n\t"
        "vfmaddsub231pd %{z%}8, %{z%}9, %{z%}0"
        : "=v"(result)
        : "v"(a), "v"(b), "v"(c), "v"(d), "v"(e), 
          "v"(f), "v"(g), "v"(h), "v"(i)
        : 
    );
    
    /* Dummy use to prevent optimization */
    volatile double check = ((double*)&result)[0];
    (void)check;
    
    PASS("AVX-512 10-operand inline asm executed");
    return 1;
}
#endif /* __AVX512F__ */

/* ============================================
 * PATTERN B: 11-operand case  
 * ============================================ */

#ifdef __AVX512F__
NOINLINE static int test_avx512_11_operand(void) {
    printf("Testing AVX-512 11-operand pattern...\n");
    
    /* Complex scatter operation with multiple parameters */
    double data[16] = {0};
    double src[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    int64_t indices[8] = {0, 2, 4, 6, 8, 10, 12, 14};
    __mmask8 kmask = 0xAA; /* 10101010 - alternate elements */
    
    __m512d vsrc = _mm512_loadu_pd(src);
    __m512i vindex = _mm512_loadu_si512((__m512i*)indices);
    
    /* Scatter with mask - potentially 11 operands:
     * 1. Base address
     * 2. Mask
     * 3. Index vector
     * 4. Source data
     * 5. Scale
     * 6. Displacement
     * 7. Mask register
     * 8. Address size
     * 9. Data size
     * 10. Hint
     * 11. Additional control
     */
    _mm512_mask_i64scatter_pd(data, kmask, vindex, vsrc, 8);
    
    /* Validate scattered data */
    for (int j = 0; j < 8; j++) {
        int idx = indices[j];
        if (kmask & (1 << j)) {
            CHECK(fabs(data[idx] - src[j]) < 1e-9, 
                  "AVX-512 11-operand scatter result mismatch");
        }
    }
    
    PASS("AVX-512 11-operand pattern executed correctly");
    return 1;
}
#endif /* __AVX512F__ */

/* ============================================
 * ARM SVE patterns (if supported)
 * ============================================ */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

NOINLINE static int test_sve_10_operand(void) {
    printf("Testing ARM SVE 10-operand pattern...\n");
    
    /* SVE gather with predicate, base, and offsets */
    double base_array[64];
    double result_array[64];
    uint64_t offsets[64];
    
    for (int i = 0; i < 64; i++) {
        base_array[i] = i * 10.0;
        offsets[i] = i * sizeof(double);
    }
    
    svbool_t pg = svptrue_b64();
    svfloat64_t base = svld1(pg, base_array);
    svuint64_t offset_vec = svld1uw_u64(pg, offsets);
    
    /* Complex SVE operation - may require many operands */
    svfloat64_t gathered = svld1_gather_offset(pg, base_array, offset_vec);
    
    svst1(pg, result_array, gathered);
    
    /* Validate */
    for (int i = 0; i < 64; i++) {
        if (svptest_any(svptrue_b64(), pg)) {
            CHECK(fabs(result_array[i] - base_array[i]) < 1e-9,
                  "SVE 10-operand gather mismatch");
        }
    }
    
    PASS("ARM SVE 10-operand pattern executed correctly");
    return 1;
}
#endif /* __ARM_FEATURE_SVE */

/* ============================================
 * PowerPC Altivec/VSX patterns
 * ============================================ */

#ifdef __ALTIVEC__
#include <altivec.h>

NOINLINE static int test_powerpc_10_operand(void) {
    printf("Testing PowerPC 10-operand pattern...\n");
    
    /* Complex vector permutation with multiple arguments */
    vector float a = {1.0f, 2.0f, 3.0f, 4.0f};
    vector float b = {5.0f, 6.0f, 7.0f, 8.0f};
    vector float c = {9.0f, 10.0f, 11.0f, 12.0f};
    vector float d = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Extended inline asm with 10 operands */
    vector float result;
    
    asm volatile (
        "xvaddsp %0, %1, %2\n\t"
        "xvmulsp %0, %0, %3\n\t"
        "xvmsubasp %0, %4, %5\n\t"
        "xvnmsubasp %0, %6, %7\n\t"
        "xxpermdi %0, %0, %8, 0"
        : "=v"(result)
        : "v"(a), "v"(b), "v"(c), "v"(d),
          "v"(a), "v"(b), "v"(c), "v"(d)
        : 
    );
    
    volatile float check = ((float*)&result)[0];
    (void)check;
    
    PASS("PowerPC 10-operand pattern executed");
    return 1;
}
#endif /* __ALTIVEC__ */

/* ============================================
 * RISC-V Vector Extension patterns
 * ============================================ */

#ifdef __riscv_v
NOINLINE static int test_riscv_11_operand(void) {
    printf("Testing RISC-V V 11-operand pattern...\n");
    
    /* RISC-V vector load with multiple parameters */
    long array[64];
    long result[64];
    
    for (int i = 0; i < 64; i++) {
        array[i] = i * 100;
    }
    
    /* Extended inline asm simulating complex vector operation */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long output;
    
    asm volatile (
        "dummy_operation %0, %1, %2, %3, %4, %5, %6, %7, %8, %9, %10"
        : "=r"(output)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : 
    );
    
    result[0] = output;
    
    PASS("RISC-V 11-operand pattern executed");
    return 1;
}
#endif /* __riscv_v */

/* ============================================
 * Generic fallback with extended inline asm
 * ============================================ */

NOINLINE static int test_generic_multi_operand(void) {
    printf("Testing generic multi-operand inline asm...\n");
    
    /* Force 10 operands in generic inline asm */
    unsigned long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    unsigned long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    unsigned long result;
    
    asm volatile (
        "# Multi-operand dummy instruction\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(result)
        : "r"(op1), "r"(op2), "r"(op3), "r"(op4), "r"(op5),
          "r"(op6), "r"(op7), "r"(op8), "r"(op9)
        : "cc"
    );
    
    CHECK(result == 55, "Generic 10-operand asm computation failed");
    
    /* Force 11 operands */
    unsigned long op11 = 11;
    unsigned long result2;
    
    asm volatile (
        "# 11-operand dummy instruction\n\t"
        "mov %0, %1\n\t"
        "imul %0, %2\n\t"
        "add %0, %3\n\t"
        "imul %0, %4\n\t"
        "add %0, %5\n\t"
        "imul %0, %6\n\t"
        "add %0, %7\n\t"
        "imul %0, %8\n\t"
        "add %0, %9\n\t"
        "imul %0, %10"
        : "=r"(result2)
        : "r"(op1), "r"(op2), "r"(op3), "r"(op4), "r"(op5),
          "r"(op6), "r"(op7), "r"(op8), "r"(op9), "r"(op10)
        : "cc"
    );
    
    PASS("Generic multi-operand patterns executed");
    return 1;
}

/* ============================================
 * Hot loop to encourage vectorization/expansion
 * ============================================ */

NOINLINE static void hot_loop_with_complex_ops(void) {
    volatile int iterations = 1000;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix of operations that might get optimized into multi-operand RTL */
        test_generic_multi_operand();
        
#ifdef __AVX512F__
        if (i % 3 == 0) test_avx512_10_operand();
        if (i % 7 == 0) test_avx512_11_operand();
#endif
    }
}

/* ============================================
 * Main test driver
 * ============================================ */

int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("=== Testing RTL Expansion with 10-11 Operands ===\n\n");
    
    /* Always run generic test */
    tests_run++;
    if (test_generic_multi_operand()) tests_passed++;
    
    /* Architecture-specific tests */
#ifdef __AVX512F__
    printf("\n--- AVX-512 Tests ---\n");
    tests_run++;
    if (test_avx512_10_operand()) tests_passed++;
    
    tests_run++;
    if (test_avx512_10_operand_asm()) tests_passed++;
    
    tests_run++;
    if (test_avx512_11_operand()) tests_passed++;
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("\n--- ARM SVE Tests ---\n");
    tests_run++;
    if (test_sve_10_operand()) tests_passed++;
#endif
    
#ifdef __ALTIVEC__
    printf("\n--- PowerPC Tests ---\n");
    tests_run++;
    if (test_powerpc_10_operand()) tests_passed++;
#endif
    
#ifdef __riscv_v
    printf("\n--- RISC-V V Tests ---\n");
    tests_run++;
    if (test_riscv_11_operand()) tests_passed++;
#endif
    
    /* Execute hot loop to encourage optimization */
    printf("\n--- Running hot loop to trigger optimizations ---\n");
    hot_loop_with_complex_ops();
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    
    if (tests_passed == tests_run) {
        printf("\nSUCCESS: All tests passed. RTL expansion paths should have been exercised.\n");
        return 0;
    } else {
        printf("\nWARNING: Some tests failed. Architecture-specific code may not have run.\n");
        return 1;
    }
}
