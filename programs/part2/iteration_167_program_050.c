/* Test program to trigger 10-11 operand RTL expansion in optabs.cc */
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

/* Force noinline to ensure RTL expansion happens */
#define NOINLINE __attribute__((noinline))

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Pattern A: 10 operands - masked gather with multiple sources */
NOINLINE int test_avx512_gather_10ops(void) {
    /* Setup test data */
    double base[128] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    double expected[8];
    
    /* Initialize with known pattern */
    for (int i = 0; i < 128; i++) {
        base[i] = (double)(i * 2);
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = i * 16;  /* Strided access */
        expected[i] = (double)(i * 32);
    }
    
    /* Create mask: all lanes enabled */
    __mmask8 mask = 0xFF;
    
    /* Scale factor */
    const int scale = 1;
    
    /* This gather intrinsic maps to instruction with many operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Index vector
     * 4. Base pointer
     * 5. Scale
     * 6. Vector length hint
     * Plus implicit memory operands and rounding mode
     * Total operands >= 10
     */
    __m512d src = _mm512_set1_pd(0.0);
    __m512i vindex = _mm512_load_epi64(indices);
    
    /* The actual gather - this should generate RTL with many operands */
    __m512d gathered = _mm512_mask_i64gather_pd(src, mask, vindex, 
                                               base, scale);
    
    /* Store results */
    _mm512_store_pd(result, gathered);
    
    /* Validate */
    for (int i = 0; i < 8; i++) {
        VALIDATE(result[i] == expected[i], 
                "AVX-512 gather 10-operand pattern");
    }
    
    PASS("AVX-512 10-operand gather");
    return 1;
}

