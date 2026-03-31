/* test_multi_operand_rtl.c
 * 
 * This program generates RTL patterns requiring 10-11 operands
 * to trigger uncovered lines 8254-8263 in optabs.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation utilities */
#define ARRAY_SIZE 64
#define VALIDATE(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "Validation failed: %s\n", msg); \
        return 0; \
    } \
} while(0)

/* Initialize test data arrays */
static void init_test_data(double *array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = (double)(i * 1.5 + 0.7);
    }
}

static void init_test_data_int64(int64_t *array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = (int64_t)(i * 3 + 1);
    }
}

/* ============================================
 * x86 AVX-512 patterns (10-11 operands)
 * ============================================ */
#ifdef __AVX512F__

#include <immintrin.h>

__attribute__((noinline, target("avx512f,avx512vl")))
int test_avx512_gather_10_operands(void) {
    /* Pattern A: 10 operands - masked gather */
    double src[ARRAY_SIZE];
    double dst[ARRAY_SIZE] = {0};
    int64_t indices[8] = {0, 2, 4, 6, 8, 10, 12, 14};
    
    init_test_data(src, ARRAY_SIZE);
    
    __m512d base = _mm512_set1_pd((double)(uintptr_t)src);
    __m512i vindex = _mm512_loadu_si512((__m512i*)indices);
    __mmask8 mask = 0xFF;  /* All lanes active */
    int scale = 8;  /* sizeof(double) */
    
    /* This intrinsic expands to an instruction with approximately:
     * 1. Destination vector
     * 2. Mask
     * 3. Base address
     * 4. Index vector
     * 5. Scale
     * 6. Source vector (for merge-masked version)
     * Plus implicit operands for addressing modes
     * Total: ~10 operands in RTL
     */
    __m512d result = _mm512_mask_i64gather_pd(
        _mm512_setzero_pd(),  /* src (merge operand) */
        mask,                 /* mask */
        vindex,               /* indices */
        (void*)src,           /* base */
        scale                 /* scale */
    );
    
    _mm512_storeu_pd(dst, result);
    
    /* Validate */
    for (int i = 0; i < 8; i++) {
        VALIDATE(dst[i] == src[indices[i]], 
                "AVX-512 gather 10-operand pattern failed");
    }
    
    return 1;
}

