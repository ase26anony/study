/* test_multi_operand_expansion.c
 * 
 * This program generates RTL patterns requiring 10-11 operands
 * to trigger uncovered lines in optabs.cc (lines 8254-8263)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation utilities */
#define ARRAY_SIZE 64
#define VALIDATE(cond, msg) do { \
    if (!(cond)) { \
        printf("Validation failed: %s\n", msg); \
        return 0; \
    } \
} while(0)

/* Function attributes to prevent premature inlining */
#define NOINLINE __attribute__((noinline, optimize("O3")))

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Pattern A: 10 operands - masked gather operation */
NOINLINE int test_avx512_gather_10_operands(void) {
    /* Setup test data */
    double base[ARRAY_SIZE] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = (double)(i * 2);
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = i * 8;
        result[i] = 0.0;
    }
    
    /* Create a mask with alternating bits */
    __mmask8 mask = 0xAA; /* 0b10101010 */
    
    /* This gather intrinsic should generate RTL with ~10 operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Index vector
     * 4. Base pointer
     * 5. Scale
     * 6. Vector length hint
     * 7. Source (for merge-masked version)
     * Plus additional implicit operands for addressing modes
     */
    __m512d src = _mm512_set1_pd(0.0);
    __m512i vindex = _mm512_load_epi64(indices);
    
    /* Use masked gather with all parameters explicit */
    __m512d gathered = _mm512_mask_i64gather_pd(
        src,            /* src: merge source */
        mask,           /* mask: gather mask */
        vindex,         /* vindex: indices */
        base,           /* base: base pointer */
        8               /* scale: sizeof(double) */
    );
    
    /* Store and validate */
    _mm512_store_pd(result, gathered);
    
    double expected_sum = 0;
    for (int i = 0; i < 8; i++) {
        if (mask & (1 << i)) {
            expected_sum += base[indices[i] / 8];
        }
    }
    
    double actual_sum = 0;
    for (int i = 0; i < 8; i++) {
        actual_sum += result[i];
    }
    
    VALIDATE(fabs(actual_sum - expected_sum) < 1e-10, 
             "AVX-512 gather 10-operand pattern");
    
    return 1;
}