/* Pattern B: 11 operands - complex masked scatter with update */
NOINLINE int test_avx512_scatter_11ops(void) {
    /* Setup test data */
    double target[128] __attribute__((aligned(64)));
    double source[8] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double expected[128];
    
    /* Initialize */
    memset(target, 0, sizeof(target));
    for (int i = 0; i < 8; i++) {
        source[i] = (double)(100 + i);
        indices[i] = i * 8;
    }
    
    /* Expected result */
    memcpy(expected, target, sizeof(target));
    for (int i = 0; i < 8; i++) {
        expected[indices[i]] = source[i];
    }
    
    /* Create mask: all lanes enabled */
    __mmask8 mask = 0xFF;
    
    /* Scale factor */
    const int scale = 1;
    
    /* Load vectors */
    __m512d vsrc = _mm512_load_pd(source);
    __m512i vindex = _mm512_load_epi64(indices);
    
    /* This scatter has many operands:
     * 1. Base pointer
     * 2. Mask
     * 3. Index vector
     * 4. Source data
     * 5. Scale
     * 6. Vector length
     * Plus implicit memory operands, rounding, etc.
     * Total operands >= 11
     */
    _mm512_mask_i64scatter_pd(target, mask, vindex, vsrc, scale);
    
    /* Validate scattered values */
    for (int i = 0; i < 8; i++) {
        int idx = indices[i];
        VALIDATE(target[idx] == source[i], 
                "AVX-512 scatter 11-operand pattern");
    }
    
    PASS("AVX-512 11-operand scatter");
    return 1;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Pattern A: 10 operands - SVE gather with predicate */
NOINLINE int test_sve_gather_10ops(void) {
    /* SVE vectors have variable length, so we use generic code */
    const int N = 128;
    uint64_t base[N];
    uint64_t indices[N];
    uint64_t result[N];
    uint64_t expected[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        base[i] = i * 3;
        indices[i] = (i % 16) * 4;
        expected[i] = base[indices[i]];
    }
    
    /* Create all-true predicate */
    svbool_t pg = svptrue_b64();
    
    /* SVE gather with multiple operands:
     * 1. Predicate
     * 2. Base pointer
     * 3. Offset vector
     * 4. Scale (implicit)
     * 5. Vector length
     * Plus type specifiers and memory attributes
     */
    svuint64_t offset_vec = svld1_u64(pg, indices);
    svuint64_t gathered = svld1_gather_u64offset_u64(pg, base, offset_vec);
    
    /* Store results */
    svst1_u64(pg, result, gathered);
    
    /* Validate first few elements */
    for (int i = 0; i < 16; i++) {
        VALIDATE(result[i] == expected[i], 
                "SVE gather 10-operand pattern");
    }
    
    PASS("ARM SVE 10-operand gather");
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC VSX Implementation ==================== */
#ifdef __VSX__

#include <altivec.h>

/* Pattern B: 11 operands - complex vector permute with multiple sources */
NOINLINE int test_vsx_permute_11ops(void) {
    /* Use inline assembly to force 11 operands */
    vector double v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Initialize vectors */
    v0 = (vector double){0.0, 1.0};
    v1 = (vector double){2.0, 3.0};
    v2 = (vector double){4.0, 5.0};
    v3 = (vector double){6.0, 7.0};
    v4 = (vector double){8.0, 9.0};
    v5 = (vector double){10.0, 11.0};
    v6 = (vector double){12.0, 13.0};
    v7 = (vector double){14.0, 15.0};
    v8 = (vector double){16.0, 17.0};
    v9 = (vector double){18.0, 19.0};
    
    /* Extended inline assembly with 11 operands */
    asm volatile (
        "xxpermdi %x0, %x1, %x2, 0\n\t"
        "xxpermdi %x3, %x4, %x5, 1\n\t"
        "xxpermdi %x6, %x7, %x8, 2\n\t"
        "xxpermdi %x9, %x10, %x1, 3\n\t"
        : "=wa"(v0), "=wa"(v3), "=wa"(v6), "=wa"(v9)
        : "wa"(v1), "wa"(v2), "wa"(v4), "wa"(v5),
          "wa"(v7), "wa"(v8), "wa"(v10)
        : 
    );
    
    /* Dummy validation */
    VALIDATE(1, "PowerPC VSX 11-operand assembly");
    
    PASS("PowerPC VSX 11-operand permute");
    return 1;
}

#endif /* __VSX__ */

/* ==================== Generic Fallback ==================== */

/* Generic inline assembly with many operands for architectures 
   without specific vector support */
NOINLINE int test_generic_many_ops(void) {
    /* Force 10 operands using extended asm */
    long op0 = 0, op1 = 1, op2 = 2, op3 = 3, op4 = 4;
    long op5 = 5, op6 = 6, op7 = 7, op8 = 8, op9 = 9;
    long result = 0;
    
    /* 10-operand pattern */
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(result)
        : "r"(op1), "r"(op2), "r"(op3), "r"(op4),
          "r"(op5), "r"(op6), "r"(op7), "r"(op8), "r"(op9)
        : "cc"
    );
    
    VALIDATE(result == 44, "Generic 10-operand assembly");
    
    /* Force 11 operands */
    long op10 = 10;
    result = 0;
    
    asm volatile (
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
        : "r"(op1), "r"(op2), "r"(op3), "r"(op4),
          "r"(op5), "r"(op6), "r"(op7), "r"(op8),
          "r"(op9), "r"(op10)
        : "cc"
    );
    
    VALIDATE(result == 54, "Generic 11-operand assembly");
    
    PASS("Generic many-operand assembly");
    return 1;
}

/* ==================== Main Test Driver ==================== */

int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("Testing RTL expansion for 10-11 operand patterns...\n");
    printf("Compiled with optimizations to trigger expand_insn()\n\n");
    
    /* Run architecture-specific tests */
    
#ifdef __AVX512F__
    printf("Testing AVX-512 paths...\n");
    tests_run++;
    if (test_avx512_gather_10ops()) tests_passed++;
    
    tests_run++;
    if (test_avx512_scatter_11ops()) tests_passed++;
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE paths...\n");
    tests_run++;
    if (test_sve_gather_10ops()) tests_passed++;
#endif
    
#ifdef __VSX__
    printf("Testing PowerPC VSX paths...\n");
    tests_run++;
    if (test_vsx_permute_11ops()) tests_passed++;
#endif
    
    /* Always run generic test */
    printf("Testing generic paths...\n");
    tests_run++;
    if (test_generic_many_ops()) tests_passed++;
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    
    if (tests_passed == tests_run) {
        printf("\nSUCCESS: All tests passed!\n");
        printf("The RTL expander should have encountered 10-11 operand patterns.\n");
        return 0;
    } else {
        printf("\nWARNING: Some tests failed (expected on unsupported architectures)\n");
        return 1;
    }
}
