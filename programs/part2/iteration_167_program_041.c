/* Test program to trigger 10-11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation utilities */
#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s\n", msg); \
        return 0; \
    } \
} while(0)

#define PASS(msg) printf("PASS: %s\n", msg)

/* Force functions to not be inlined before RTL expansion */
#define NOINLINE __attribute__((noinline))

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

NOINLINE int test_avx512_10_operands(void) {
    /* Pattern A: 10 operands - masked gather operation */
    printf("Testing AVX-512 10-operand pattern...\n");
    
    /* Initialize test data */
    double base[1024] __attribute__((aligned(64)));
    int64_t indices[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 1024; i++) {
        base[i] = (double)i;
    }
    
    /* Create mask (all ones) */
    __mmask8 mask = 0xFF;
    
    /* This gather intrinsic expands to approximately 10 operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Index vector
     * 4. Base pointer
     * 5. Scale
     * 6. Vector length hint
     * 7. Source (for merge-masked version)
     * Plus implicit operands for addressing modes
     */
    __m512d src = _mm512_set1_pd(0.0);
    __m512i vindex = _mm512_loadu_si512((const __m512i*)indices);
    
    /* _mm512_mask_i64gather_pd with many parameters */
    __m512d gathered = _mm512_mask_i64gather_pd(
        src,                    /* src (merge operand) */
        mask,                   /* mask */
        vindex,                 /* index vector */
        base,                   /* base pointer */
        8                       /* scale (sizeof(double)) */
    );
    
    _mm512_store_pd(result, gathered);
    
    /* Validate results */
    for (int i = 0; i < 8; i++) {
        double expected = base[indices[i]];
        CHECK(result[i] == expected, "AVX-512 gather result mismatch");
    }
    
    PASS("AVX-512 10-operand pattern");
    return 1;
}

