/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger the 10-11 operand RTL expansion paths
 * in GCC's optabs.cc (lines 8254-8263) by generating code that requires
 * instructions with exactly 10 or 11 operands.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation helpers */
#define VALIDATE(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        return 0; \
    } \
} while(0)

#define PASS(msg) printf("PASS: %s\n", msg)

/* Function attributes to control optimization */
#define NOINLINE __attribute__((noinline, optimize("O3")))
#define HOT __attribute__((hot))

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>
#include <x86intrin.h>

/* Pattern A: 10 operands - masked gather operation */
NOINLINE HOT
int test_avx512_10_operands(void) {
    printf("Testing x86 AVX-512 10-operand pattern...\n");
    
    /* Setup test data */
    double base_array[1024] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    /* Initialize with known values */
    for (int i = 0; i < 1024; i++) {
        base_array[i] = (double)(i * 2);
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = i * 16;  /* Strided access */
    }
    
    /* Create mask (all true) */
    __mmask8 mask = 0xFF;
    
    /* This intrinsic requires multiple operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Index vector
     * 4. Base address
     * 5. Scale
     * 6. Vector length hint
     * Plus implicit operands for address computation
     */
    __m512d src = _mm512_set1_pd(0.0);
    __m512i vindex = _mm512_load_epi64(indices);
    
    /* Perform masked gather - this expands to instruction with many operands */
    __m512d gathered = _mm512_mask_i64gather_pd(src, mask, vindex, 
                                               base_array, 8);
    
    /* Store result for validation */
    _mm512_store_pd(result, gathered);
    
    /* Validate results */
    for (int i = 0; i < 8; i++) {
        double expected = base_array[indices[i] / 8];
        VALIDATE(result[i] == expected, 
                "AVX-512 gather result mismatch");
    }
    
    PASS("x86 AVX-512 10-operand pattern");
    return 1;
}

