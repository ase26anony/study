/* test_multi_operand_rtl.c
 * 
 * This program generates RTL patterns requiring 10-11 operands
 * to trigger uncovered lines 8254-8263 in optabs.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation helpers */
#define ARRAY_SIZE 64
#define VALIDATE(cond, msg) if (!(cond)) { printf("FAIL: %s\n", msg); return 0; } else { printf("PASS: %s\n", msg); }

/* Force no inlining to ensure RTL expansion happens */
#define NOINLINE __attribute__((noinline))

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

NOINLINE int test_avx512_10_operands(void) {
    /* Pattern A: 10 operands - masked gather with multiple parameters */
    printf("Testing AVX-512 10-operand pattern...\n");
    
    /* Setup test data */
    double base[ARRAY_SIZE] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = (double)i * 1.5;
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = i * 8;
    }
    
    /* Create mask: all lanes enabled */
    __mmask8 mask = 0xFF;
    
    /* This intrinsic typically requires:
     * 1. Destination vector
     * 2. Mask
     * 3. Index vector
     * 4. Base pointer
     * 5. Scale
     * 6. Vector length hint
     * Plus implicit operands during RTL expansion
     */
    __m512d src = _mm512_set1_pd(0.0);
    __m512i vindex = _mm512_load_epi64(indices);
    
    /* Complex gather pattern that may expand to 10 operands */
    __m512d gathered = _mm512_mask_i64gather_pd(src, mask, vindex, 
                                               base, 8);
    
    /* Store result for validation */
    _mm512_store_pd(result, gathered);
    
    /* Validation */
    for (int i = 0; i < 8; i++) {
        double expected = base[indices[i] / 8];
        VALIDATE(result[i] == expected, 
                "AVX-512 gather result validation");
    }
    
    return 1;
}

NOINLINE int test_avx512_11_operands(void) {
    /* Pattern B: 11 operands - complex masked scatter with update */
    printf("Testing AVX-512 11-operand pattern...\n");
    
    /* Setup test data */
    double target[ARRAY_SIZE] __attribute__((aligned(64)));
    double source[8] __attribute__((aligned(64)));
    int64_t scatter_indices[8] __attribute__((aligned(64)));
    __mmask8 mask = 0xFF;
    
    for (int i = 0; i < 8; i++) {
        source[i] = (double)i * 2.0;
        scatter_indices[i] = i * 4;
    }
    memset(target, 0, sizeof(target));
    
    __m512d src_vec = _mm512_load_pd(source);
    __m512i vindices = _mm512_load_epi64(scatter_indices);
    
    /* Scatter operation with multiple parameters */
    _mm512_mask_i64scatter_pd(target, mask, vindices, src_vec, 8);
    
    /* Validation */
    for (int i = 0; i < 8; i++) {
        int idx = scatter_indices[i] / 8;
        VALIDATE(target[idx] == source[i], 
                "AVX-512 scatter result validation");
    }
    
    return 1;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

NOINLINE int test_arm_sve_10_operands(void) {
    /* Pattern A: 10 operands - SVE gather with predicate */
    printf("Testing ARM SVE 10-operand pattern...\n");
    
    uint64_t base[ARRAY_SIZE];
    uint64_t indices[svcntd()];
    uint64_t result[svcntd()];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = i * 3;
    }
    for (size_t i = 0; i < svcntd(); i++) {
        indices[i] = i * 2;
    }
    
    /* Create all-true predicate */
    svbool_t pg = svptrue_b64();
    
    /* SVE gather with multiple operands */
    svuint64_t offset_vec = svld1_u64(pg, indices);
    svuint64_t gathered = svld1_gather_u64offset_u64(pg, base, offset_vec);
    
    /* Store and validate */
    svst1_u64(pg, result, gathered);
    
    for (size_t i = 0; i < svcntd(); i++) {
        VALIDATE(result[i] == base[indices[i]], 
                "ARM SVE gather validation");
    }
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec/VSX Implementation ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

NOINLINE int test_powerpc_11_operands(void) {
    /* Pattern B: 11 operands - complex vector permutation */
    printf("Testing PowerPC 11-operand pattern...\n");
    
    /* Use inline assembly to force 11 operands */
    vector float v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Initialize vectors */
    v0 = (vector float){0.0f, 1.0f, 2.0f, 3.0f};
    v1 = (vector float){4.0f, 5.0f, 6.0f, 7.0f};
    v2 = (vector float){8.0f, 9.0f, 10.0f, 11.0f};
    v3 = (vector float){12.0f, 13.0f, 14.0f, 15.0f};
    v4 = (vector float){16.0f, 17.0f, 18.0f, 19.0f};
    v5 = (vector float){20.0f, 21.0f, 22.0f, 23.0f};
    v6 = (vector float){24.0f, 25.0f, 26.0f, 27.0f};
    v7 = (vector float){28.0f, 29.0f, 30.0f, 31.0f};
    v8 = (vector float){32.0f, 33.0f, 34.0f, 35.0f};
    v9 = (vector float){36.0f, 37.0f, 38.0f, 39.0f};
    
    /* Extended inline assembly with 11 operands */
    asm volatile (
        "xxpermdi %x0, %x1, %x2, 0\n\t"
        "xxpermdi %x0, %x0, %x3, 1\n\t"
        "xxpermdi %x0, %x0, %x4, 2\n\t"
        "xxpermdi %x0, %x0, %x5, 3\n\t"
        : "=wa"(v10)
        : "wa"(v0), "wa"(v1), "wa"(v2), "wa"(v3), 
          "wa"(v4), "wa"(v5), "wa"(v6), "wa"(v7),
          "wa"(v8), "wa"(v9)
        : "v10"
    );
    
    /* Simple validation */
    float *result = (float*)&v10;
    VALIDATE(result[0] == 0.0f, "PowerPC permutation validation");
    
    return 1;
}

#endif /* __ALTIVEC__ */

/* ==================== Generic Inline Assembly Fallback ==================== */

NOINLINE int test_generic_10_operands(void) {
    /* Generic inline assembly with exactly 10 operands */
    printf("Testing generic 10-operand inline assembly...\n");
    
    long ops[10];
    long result = 0;
    
    for (int i = 0; i < 10; i++) {
        ops[i] = i + 1;
    }
    
    /* 10-operand inline assembly pattern */
    asm volatile (
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        : "=r"(result)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), "r"(ops[3]),
          "r"(ops[4]), "r"(ops[5]), "r"(ops[6]), "r"(ops[7]),
          "r"(ops[8]), "r"(ops[9])
        : "cc"
    );
    
    /* Expected: sum of 1..10 = 55 */
    VALIDATE(result == 55, "Generic 10-operand assembly validation");
    
    return 1;
}

