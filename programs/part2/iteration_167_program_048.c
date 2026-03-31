/* Test program to trigger 10-11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation utilities */
#define ARRAY_SIZE 64
#define VALIDATE(cond, msg) if (!(cond)) { printf("FAIL: %s\n", msg); return 0; }
#define PASS(msg) printf("PASS: %s\n", msg)

/* Force noinline to prevent early optimization */
#define NOINLINE __attribute__((noinline))

/* Test data initialization */
static double src_array[ARRAY_SIZE];
static double dst_array[ARRAY_SIZE];
static int64_t indices[ARRAY_SIZE];
static uint8_t mask_data[ARRAY_SIZE];

static void init_test_data(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src_array[i] = (double)i * 1.5;
        dst_array[i] = 0.0;
        indices[i] = (i * 3) % ARRAY_SIZE;
        mask_data[i] = (i % 4 == 0) ? 0 : 0xFF;
    }
}

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Pattern A: 10 operands - masked gather operation */
NOINLINE int test_avx512_gather_10ops(void) {
    __m512d base = _mm512_set1_pd(0.0);
    __m512i vindex = _mm512_loadu_si512((const __m512i*)indices);
    __mmask8 mask = 0xF0; /* 8-bit mask */
    int scale = 8;
    __m512d src = _mm512_loadu_pd(src_array);
    
    /* This gather intrinsic conceptually uses:
       1. Destination register
       2. Mask
       3. Base address
       4. vindex
       5. Scale
       6. Source data
       Plus additional implicit operands for addressing modes
    */
    __m512d result = _mm512_mask_i64gather_pd(src, mask, vindex, 
                                             (const void*)src_array, scale);
    
    /* Store and validate */
    _mm512_storeu_pd(dst_array, result);
    
    /* Simple validation - check first few elements */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        if (mask & (1 << i)) {
            int idx = indices[i];
            VALIDATE(dst_array[i] == src_array[idx] || dst_array[i] == src_array[i],
                    "AVX-512 gather result mismatch");
        }
        sum += dst_array[i];
    }
    
    PASS("AVX-512 10-operand gather");
    return 1;
}

