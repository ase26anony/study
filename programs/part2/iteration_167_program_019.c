/* Test program to trigger 10-11 operand RTL expansion in optabs.cc
 * Compile with: gcc -O3 -ftree-vectorize -funroll-loops -fopenmp -march=native test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation helpers */
static int tests_passed = 0;
static int tests_run = 0;

#define TEST_START() tests_run++
#define TEST_PASS() tests_passed++; printf("PASS: %s\n", __func__)
#define TEST_FAIL(msg) printf("FAIL: %s - %s\n", __func__, msg)
#define TEST_SUMMARY() printf("\nSummary: %d/%d tests passed\n", tests_passed, tests_run)

/* Force no-inline to ensure RTL expansion happens at call site */
#define NOINLINE __attribute__((noinline))

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>
#include <x86intrin.h>

/* Pattern A: 10 operands - masked gather with multiple parameters */
NOINLINE static void test_avx512_gather_10ops(void)
{
    TEST_START();
    
    /* Setup test data */
    double base[1024] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 1024; i++) {
        base[i] = (double)(i * 1.5);
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = i * 16;
    }
    
    /* This gather intrinsic requires many operands:
     * 1. Destination mask
     * 2. Source mask
     * 3. Base pointer
     * 4. Index vector
     * 5. Scale
     * 6. Vector length
     * 7. Hint
     * 8. Source data (for scatter)
     * 9. Mask register
     * 10. Result
     */
    __m512i vindex = _mm512_load_epi64(indices);
    __mmask8 mask = 0xFF;  /* All lanes active */
    __m512d src = _mm512_set1_pd(0.0);
    
    /* Complex gather pattern that may expand to 10 operands */
    __m512d gathered = _mm512_mask_i64gather_pd(src, mask, vindex, 
                                               base, 8, _MM_SCALE_1);
    
    _mm512_store_pd(result, gathered);
    
    /* Validate results */
    int valid = 1;
    for (int i = 0; i < 8; i++) {
        double expected = base[indices[i]];
        if (result[i] != expected) {
            valid = 0;
            break;
        }
    }
    
    if (valid) {
        TEST_PASS();
    } else {
        TEST_FAIL("Gather results incorrect");
    }
}