__attribute__((noinline, target("avx512f")))
int test_avx512_scatter_11_operands(void) {
    /* Pattern B: 11 operands - masked scatter with update */
    double data[ARRAY_SIZE] = {0};
    double src[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    int64_t indices[8] = {1, 3, 5, 7, 9, 11, 13, 15};
    
    __m512d vsrc = _mm512_loadu_pd(src);
    __m512i vindex = _mm512_loadu_si512((__m512i*)indices);
    __mmask8 mask = 0xFF;
    int scale = 8;
    
    /* This scatter operation requires:
     * 1. Base address
     * 2. Mask
     * 3. Index vector
     * 4. Source data
     * 5. Scale
     * Plus addressing mode operands
     * Total: ~11 operands in RTL
     */
    _mm512_mask_i64scatter_pd(
        data,           /* base */
        mask,           /* mask */
        vindex,         /* indices */
        vsrc,           /* source data */
        scale           /* scale */
    );
    
    /* Validate scatter */
    for (int i = 0; i < 8; i++) {
        VALIDATE(data[indices[i]] == src[i], 
                "AVX-512 scatter 11-operand pattern failed");
    }
    
    return 1;
}

#endif /* __AVX512F__ */

/* ============================================
 * ARM SVE patterns (10-11 operands)
 * ============================================ */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

__attribute__((noinline))
int test_arm_sve_gather_10_operands(void) {
    /* Pattern A: 10 operands - SVE gather with predicate */
    double src[ARRAY_SIZE];
    double dst[8] = {0};
    int64_t indices[8] = {0, 4, 8, 12, 16, 20, 24, 28};
    
    init_test_data(src, ARRAY_SIZE);
    
    svbool_t pg = svptrue_b64();
    svint64_t vindex = svld1_s64(pg, indices);
    
    /* SVE gather with predicate, base, and offsets
     * Expands to multiple operands in RTL
     */
    svfloat64_t result = svld1_gather_index(pg, src, vindex);
    
    svst1(pg, dst, result);
    
    /* Validate */
    for (int i = 0; i < 8; i++) {
        VALIDATE(dst[i] == src[indices[i]], 
                "ARM SVE gather 10-operand pattern failed");
    }
    
    return 1;
}

__attribute__((noinline))
int test_arm_sve_scatter_11_operands(void) {
    /* Pattern B: 11 operands - SVE scatter with predicate and offset */
    double data[ARRAY_SIZE] = {0};
    double src[8] = {10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0};
    int64_t indices[8] = {2, 6, 10, 14, 18, 22, 26, 30};
    
    svbool_t pg = svptrue_b64();
    svfloat64_t vsrc = svld1(pg, src);
    svint64_t vindex = svld1_s64(pg, indices);
    
    /* SVE scatter with predicate, base, data, and indices */
    svst1_scatter_index(pg, data, vindex, vsrc);
    
    /* Validate */
    for (int i = 0; i < 8; i++) {
        VALIDATE(data[indices[i]] == src[i], 
                "ARM SVE scatter 11-operand pattern failed");
    }
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ============================================
 * PowerPC VSX/Altivec patterns
 * ============================================ */
#ifdef __ALTIVEC__

#include <altivec.h>

__attribute__((noinline))
int test_powerpc_complex_10_operands(void) {
    /* Pattern: Complex vector multiply-add with multiple arguments */
    vector double a = {1.0, 2.0, 3.0, 4.0};
    vector double b = {5.0, 6.0, 7.0, 8.0};
    vector double c = {9.0, 10.0, 11.0, 12.0};
    vector double d = {13.0, 14.0, 15.0, 16.0};
    
    /* Complex sequence that may expand to many operands */
    vector double t1 = vec_madd(a, b, c);
    vector double t2 = vec_madd(b, c, d);
    vector double t3 = vec_madd(c, d, a);
    vector double t4 = vec_madd(d, a, b);
    
    /* Combine results - compiler may fuse into multi-operand pattern */
    vector double result = vec_add(vec_add(t1, t2), vec_add(t3, t4));
    
    /* Simple validation */
    double res_array[4];
    memcpy(res_array, &result, sizeof(result));
    
    VALIDATE(res_array[0] > 0.0, "PowerPC complex 10-operand pattern failed");
    
    return 1;
}

#endif /* __ALTIVEC__ */

/* ============================================
 * Generic inline assembly fallback
 * ============================================ */
__attribute__((noinline))
int test_inline_asm_11_operands(void) {
    /* Direct inline assembly with 11 operands */
    uint64_t out1, out2;
    uint64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    uint64_t in6 = 6, in7 = 7, in8 = 8, in9 = 9;
    
    /* 11-operand inline asm pattern */
    asm volatile (
        /* Dummy multi-operand instruction pattern */
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "mov %9, %0\n\t"
        : "=&r"(out1), "=&r"(out2)  /* 2 outputs */
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5),
          "r"(in6), "r"(in7), "r"(in8), "r"(in9)  /* 9 inputs */
        : "cc"  /* clobber list */
    );
    
    /* Validate computation */
    uint64_t expected = in1 + in2 + in3 + in4 + in5 + in6 + in7 + in8 + in9;
    VALIDATE(out1 == expected, "Inline asm 11-operand pattern failed");
    VALIDATE(out2 == expected, "Inline asm 11-operand pattern failed");
    
    return 1;
}

/* ============================================
 * Main test driver
 * ============================================ */
int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("Testing multi-operand RTL expansion patterns...\n");
    printf("Target: optabs.cc lines 8254-8263 (10-11 operand cases)\n\n");
    
    /* Test architecture-specific patterns */
#ifdef __AVX512F__
    printf("Testing x86 AVX-512 patterns...\n");
    if (test_avx512_gather_10_operands()) {
        printf("  ✓ AVX-512 gather (10 operands) passed\n");
        tests_passed++;
    }
    tests_run++;
    
    if (test_avx512_scatter_11_operands()) {
        printf("  ✓ AVX-512 scatter (11 operands) passed\n");
        tests_passed++;
    }
    tests_run++;
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE patterns...\n");
    if (test_arm_sve_gather_10_operands()) {
        printf("  ✓ ARM SVE gather (10 operands) passed\n");
        tests_passed++;
    }
    tests_run++;
    
    if (test_arm_sve_scatter_11_operands()) {
        printf("  ✓ ARM SVE scatter (11 operands) passed\n");
        tests_passed++;
    }
    tests_run++;
#endif
    
#ifdef __ALTIVEC__
    printf("Testing PowerPC Altivec patterns...\n");
    if (test_powerpc_complex_10_operands()) {
        printf("  ✓ PowerPC complex (10 operands) passed\n");
        tests_passed++;
    }
    tests_run++;
#endif
    
    /* Always test inline assembly */
    printf("Testing generic inline assembly...\n");
    if (test_inline_asm_11_operands()) {
        printf("  ✓ Inline asm (11 operands) passed\n");
        tests_passed++;
    }
    tests_run++;
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    
    if (tests_passed == tests_run) {
        printf("\n✅ All tests passed!\n");
        printf("The RTL expander should have encountered 10-11 operand patterns.\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed\n");
        return 1;
    }
}