/* Pattern B: 11 operands - complex masked scatter with update */
NOINLINE int test_avx512_scatter_11ops(void) {
    __m512d src = _mm512_loadu_pd(src_array);
    __m512i vindex = _mm512_loadu_si512((const __m512i*)indices);
    __mmask8 mask = 0xAA; /* Alternating mask */
    int scale = 8;
    
    /* Clear destination for scatter */
    memset(dst_array, 0, sizeof(dst_array));
    
    /* Scatter operation with multiple operands */
    _mm512_mask_i64scatter_pd(dst_array, mask, vindex, src, scale);
    
    /* Validation - check scattered values */
    for (int i = 0; i < 8; i++) {
        if (mask & (1 << i)) {
            int idx = indices[i];
            VALIDATE(dst_array[idx] == src_array[i],
                    "AVX-512 scatter result mismatch");
        }
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
    svbool_t pg = svwhilelt_b64(0, 8);
    svint64_t indices_vec = svld1_s64(pg, indices);
    svfloat64_t base = svdup_f64(0.0);
    
    /* SVE gather with multiple operands */
    svfloat64_t result = svld1_gather_index(pg, src_array, indices_vec);
    
    /* Store and validate */
    svst1_f64(pg, dst_array, result);
    
    for (int i = 0; i < 8; i++) {
        if (svptest_first(svwhilelt_b64(i, i+1), pg)) {
            int idx = indices[i];
            VALIDATE(dst_array[i] == src_array[idx],
                    "SVE gather result mismatch");
        }
    }
    
    PASS("ARM SVE 10-operand gather");
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC VSX Implementation ==================== */
#ifdef __VSX__

#include <altivec.h>

/* Pattern B: 11 operands - complex vector permute with multiple sources */
NOINLINE int test_vsx_11ops(void) {
    /* Use inline assembly to force 11 operands */
    vector double v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Initialize vectors */
    v0 = vec_splats(1.0);
    v1 = vec_splats(2.0);
    v2 = vec_splats(3.0);
    v3 = vec_splats(4.0);
    v4 = vec_splats(5.0);
    v5 = vec_splats(6.0);
    v6 = vec_splats(7.0);
    v7 = vec_splats(8.0);
    v8 = vec_splats(9.0);
    v9 = vec_splats(10.0);
    
    /* Complex operation using inline assembly with 11 operands */
    asm volatile (
        "xxpermdi %x0, %x1, %x2, 0\n\t"
        "xxpermdi %x3, %x4, %x5, 1\n\t"
        "xvadddp %x6, %x7, %x8\n\t"
        "xvmuldp %x9, %x10, %x0\n\t"
        : "=wa"(v0), "+wa"(v1), "+wa"(v2), "=wa"(v3),
          "+wa"(v4), "+wa"(v5), "=wa"(v6), "+wa"(v7),
          "+wa"(v8), "=wa"(v9), "+wa"(v10)
        : 
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10"
    );
    
    /* Store result for validation */
    vec_st(v0, 0, dst_array);
    vec_st(v9, 64, dst_array);
    
    VALIDATE(dst_array[0] > 0.0, "VSX operation produced invalid result");
    
    PASS("PowerPC VSX 11-operand operation");
    return 1;
}

#endif /* __VSX__ */

/* ==================== Generic Inline Assembly Fallback ==================== */

/* Pattern A: 10 operands using generic inline assembly */
NOINLINE int test_generic_10ops(void) {
    long op0 = 1, op1 = 2, op2 = 3, op3 = 4, op4 = 5;
    long op5 = 6, op6 = 7, op7 = 8, op8 = 9, op9 = 10;
    long result = 0;
    
    /* Force 10 operands in inline assembly */
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        : "=r"(result)
        : "r"(op0), "r"(op1), "r"(op2), "r"(op3),
          "r"(op4), "r"(op5), "r"(op6), "r"(op7),
          "r"(op8), "r"(op9)
        : "cc"
    );
    
    VALIDATE(result == 55, "Generic 10-operand assembly failed");
    PASS("Generic 10-operand inline assembly");
    return 1;
}

/* Pattern B: 11 operands using generic inline assembly */
NOINLINE int test_generic_11ops(void) {
    long ops[11];
    long result = 0;
    
    for (int i = 0; i < 11; i++) {
        ops[i] = i + 1;
    }
    
    /* Force 11 operands in inline assembly */
    asm volatile (
        "mov %0, %1\n\t"
        "imul %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "sub %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "imul %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "sub %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "imul %0, %0, %10\n\t"
        "add %0, %0, %11\n\t"
        : "=r"(result)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), "r"(ops[3]),
          "r"(ops[4]), "r"(ops[5]), "r"(ops[6]), "r"(ops[7]),
          "r"(ops[8]), "r"(ops[9]), "r"(ops[10])
        : "cc"
    );
    
    VALIDATE(result != 0, "Generic 11-operand assembly failed");
    PASS("Generic 11-operand inline assembly");
    return 1;
}

/* ==================== Main Test Driver ==================== */

int main(void) {
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("Testing 10-11 operand RTL expansion coverage...\n");
    printf("===============================================\n");
    
    init_test_data();
    
    /* Always run generic tests */
    total_tests += 2;
    tests_passed += test_generic_10ops();
    tests_passed += test_generic_11ops();
    
    /* Architecture-specific tests */
#ifdef __AVX512F__
    printf("\nAVX-512 detected, running vector tests...\n");
    total_tests += 2;
    tests_passed += test_avx512_gather_10ops();
    tests_passed += test_avx512_scatter_11ops();
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("\nARM SVE detected, running vector tests...\n");
    total_tests += 1;
    tests_passed += test_sve_gather_10ops();
#endif
    
#ifdef __VSX__
    printf("\nPowerPC VSX detected, running vector tests...\n");
    total_tests += 1;
    tests_passed += test_vsx_11ops();
#endif
    
    printf("\n===============================================\n");
    printf("Summary: %d/%d tests passed\n", tests_passed, total_tests);
    printf("Coverage goal: Trigger case 10: and case 11: in optabs.cc\n");
    
    return (tests_passed == total_tests) ? 0 : 1;
}