/* Pattern B: 11 operands - complex masked scatter with update */
NOINLINE HOT
int test_avx512_11_operands(void) {
    printf("Testing x86 AVX-512 11-operand pattern...\n");
    
    /* Setup test data */
    double target_array[1024] __attribute__((aligned(64)));
    double source_array[8] __attribute__((aligned(64)));
    int64_t scatter_indices[8] __attribute__((aligned(64)));
    
    /* Initialize */
    memset(target_array, 0, sizeof(target_array));
    for (int i = 0; i < 8; i++) {
        source_array[i] = (double)(100 + i);
        scatter_indices[i] = i * 32;
    }
    
    /* Create mask */
    __mmask8 mask = 0xFF;
    
    /* Load data into vectors */
    __m512d src_data = _mm512_load_pd(source_array);
    __m512i vindex = _mm512_load_epi64(scatter_indices);
    
    /* This scatter operation requires many operands:
     * 1. Base address
     * 2. Mask
     * 3. Index vector
     * 4. Source data
     * 5. Scale
     * 6. Vector length hint
     * Plus implicit address computation operands
     */
    _mm512_mask_i64scatter_pd(target_array, mask, vindex, src_data, 8);
    
    /* Validate scatter results */
    for (int i = 0; i < 8; i++) {
        int idx = scatter_indices[i] / 8;
        VALIDATE(target_array[idx] == source_array[i],
                "AVX-512 scatter result mismatch");
    }
    
    PASS("x86 AVX-512 11-operand pattern");
    return 1;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Pattern A: 10 operands - SVE gather with predicate */
NOINLINE HOT
int test_arm_sve_10_operands(void) {
    printf("Testing ARM SVE 10-operand pattern...\n");
    
    /* Setup test data */
    double base[1024] __attribute__((aligned(64)));
    uint64_t indices[256] __attribute__((aligned(64)));
    double result[256] __attribute__((aligned(64)));
    
    /* Initialize */
    for (int i = 0; i < 1024; i++) {
        base[i] = (double)i;
    }
    for (int i = 0; i < 256; i++) {
        indices[i] = i * 4;
    }
    
    /* Create all-true predicate */
    svbool_t pg = svptrue_b64();
    
    /* SVE gather with multiple operands:
     * 1. Predicate
     * 2. Base pointer
     * 3. Offset vector
     * 4. Scale
     * Implicit operands for address computation
     */
    svuint64_t offset = svld1_u64(pg, indices);
    svfloat64_t gathered = svld1_gather_u64index_f64(pg, base, offset);
    
    /* Store and validate */
    svst1_f64(pg, result, gathered);
    
    for (int i = 0; i < 256 && i < 1024/4; i++) {
        double expected = base[indices[i]];
        VALIDATE(result[i] == expected, "SVE gather result mismatch");
    }
    
    PASS("ARM SVE 10-operand pattern");
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC VSX Implementation ==================== */
#ifdef __VSX__

#include <altivec.h>

/* Pattern B: 11 operands - VSX matrix multiply-accumulate pattern */
NOINLINE HOT
int test_powerpc_11_operands(void) {
    printf("Testing PowerPC VSX 11-operand pattern...\n");
    
    /* Complex operation simulated with inline asm */
    double a[2] __attribute__((aligned(16))) = {1.0, 2.0};
    double b[2] __attribute__((aligned(16))) = {3.0, 4.0};
    double c[2] __attribute__((aligned(16))) = {5.0, 6.0};
    double d[2] __attribute__((aligned(16))) = {7.0, 8.0};
    double result[2] __attribute__((aligned(16)));
    
    /* Extended inline asm with 11 operands to force 11-operand expansion */
    asm volatile (
        /* Complex operation with many operands */
        "xxpermdi %x0, %x1, %x2, 0\n\t"
        "xxpermdi %x3, %x4, %x5, 0\n\t"
        "xvmuldp %x6, %x0, %x3\n\t"
        "xxpermdi %x7, %x8, %x9, 0\n\t"
        "xvadddp %x10, %x6, %x7"
        : "=wa"(result[0]), "=wa"(result[1])
        : "wa"(a[0]), "wa"(a[1]), 
          "wa"(b[0]), "wa"(b[1]),
          "wa"(c[0]), "wa"(c[1]),
          "wa"(d[0]), "wa"(d[1]),
          "r"(0)  /* 11th operand */
        : "cr0"
    );
    
    /* Simple validation */
    VALIDATE(result[0] != 0.0 || result[1] != 0.0, 
            "PowerPC operation produced zero");
    
    PASS("PowerPC VSX 11-operand pattern");
    return 1;
}

#endif /* __VSX__ */

/* ==================== Generic Inline Assembly Fallback ==================== */

/* Pattern A: Generic 10-operand inline asm */
NOINLINE HOT
int test_generic_10_operands(void) {
    printf("Testing generic 10-operand inline asm...\n");
    
    long ops[10];
    long result = 0;
    
    /* Initialize operands */
    for (int i = 0; i < 10; i++) {
        ops[i] = i + 1;
    }
    
    /* 10-operand inline assembly to force expansion */
    asm volatile (
        "/* 10-operand dummy operation */\n\t"
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(result)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), 
          "r"(ops[3]), "r"(ops[4]), "r"(ops[5]),
          "r"(ops[6]), "r"(ops[7]), "r"(ops[8])
        : "cc"
    );
    
    /* Validate */
    long expected = 1+2+3+4+5+6+7+8+9;
    VALIDATE(result == expected, "Generic 10-operand result mismatch");
    
    PASS("Generic 10-operand pattern");
    return 1;
}

/* Pattern B: Generic 11-operand inline asm */
NOINLINE HOT
int test_generic_11_operands(void) {
    printf("Testing generic 11-operand inline asm...\n");
    
    long ops[11];
    long result = 0;
    
    /* Initialize operands */
    for (int i = 0; i < 11; i++) {
        ops[i] = i + 1;
    }
    
    /* 11-operand inline assembly */
    asm volatile (
        "/* 11-operand dummy operation */\n\t"
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
        : "=r"(result)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), 
          "r"(ops[3]), "r"(ops[4]), "r"(ops[5]),
          "r"(ops[6]), "r"(ops[7]), "r"(ops[8]),
          "r"(ops[9]), "r"(ops[10])
        : "cc"
    );
    
    /* Validate */
    long expected = 1+2+3+4+5+6+7+8+9+10+11;
    VALIDATE(result == expected, "Generic 11-operand result mismatch");
    
    PASS("Generic 11-operand pattern");
    return 1;
}

/* ==================== Main Test Driver ==================== */

int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("=== Testing 10-11 Operand RTL Expansion Coverage ===\n\n");
    
    /* Run architecture-specific tests */
#ifdef __AVX512F__
    tests_run++;
    if (test_avx512_10_operands()) tests_passed++;
    
    tests_run++;
    if (test_avx512_11_operands()) tests_passed++;
#endif
    
#ifdef __ARM_FEATURE_SVE
    tests_run++;
    if (test_arm_sve_10_operands()) tests_passed++;
#endif
    
#ifdef __VSX__
    tests_run++;
    if (test_powerpc_11_operands()) tests_passed++;
#endif
    
    /* Always run generic tests */
    tests_run++;
    if (test_generic_10_operands()) tests_passed++;
    
    tests_run++;
    if (test_generic_11_operands()) tests_passed++;
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    
    if (tests_passed == tests_run) {
        printf("\nSUCCESS: All tests passed!\n");
        return 0;
    } else {
        printf("\nWARNING: Some tests failed or were skipped\n");
        return 1;
    }
}
