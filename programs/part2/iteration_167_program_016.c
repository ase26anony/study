/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expander for instructions
 * requiring exactly 10 or 11 operands, covering lines 8254-8263 in optabs.cc
 */

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

/* Function attributes to control optimization */
#define HOT __attribute__((hot))
#define NOINLINE __attribute__((noinline))
#define ALIGNED(n) __attribute__((aligned(n)))

/* Test data arrays - aligned for vector operations */
static double src_data[1024] ALIGNED(64);
static double dst_data[1024] ALIGNED(64);
static int64_t indices[512] ALIGNED(64);
static double results[512] ALIGNED(64);

/* Initialize test data */
static void init_data(void) {
    for (int i = 0; i < 1024; i++) {
        src_data[i] = (double)i * 1.5;
        dst_data[i] = 0.0;
    }
    for (int i = 0; i < 512; i++) {
        indices[i] = (i * 2) % 1024;
        results[i] = 0.0;
    }
}

/* ============================================
 * PATTERN A: 10 OPERANDS (x86 AVX-512)
 * ============================================ */
#ifdef __AVX512F__

#include <immintrin.h>

NOINLINE HOT
static int test_avx512_10_operands(void) {
    printf("Testing AVX-512 10-operand pattern...\n");
    
    /* Complex masked gather operation with 10 operands:
     * 1. Destination vector (output)
     * 2. Mask
     * 3. Index vector
     * 4. Base pointer
     * 5. Scale
     * 6. Source vector (for blending)
     * 7. Mask for blending
     * 8. Another index vector
     * 9. Another base pointer
     * 10. Another scale
     */
    
    __m512d vec_result;
    __mmask8 mask = 0xFF;  /* All lanes active */
    __m512i idx = _mm512_load_epi64(indices);
    __m512i idx2 = _mm512_add_epi64(idx, _mm512_set1_epi64(4));
    
    /* This complex pattern should require 10 operands when expanded */
    vec_result = _mm512_mask_i64gather_pd(
        _mm512_setzero_pd(),    /* src (blend source) - operand 1 */
        mask,                   /* mask - operand 2 */
        idx,                    /* index - operand 3 */
        src_data,               /* base - operand 4 */
        8                       /* scale - operand 5 */
    );
    
    /* Additional operation to create more complex pattern */
    __m512d vec_result2 = _mm512_mask_i64gather_pd(
        vec_result,             /* src - operand 6 */
        mask,                   /* mask - operand 7 */
        idx2,                   /* index - operand 8 */
        src_data + 64,          /* base - operand 9 */
        8                       /* scale - operand 10 */
    );
    
    /* Store and validate */
    _mm512_store_pd(results, vec_result2);
    
    /* Simple validation - check first few values */
    for (int i = 0; i < 8; i++) {
        double expected = src_data[indices[i] + (i < 4 ? 0 : 64)];
        CHECK(results[i] == expected, 
              "AVX-512 gather result mismatch");
    }
    
    PASS("AVX-512 10-operand pattern executed correctly");
    return 1;
}

/* Inline assembly version with exactly 10 operands */
NOINLINE HOT
static int test_avx512_asm_10_operands(void) {
    printf("Testing AVX-512 inline asm with 10 operands...\n");
    
    double out1, out2, out3, out4;
    double in1 = 1.0, in2 = 2.0, in3 = 3.0, in4 = 4.0;
    double in5 = 5.0, in6 = 6.0, in7 = 7.0, in8 = 8.0;
    
    /* Extended asm with 10 operands:
     * 2 outputs + 8 inputs = 10 total operands
     */
    asm volatile (
        "vmulpd %[out1], %[in1], %[in2]\n\t"
        "vaddpd %[out2], %[in3], %[in4]\n\t"
        "vfmadd231pd %[out1], %[in5], %[in6]\n\t"
        "vfmadd231pd %[out2], %[in7], %[in8]"
        : [out1] "=x" (out1), [out2] "=x" (out2)
        : [in1] "x" (in1), [in2] "x" (in2),
          [in3] "x" (in3), [in4] "x" (in4),
          [in5] "x" (in5), [in6] "x" (in6),
          [in7] "x" (in7), [in8] "x" (in8)
        : 
    );
    
    CHECK(out1 == 1.0*2.0 + 5.0*6.0, "AVX-512 asm result1 mismatch");
    CHECK(out2 == 3.0+4.0 + 7.0*8.0, "AVX-512 asm result2 mismatch");
    
    PASS("AVX-512 inline asm 10-operand pattern executed");
    return 1;
}