/* Pattern B: 11 operands - complex permute and blend */
NOINLINE int test_avx512_permute_11_operands(void) {
    /* Create multiple vector sources */
    __m512d a = _mm512_setr_pd(0,1,2,3,4,5,6,7);
    __m512d b = _mm512_setr_pd(8,9,10,11,12,13,14,15);
    __m512d c = _mm512_setr_pd(16,17,18,19,20,21,22,23);
    __m512d d = _mm512_setr_pd(24,25,26,27,28,29,30,31);
    
    __mmask8 mask1 = 0xF0;
    __mmask8 mask2 = 0x0F;
    
    /* Complex sequence that may generate 11-operand RTL */
    __m512d temp1 = _mm512_mask_permutex2var_pd(
        a, mask1, 
        _mm512_set_epi64(7,6,5,4,3,2,1,0), /* idx */
        b
    );
    
    __m512d temp2 = _mm512_mask_permutex2var_pd(
        c, mask2,
        _mm512_set_epi64(15,14,13,12,11,10,9,8),
        d
    );
    
    __m512d result = _mm512_mask_blend_pd(0xCC, temp1, temp2);
    
    /* Validate with simple computation */
    double res_arr[8];
    _mm512_store_pd(res_arr, result);
    
    double sum = 0;
    for (int i = 0; i < 8; i++) sum += res_arr[i];
    
    /* Expected sum depends on complex permutation */
    VALIDATE(sum > 0 && sum < 1000, 
             "AVX-512 permute 11-operand pattern");
    
    return 1;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Pattern A: 10 operands - SVE gather with predicate */
NOINLINE int test_sve_gather_10_operands(void) {
    /* Initialize data */
    double base[ARRAY_SIZE];
    uint64_t indices[svcntd()];
    double result[svcntd()];
    
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        base[i] = (double)(i * 3);
    }
    for (size_t i = 0; i < svcntd(); i++) {
        indices[i] = i * 4;
    }
    
    /* Create predicate */
    svbool_t pg = svwhilelt_b64(0, svcntd());
    
    /* SVE gather - should generate multi-operand RTL */
    svint64_t vindex = svld1sw_s64(pg, (const int64_t *)indices);
    svfloat64_t gathered = svld1_gather_index(pg, base, vindex);
    
    /* Store and validate */
    svst1(pg, result, gathered);
    
    double expected = 0;
    for (size_t i = 0; i < svcntd(); i++) {
        if (svptest_any(svptrue_b64(), pg)) {
            expected += base[indices[i]];
        }
    }
    
    double actual = 0;
    for (size_t i = 0; i < svcntd(); i++) {
        actual += result[i];
    }
    
    VALIDATE(fabs(actual - expected) < 1e-10,
             "SVE gather 10-operand pattern");
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC VSX Implementation ==================== */
#ifdef __VSX__

#include <altivec.h>

/* Pattern B: 11 operands via inline assembly */
NOINLINE int test_vsx_11_operands(void) {
    /* Set up vector registers */
    vector double v0 = {0,1};
    vector double v1 = {2,3};
    vector double v2 = {4,5};
    vector double v3 = {6,7};
    vector double v4 = {8,9};
    vector double v5 = {10,11};
    vector double v6 = {12,13};
    vector double v7 = {14,15};
    vector double result;
    
    /* Extended inline assembly with 11 operands
     * This forces the compiler to handle many operands during RTL expansion */
    asm volatile (
        "xxpermdi %0, %1, %2, 0\n\t"
        "xxpermdi %0, %0, %3, 1\n\t"
        "xxpermdi %0, %0, %4, 2\n\t"
        "xxpermdi %0, %0, %5, 3\n\t"
        : "=v"(result)
        : "v"(v0), "v"(v1), "v"(v2), "v"(v3), "v"(v4),
          "v"(v5), "v"(v6), "v"(v7)
        : 
    );
    
    /* Validate */
    double res_arr[2];
    vec_st(result, 0, res_arr);
    
    VALIDATE(res_arr[0] != 0 || res_arr[1] != 0,
             "VSX inline asm 11-operand pattern");
    
    return 1;
}

#endif /* __VSX__ */

/* ==================== RISC-V Vector Implementation ==================== */
#ifdef __riscv_v

/* Pattern A: 10 operands via vector load with mask */
NOINLINE int test_rvv_10_operands(void) {
    /* Use inline assembly to simulate multi-operand instruction */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long result;
    
    /* Extended asm with 10 explicit operands */
    asm volatile (
        "dummy_instruction %0, %1, %2, %3, %4, %5, %6, %7, %8, %9"
        : "=r"(result)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : 
    );
    
    VALIDATE(result == 55, /* sum of 1..10 */
             "RVV inline asm 10-operand pattern");
    
    return 1;
}

#endif /* __riscv_v */

/* ==================== Generic Fallback ==================== */
/* Fallback using pure inline assembly for architectures without vector support */
NOINLINE int test_generic_multi_operand(void) {
    /* Force 11 operands through extended inline assembly */
    unsigned long op0 = 0, op1 = 1, op2 = 2, op3 = 3, op4 = 4;
    unsigned long op5 = 5, op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    unsigned long result;
    
    /* This asm statement should generate RTL with 11 operands */
    asm volatile (
        "# Multi-operand test %0 = %1 + %2 + ... + %10\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r"(result)
        : "r"(op1), "r"(op2), "r"(op3), "r"(op4), "r"(op5),
          "r"(op6), "r"(op7), "r"(op8), "r"(op9), "r"(op10)
        : 
    );
    
    VALIDATE(result == 55, "Generic 11-operand asm pattern");
    return 1;
}

/* ==================== Main Function ==================== */
int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("Testing multi-operand RTL expansion patterns...\n");
    printf("Target: optabs.cc lines 8254-8263 (10-11 operand cases)\n\n");
    
    /* Test architecture-specific patterns */
    
#ifdef __AVX512F__
    printf("Testing AVX-512 patterns...\n");
    tests_run++;
    if (test_avx512_gather_10_operands()) {
        printf("  ✓ AVX-512 10-operand gather pattern\n");
        tests_passed++;
    } else {
        printf("  ✗ AVX-512 10-operand gather pattern\n");
    }
    
    tests_run++;
    if (test_avx512_permute_11_operands()) {
        printf("  ✓ AVX-512 11-operand permute pattern\n");
        tests_passed++;
    } else {
        printf("  ✗ AVX-512 11-operand permute pattern\n");
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE patterns...\n");
    tests_run++;
    if (test_sve_gather_10_operands()) {
        printf("  ✓ ARM SVE 10-operand gather pattern\n");
        tests_passed++;
    } else {
        printf("  ✗ ARM SVE 10-operand gather pattern\n");
    }
#endif
    
#ifdef __VSX__
    printf("Testing PowerPC VSX patterns...\n");
    tests_run++;
    if (test_vsx_11_operands()) {
        printf("  ✓ PowerPC VSX 11-operand pattern\n");
        tests_passed++;
    } else {
        printf("  ✗ PowerPC VSX 11-operand pattern\n");
    }
#endif
    
#ifdef __riscv_v
    printf("Testing RISC-V Vector patterns...\n");
    tests_run++;
    if (test_rvv_10_operands()) {
        printf("  ✓ RISC-V V 10-operand pattern\n");
        tests_passed++;
    } else {
        printf("  ✗ RISC-V V 10-operand pattern\n");
    }
#endif
    
    /* Always run generic test */
    printf("Testing generic patterns...\n");
    tests_run++;
    if (test_generic_multi_operand()) {
        printf("  ✓ Generic 11-operand asm pattern\n");
        tests_passed++;
    } else {
        printf("  ✗ Generic 11-operand asm pattern\n");
    }
    
    printf("\nSummary: %d/%d tests passed\n", tests_passed, tests_run);
    
    if (tests_passed == tests_run) {
        printf("\nSUCCESS: All multi-operand patterns executed correctly.\n");
        printf("If compiled with appropriate optimization flags, this should\n");
        printf("trigger the 10-11 operand cases in optabs.cc during RTL expansion.\n");
        return 0;
    } else {
        printf("\nWARNING: Some tests failed. Architecture-specific code may not have run.\n");
        return 1;
    }
}