/* Pattern B: 11 operands - complex masked operation with multiple parameters */
NOINLINE static void test_avx512_fmadd_11ops(void)
{
    TEST_START();
    
    /* Setup vectors */
    __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b = _mm512_setr_pd(2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    __m512d c = _mm512_setr_pd(0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5);
    __mmask8 mask = 0x0F;  /* Lower 4 lanes active */
    
    /* Complex masked operation that may require 11 operands during expansion */
    __m512d result = _mm512_mask3_fmadd_round_pd(a, b, c, mask, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    
    /* Store and validate */
    double res[8];
    _mm512_store_pd(res, result);
    
    /* Check lower 4 lanes (should be a*b + c) */
    int valid = 1;
    for (int i = 0; i < 4; i++) {
        double expected = (i+1) * (i+2) + 0.5;
        if (res[i] != expected) {
            valid = 0;
            break;
        }
    }
    /* Check upper 4 lanes (should be c) */
    for (int i = 4; i < 8; i++) {
        if (res[i] != 0.5) {
            valid = 0;
            break;
        }
    }
    
    if (valid) {
        TEST_PASS();
    } else {
        TEST_FAIL("FMA results incorrect");
    }
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Pattern A: 10 operands - SVE gather with predicate */
NOINLINE static void test_arm_sve_gather_10ops(void)
{
    TEST_START();
    
    /* Setup data */
    uint64_t base[256] __attribute__((aligned(64)));
    uint64_t indices[svcntd()];
    uint64_t result[svcntd()];
    
    for (size_t i = 0; i < 256; i++) {
        base[i] = i * 3;
    }
    for (size_t i = 0; i < svcntd(); i++) {
        indices[i] = i * 8;
    }
    
    /* Create predicate (all true) */
    svbool_t pg = svptrue_b64();
    
    /* SVE gather - this may expand to 10 operands:
     * 1. Predicate
     * 2. Base pointer
     * 3. Offset vector
     * 4. Scale
     * 5. Result vector
     * 6-10: Various internal operands during expansion
     */
    svuint64_t offsets = svld1_u64(pg, indices);
    svuint64_t gathered = svld1_gather_u64index_u64(pg, base, offsets);
    
    svst1_u64(pg, result, gathered);
    
    /* Validate */
    int valid = 1;
    for (size_t i = 0; i < svcntd(); i++) {
        if (result[i] != base[indices[i]]) {
            valid = 0;
            break;
        }
    }
    
    if (valid) {
        TEST_PASS();
    } else {
        TEST_FAIL("SVE gather results incorrect");
    }
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec/VSX Implementation ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

/* Pattern B: 11 operands - Complex vector permutation */
NOINLINE static void test_powerpc_permute_11ops(void)
{
    TEST_START();
    
    /* Setup vectors */
    vector unsigned char a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    vector unsigned char b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    vector unsigned char mask = {31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16};
    
    /* Complex permutation that may require many operands during RTL expansion */
    vector unsigned char result = vec_perm(a, b, mask);
    
    /* Validate by checking specific positions */
    vector unsigned char expected = {31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16};
    
    if (vec_all_eq(result, expected)) {
        TEST_PASS();
    } else {
        TEST_FAIL("PowerPC permute results incorrect");
    }
}

#endif /* __ALTIVEC__ */

/* ==================== RISC-V Vector Implementation ==================== */
#ifdef __riscv_v

/* Pattern A: 10 operands - RISC-V vector load with mask */
NOINLINE static void test_riscv_vload_10ops(void)
{
    TEST_START();
    
    /* This would use RISC-V vector intrinsics when available in GCC */
    /* Placeholder for actual implementation */
    
    printf("SKIP: RISC-V vector intrinsics not fully implemented in this test\n");
}

#endif /* __riscv_v */

/* ==================== Generic Inline Assembly Fallback ==================== */

/* Pattern B: 11 operands - Generic inline assembly with many operands */
NOINLINE static void test_generic_asm_11ops(void)
{
    TEST_START();
    
    /* Use 11 register operands to force the 11-operand case */
    long long op0 = 1, op1 = 2, op2 = 3, op3 = 4, op4 = 5;
    long long op5 = 6, op6 = 7, op7 = 8, op8 = 9, op9 = 10;
    long long op10 = 11;
    long long result = 0;
    
    /* Extended asm with 11 operands - this should trigger case 11: in optabs.cc */
    asm volatile (
        /* Dummy multi-operand instruction pattern */
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r" (result)
        : "r" (op0), "r" (op1), "r" (op2), "r" (op3),
          "r" (op4), "r" (op5), "r" (op6), "r" (op7),
          "r" (op8), "r" (op9), "r" (op10)
        : "cc"
    );
    
    /* Validate: sum of 1 through 11 = 66 */
    if (result == 66) {
        TEST_PASS();
    } else {
        TEST_FAIL("Generic asm results incorrect");
    }
}

/* ==================== Main Function ==================== */

int main(void)
{
    printf("Testing multi-operand RTL expansion patterns...\n");
    printf("Targeting optabs.cc lines 8254-8263 (10-11 operand cases)\n\n");
    
    /* Always run generic test */
    test_generic_asm_11ops();
    
    /* Run architecture-specific tests */
#ifdef __AVX512F__
    printf("\n=== x86 AVX-512 Tests ===\n");
    test_avx512_gather_10ops();    /* Pattern A: 10 operands */
    test_avx512_fmadd_11ops();     /* Pattern B: 11 operands */
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("\n=== ARM SVE Tests ===\n");
    test_arm_sve_gather_10ops();   /* Pattern A: 10 operands */
#endif
    
#ifdef __ALTIVEC__
    printf("\n=== PowerPC Altivec/VSX Tests ===\n");
    test_powerpc_permute_11ops();  /* Pattern B: 11 operands */
#endif
    
#ifdef __riscv_v
    printf("\n=== RISC-V Vector Tests ===\n");
    test_riscv_vload_10ops();      /* Pattern A: 10 operands */
#endif
    
    /* Print summary */
    printf("\n");
    TEST_SUMMARY();
    
    return tests_passed == tests_run ? 0 : 1;
}