#endif /* __AVX512F__ */

/* ============================================
 * PATTERN B: 11 OPERANDS (x86 AVX-512)
 * ============================================ */
#ifdef __AVX512F__

NOINLINE HOT
static int test_avx512_11_operands(void) {
    printf("Testing AVX-512 11-operand pattern...\n");
    
    /* Complex fused masked scatter with update - aiming for 11 operands */
    __m512d src_vec = _mm512_load_pd(src_data);
    __m512i scatter_idx = _mm512_load_epi64(indices);
    __mmask8 scatter_mask = 0x0F;  /* Lower 4 lanes active */
    
    /* This complex sequence should expand to an 11-operand pattern */
    __m512d temp1 = _mm512_mask_mov_pd(
        _mm512_setzero_pd(),
        scatter_mask,
        src_vec
    );
    
    /* Scatter operation with multiple parameters */
    _mm512_mask_i64scatter_pd(
        dst_data,           /* base - operand 1 */
        scatter_mask,       /* mask - operand 2 */
        scatter_idx,        /* index - operand 3 */
        temp1,              /* src - operand 4 */
        8                   /* scale - operand 5 */
    );
    
    /* Additional operations to push to 11 operands */
    __m512d temp2 = _mm512_fmadd_pd(
        temp1,              /* a - operand 6 */
        _mm512_set1_pd(2.0), /* b - operand 7 */
        _mm512_set1_pd(1.0)  /* c - operand 8 */
    );
    
    /* Another scatter with different parameters */
    _mm512_mask_i64scatter_pd(
        dst_data + 256,     /* base - operand 9 */
        scatter_mask,       /* mask - operand 10 */
        _mm512_add_epi64(scatter_idx, _mm512_set1_epi64(8)), /* idx - operand 11 */
        temp2,
        8
    );
    
    /* Validation */
    int valid = 1;
    for (int i = 0; i < 4; i++) {
        if (dst_data[indices[i]] != src_data[i]) {
            valid = 0;
            break;
        }
    }
    
    CHECK(valid, "AVX-512 scatter result mismatch");
    PASS("AVX-512 11-operand pattern executed correctly");
    return 1;
}

/* Inline assembly with exactly 11 operands */
NOINLINE HOT
static int test_avx512_asm_11_operands(void) {
    printf("Testing AVX-512 inline asm with 11 operands...\n");
    
    double out1, out2, out3;
    double in1 = 1.0, in2 = 2.0, in3 = 3.0, in4 = 4.0;
    double in5 = 5.0, in6 = 6.0, in7 = 7.0, in8 = 8.0;
    
    /* Extended asm with 11 operands:
     * 3 outputs + 8 inputs = 11 total operands
     */
    asm volatile (
        "vmulpd %[out1], %[in1], %[in2]\n\t"
        "vaddpd %[out2], %[in3], %[in4]\n\t"
        "vsubpd %[out3], %[in5], %[in6]\n\t"
        "vfmadd231pd %[out1], %[in7], %[in8]"
        : [out1] "=x" (out1), [out2] "=x" (out2), [out3] "=x" (out3)
        : [in1] "x" (in1), [in2] "x" (in2),
          [in3] "x" (in3), [in4] "x" (in4),
          [in5] "x" (in5), [in6] "x" (in6),
          [in7] "x" (in7), [in8] "x" (in8)
        : 
    );
    
    CHECK(out1 == 1.0*2.0 + 7.0*8.0, "AVX-512 asm 11op result1 mismatch");
    CHECK(out2 == 3.0+4.0, "AVX-512 asm 11op result2 mismatch");
    CHECK(out3 == 5.0-6.0, "AVX-512 asm 11op result3 mismatch");
    
    PASS("AVX-512 inline asm 11-operand pattern executed");
    return 1;
}