NOINLINE int test_generic_11_operands(void) {
    /* Generic inline assembly with exactly 11 operands */
    printf("Testing generic 11-operand inline assembly...\n");
    
    long ops[11];
    long result = 0;
    
    for (int i = 0; i < 11; i++) {
        ops[i] = i + 1;
    }
    
    /* 11-operand inline assembly pattern */
    asm volatile (
        "mov %0, %1\n\t"
        "imul %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "sub %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "sub %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "sub %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "sub %0, %0, %10\n\t"
        : "=r"(result)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), "r"(ops[3]),
          "r"(ops[4]), "r"(ops[5]), "r"(ops[6]), "r"(ops[7]),
          "r"(ops[8]), "r"(ops[9]), "r"(ops[10])
        : "cc"
    );
    
    /* Expected: 1*2 + 3 - 4 + 5 - 6 + 7 - 8 + 9 - 10 = -2 */
    VALIDATE(result == -2, "Generic 11-operand assembly validation");
    
    return 1;
}

/* ==================== Main Function ==================== */

int main(void) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("=== Testing RTL Expansion for 10-11 Operand Cases ===\n\n");
    
    /* Test architecture-specific patterns */
#ifdef __AVX512F__
    printf("\n--- x86 AVX-512 Tests ---\n");
    total_tests++;
    if (test_avx512_10_operands()) passed_tests++;
    
    total_tests++;
    if (test_avx512_11_operands()) passed_tests++;
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("\n--- ARM SVE Tests ---\n");
    total_tests++;
    if (test_arm_sve_10_operands()) passed_tests++;
#endif
    
#ifdef __ALTIVEC__
    printf("\n--- PowerPC Altivec/VSX Tests ---\n");
    total_tests++;
    if (test_powerpc_11_operands()) passed_tests++;
#endif
    
    /* Always test generic inline assembly */
    printf("\n--- Generic Inline Assembly Tests ---\n");
    total_tests++;
    if (test_generic_10_operands()) passed_tests++;
    
    total_tests++;
    if (test_generic_11_operands()) passed_tests++;
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests compiled: %d\n", total_tests);
    printf("Tests passed: %d\n", passed_tests);
    
    if (passed_tests == total_tests) {
        printf("\nSUCCESS: All tests passed!\n");
        return 0;
    } else {
        printf("\nWARNING: Some tests failed or were not compiled\n");
        return 1;
    }
}