NOINLINE int test_avx512_11_operands(void) {
    /* Pattern B: 11 operands - complex masked scatter with update */
    printf("Testing AVX-512 11-operand pattern...\n");
    
    /* Initialize test data */
    double dest[1024] __attribute__((aligned(64)));
    double src_data[8] __attribute__((aligned(64)));
    int64_t indices[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    
    memset(dest, 0, sizeof(dest));
    for (int i = 0; i < 8; i++) {
        src_data[i] = (double)(i + 1000);
    }
    
    /* Create mask (alternating pattern) */
    __mmask8 mask = 0xAA; /* 0b10101010 */
    
    __m512d src = _mm512_load_pd(src_data);
    __m512i vindex = _mm512_loadu_si512((const __m512i*)indices);
    
    /* _mm512_mask_i64scatter_pd - this can expand to 11+ operands
     * when considering all addressing mode components */
    _mm512_mask_i64scatter_pd(
        dest,                   /* base pointer */
        mask,                   /* mask */
        vindex,                 /* index vector */
        src,                    /* source data */
        8                       /* scale */
    );
    
    /* Validate scattered data */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        if (mask & (1 << i)) {
            CHECK(dest[indices[i]] == src_data[i], 
                  "AVX-512 scatter result mismatch");
            checksum++;
        }
    }
    CHECK(checksum == 4, "AVX-512 scatter mask count mismatch");
    
    PASS("AVX-512 11-operand pattern");
    return 1;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

NOINLINE int test_arm_sve_10_operands(void) {
    /* Pattern A: 10 operands - SVE gather with predicate */
    printf("Testing ARM SVE 10-operand pattern...\n");
    
    /* Initialize test data */
    uint64_t base[1024];
    uint64_t indices[256];  /* SVE vector length dependent */
    uint64_t result[256];
    
    for (int i = 0; i < 1024; i++) {
        base[i] = i;
    }
    for (int i = 0; i < 256; i++) {
        indices[i] = i * 4;
    }
    
    /* Create all-true predicate */
    svbool_t pg = svptrue_b64();
    
    /* SVE gather - expands to multiple operands including:
     * 1. Predicate
     * 2. Base pointer
     * 3. Offset vector
     * 4. Vector length
     * Plus implicit operands */
    svuint64_t offset_vec = svld1_u64(pg, indices);
    svuint64_t gathered = svld1_gather_u64index_u64(pg, base, offset_vec);
    
    svst1_u64(pg, result, gathered);
    
    /* Validate */
    for (int i = 0; i < 256; i++) {
        CHECK(result[i] == base[indices[i]], "SVE gather result mismatch");
    }
    
    PASS("ARM SVE 10-operand pattern");
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec/VSX Implementation ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

NOINLINE int test_powerpc_11_operands(void) {
    /* Pattern B: 11 operands - complex vector permutation */
    printf("Testing PowerPC 11-operand pattern...\n");
    
    /* Initialize vectors */
    vector unsigned char a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    vector unsigned char b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    vector unsigned char c = {32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47};
    vector unsigned char d = {48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63};
    
    /* Complex permutation pattern - vec_perm with multiple inputs
     * can expand to many operands when unrolled */
    vector unsigned char perm_mask = {
        0,16,32,48, 1,17,33,49, 2,18,34,50, 3,19,35,51
    };
    
    /* This sequence may generate 11+ operand patterns when optimized */
    vector unsigned char result1 = vec_perm(a, b, perm_mask);
    vector unsigned char result2 = vec_perm(c, d, perm_mask);
    vector unsigned char result3 = vec_perm(result1, result2, perm_mask);
    
    /* Validate by checking known positions */
    vector unsigned char expected = {
        0,16,32,48, 1,17,33,49, 2,18,34,50, 3,19,35,51
    };
    
    CHECK(vec_all_eq(result3, expected), "PowerPC permutation mismatch");
    
    PASS("PowerPC 11-operand pattern");
    return 1;
}

#endif /* __ALTIVEC__ */

/* ==================== Generic Inline Assembly Fallback ==================== */
#ifndef __AVX512F__
#ifndef __ARM_FEATURE_SVE
#ifndef __ALTIVEC__

/* Generic inline assembly with exactly 10 and 11 operands */
NOINLINE int test_generic_10_operands(void) {
    printf("Testing generic 10-operand inline assembly...\n");
    
    long ops[10];
    long result = 0;
    
    /* Initialize operands */
    for (int i = 0; i < 10; i++) {
        ops[i] = i + 1;
    }
    
    /* 10-operand inline assembly pattern */
    asm volatile (
        "/* 10-operand dummy pattern */\n\t"
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r" (result)
        : "r" (ops[0]), "r" (ops[1]), "r" (ops[2]), 
          "r" (ops[3]), "r" (ops[4]), "r" (ops[5]),
          "r" (ops[6]), "r" (ops[7]), "r" (ops[8])
        : "cc"
    );
    
    CHECK(result == 45, "Generic 10-operand assembly result mismatch");
    PASS("Generic 10-operand pattern");
    return 1;
}

NOINLINE int test_generic_11_operands(void) {
    printf("Testing generic 11-operand inline assembly...\n");
    
    long ops[11];
    long result = 0;
    
    /* Initialize operands */
    for (int i = 0; i < 11; i++) {
        ops[i] = i + 1;
    }
    
    /* 11-operand inline assembly pattern */
    asm volatile (
        "/* 11-operand dummy pattern */\n\t"
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
        : "=r" (result)
        : "r" (ops[0]), "r" (ops[1]), "r" (ops[2]), 
          "r" (ops[3]), "r" (ops[4]), "r" (ops[5]),
          "r" (ops[6]), "r" (ops[7]), "r" (ops[8]),
          "r" (ops[9]), "r" (ops[10])
        : "cc"
    );
    
    CHECK(result == 66, "Generic 11-operand assembly result mismatch");
    PASS("Generic 11-operand pattern");
    return 1;
}

#endif
#endif
#endif

/* ==================== Main Test Driver ==================== */
int main(void) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("=== Testing 10-11 operand RTL expansion coverage ===\n\n");
    
    /* Test architecture-specific patterns */
#ifdef __AVX512F__
    printf("AVX-512 detected, testing...\n");
    total_tests++; passed_tests += test_avx512_10_operands();
    total_tests++; passed_tests += test_avx512_11_operands();
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("ARM SVE detected, testing...\n");
    total_tests++; passed_tests += test_arm_sve_10_operands();
#endif
    
#ifdef __ALTIVEC__
    printf("PowerPC Altivec detected, testing...\n");
    total_tests++; passed_tests += test_powerpc_11_operands();
#endif

/* Fallback to generic inline assembly if no vector ISA detected */
#if !defined(__AVX512F__) && !defined(__ARM_FEATURE_SVE) && !defined(__ALTIVEC__)
    printf("No vector ISA detected, using generic inline assembly...\n");
    total_tests++; passed_tests += test_generic_10_operands();
    total_tests++; passed_tests += test_generic_11_operands();
#endif
    
    printf("\n=== Summary ===\n");
    printf("Tests run: %d\n", total_tests);
    printf("Tests passed: %d\n", passed_tests);
    
    if (total_tests == passed_tests) {
        printf("\nSUCCESS: All tests passed!\n");
        return 0;
    } else {
        printf("\nFAILURE: Some tests failed\n");
        return 1;
    }
}