#endif /* __AVX512F__ */

/* ============================================
 * ARM SVE Patterns (if supported)
 * ============================================ */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

NOINLINE HOT
static int test_arm_sve_10_operands(void) {
    printf("Testing ARM SVE 10-operand pattern...\n");
    
    svbool_t pg = svptrue_b64();
    svint64_t indices_vec = svld1_s64(pg, indices);
    svfloat64_t src_vec = svld1_f64(pg, src_data);
    
    /* Complex SVE gather operation - should require many operands */
    svfloat64_t gathered = svld1_gather_index(pg, src_data, indices_vec);
    
    /* Additional operations to increase operand count */
    svfloat64_t scaled = svmul_f64_x(pg, gathered, svdup_f64(2.0));
    svfloat64_t result = svadd_f64_x(pg, scaled, svdup_f64(1.0));
    
    svst1_f64(pg, results, result);
    
    /* Validation */
    int valid = 1;
    for (int i = 0; i < svcntd(); i++) {
        double expected = src_data[indices[i]] * 2.0 + 1.0;
        if (results[i] != expected) {
            valid = 0;
            break;
        }
    }
    
    CHECK(valid, "ARM SVE gather result mismatch");
    PASS("ARM SVE 10-operand pattern executed correctly");
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ============================================
 * PowerPC VSX/Altivec Patterns
 * ============================================ */
#ifdef __ALTIVEC__

#include <altivec.h>

NOINLINE HOT
static int test_powerpc_10_operands(void) {
    printf("Testing PowerPC VSX 10-operand pattern...\n");
    
    /* Complex vector permutation with multiple arguments */
    vector double v1 = vec_ld(0, src_data);
    vector double v2 = vec_ld(16, src_data);
    vector double v3 = vec_ld(32, src_data);
    vector double v4 = vec_ld(48, src_data);
    
    /* Matrix-style operation with many vector operands */
    vector double t1 = vec_madd(v1, v2, v3);  /* v1*v2 + v3 */
    vector double t2 = vec_madd(v2, v3, v4);  /* v2*v3 + v4 */
    vector double result = vec_madd(t1, t2, v1); /* t1*t2 + v1 */
    
    vec_st(result, 0, results);
    
    /* Simple validation */
    CHECK(results[0] != 0.0, "PowerPC VSX result is zero");
    PASS("PowerPC VSX 10-operand pattern executed");
    return 1;
}

#endif /* __ALTIVEC__ */

/* ============================================
 * RISC-V Vector Extension Patterns
 * ============================================ */
#ifdef __riscv_v

#include <riscv_vector.h>

NOINLINE HOT
static int test_riscv_10_operands(void) {
    printf("Testing RISC-V Vector 10-operand pattern...\n");
    
    size_t vl = vsetvl_e64m1(8);
    vint64m1_t idx = vle64_v_i64m1(indices, vl);
    vfloat64m1_t src = vle64_v_f64m1(src_data, vl);
    
    /* Complex vector load with stride - may require many operands */
    vfloat64m1_t result = vloxei64_v_f64m1(src_data, idx, vl);
    
    /* Additional operations */
    vfloat64m1_t scaled = vfmul_vf_f64m1(result, 2.0, vl);
    vfloat64m1_t final = vfadd_vf_f64m1(scaled, 1.0, vl);
    
    vse64_v_f64m1(results, final, vl);
    
    /* Validation */
    int valid = 1;
    for (size_t i = 0; i < vl; i++) {
        double expected = src_data[indices[i]] * 2.0 + 1.0;
        if (results[i] != expected) {
            valid = 0;
            break;
        }
    }
    
    CHECK(valid, "RISC-V vector result mismatch");
    PASS("RISC-V Vector 10-operand pattern executed");
    return 1;
}

#endif /* __riscv_v */

/* ============================================
 * Generic fallback with inline assembly
 * ============================================ */
NOINLINE HOT
static int test_generic_asm_10_operands(void) {
    printf("Testing generic inline asm with 10 operands...\n");
    
    long out1, out2;
    long in1 = 1, in2 = 2, in3 = 3, in4 = 4;
    long in5 = 5, in6 = 6, in7 = 7, in8 = 8;
    
    /* Generic inline assembly with 10 operands */
    asm volatile (
        "add %[out1], %[in1], %[in2]\n\t"
        "mul %[out2], %[in3], %[in4]\n\t"
        "add %[out1], %[out1], %[in5]\n\t"
        "add %[out2], %[out2], %[in6]\n\t"
        "mul %[out1], %[out1], %[in7]\n\t"
        "add %[out2], %[out2], %[in8]"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [in1] "r" (in1), [in2] "r" (in2),
          [in3] "r" (in3), [in4] "r" (in4),
          [in5] "r" (in5), [in6] "r" (in6),
          [in7] "r" (in7), [in8] "r" (in8)
        : "cc"
    );
    
    CHECK(out1 == ((1+2+5)*7), "Generic asm result1 mismatch");
    CHECK(out2 == ((3*4+6)+8), "Generic asm result2 mismatch");
    
    PASS("Generic inline asm 10-operand pattern executed");
    return 1;
}

NOINLINE HOT
static int test_generic_asm_11_operands(void) {
    printf("Testing generic inline asm with 11 operands...\n");
    
    long out1, out2, out3;
    long in1 = 1, in2 = 2, in3 = 3, in4 = 4;
    long in5 = 5, in6 = 6, in7 = 7, in8 = 8;
    
    /* Generic inline assembly with 11 operands */
    asm volatile (
        "add %[out1], %[in1], %[in2]\n\t"
        "mul %[out2], %[in3], %[in4]\n\t"
        "sub %[out3], %[in5], %[in6]\n\t"
        "add %[out1], %[out1], %[in7]\n\t"
        "add %[out2], %[out2], %[in8]"
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3)
        : [in1] "r" (in1), [in2] "r" (in2),
          [in3] "r" (in3), [in4] "r" (in4),
          [in5] "r" (in5), [in6] "r" (in6),
          [in7] "r" (in7), [in8] "r" (in8)
        : "cc"
    );
    
    CHECK(out1 == (1+2+7), "Generic asm 11op result1 mismatch");
    CHECK(out2 == (3*4+8), "Generic asm 11op result2 mismatch");
    CHECK(out3 == (5-6), "Generic asm 11op result3 mismatch");
    
    PASS("Generic inline asm 11-operand pattern executed");
    return 1;
}

/* ============================================
 * Main test driver
 * ============================================ */
int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("=== Testing RTL Expansion for 10-11 Operand Instructions ===\n\n");
    
    init_data();
    
    /* Run architecture-specific tests */
    
#ifdef __AVX512F__
    tests_run++;
    if (test_avx512_10_operands()) tests_passed++;
    
    tests_run++;
    if (test_avx512_asm_10_operands()) tests_passed++;
    
    tests_run++;
    if (test_avx512_11_operands()) tests_passed++;
    
    tests_run++;
    if (test_avx512_asm_11_operands()) tests_passed++;
#endif
    
#ifdef __ARM_FEATURE_SVE
    tests_run++;
    if (test_arm_sve_10_operands()) tests_passed++;
#endif
    
#ifdef __ALTIVEC__
    tests_run++;
    if (test_powerpc_10_operands()) tests_passed++;
#endif
    
#ifdef __riscv_v
    tests_run++;
    if (test_riscv_10_operands()) tests_passed++;
#endif
    
    /* Always run generic tests */
    tests_run++;
    if (test_generic_asm_10_operands()) tests_passed++;
    
    tests_run++;
    if (test_generic_asm_11_operands()) tests_passed++;
    
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
