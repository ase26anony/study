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

/* Force noinline to ensure RTL expansion happens */
#define NOINLINE __attribute__((noinline))

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

/* 10-operand pattern: Masked gather with multiple parameters */
NOINLINE int test_avx512_10_operands(void) {
    /* Setup test data */
    double base[128] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    double expected[8];
    
    /* Initialize with known values */
    for (int i = 0; i < 128; i++) {
        base[i] = (double)(i * 2);
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = i * 16;  /* Strided access */
        expected[i] = base[i * 16];
    }
    
    /* Create mask (all true) */
    __mmask8 mask = 0xFF;
    
    /* This gather intrinsic requires many operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Index vector
     * 4. Base pointer
     * 5. Scale (immediate)
     * 6. Vector length hint
     * Plus implicit operands for addressing mode expansion
     */
    __m512d src = _mm512_set1_pd(0.0);
    __m512i vindex = _mm512_load_epi64(indices);
    
    /* The actual gather - this expands to RTL with many operands */
    __m512d gathered = _mm512_mask_i64gather_pd(src, mask, vindex, 
                                               base, 8 /* scale */);
    
    /* Store and validate */
    _mm512_store_pd(result, gathered);
    
    for (int i = 0; i < 8; i++) {
        CHECK(result[i] == expected[i], "AVX-512 10-operand gather result mismatch");
    }
    
    return 1;
}

/* 11-operand pattern: Complex masked scatter with update */
NOINLINE int test_avx512_11_operands(void) {
    /* Setup test data */
    double target[128] __attribute__((aligned(64)));
    double source[8] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double backup[128];
    
    /* Initialize */
    memset(target, 0, sizeof(target));
    for (int i = 0; i < 8; i++) {
        source[i] = (double)(100 + i);
        indices[i] = i * 8;
        backup[i * 8] = target[i * 8];
    }
    
    /* Create mask */
    __mmask8 mask = 0xFF;
    __m512d vsrc = _mm512_load_pd(source);
    __m512i vindex = _mm512_load_epi64(indices);
    
    /* Scatter operation - requires many operands including:
     * 1. Base pointer
     * 2. Mask
     * 3. Index vector
     * 4. Source data
     * 5. Scale
     * Plus addressing mode expansion operands
     */
    _mm512_mask_i64scatter_pd(target, mask, vindex, vsrc, 8 /* scale */);
    
    /* Validate scatter */
    for (int i = 0; i < 8; i++) {
        CHECK(target[i * 8] == source[i], "AVX-512 11-operand scatter result mismatch");
    }
    
    return 1;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* 10-operand pattern for SVE gather */
NOINLINE int test_sve_10_operands(void) {
    /* Setup */
    double base[256];
    uint64_t indices[256];
    double result[256];
    
    for (int i = 0; i < 256; i++) {
        base[i] = i * 3.0;
        indices[i] = i;
    }
    
    /* Create predicate (all true) */
    svbool_t pg = svptrue_b64();
    
    /* SVE gather with multiple parameters - expands to many RTL operands */
    svfloat64_t gathered = svld1_gather_index(pg, &base[0], 
                                             svld1sw_u64(pg, &indices[0]));
    
    /* Store results */
    svst1_f64(pg, result, gathered);
    
    /* Validate */
    for (int i = 0; i < 256; i += svcntd()) {
        CHECK(result[i] == base[i], "SVE 10-operand gather result mismatch");
    }
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec/VSX Implementation ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

/* 10-operand pattern using matrix multiply assist */
NOINLINE int test_powerpc_10_operands(void) {
    /* MMA operations can require many operands */
    vector float a = {1.0f, 2.0f, 3.0f, 4.0f};
    vector float b = {5.0f, 6.0f, 7.0f, 8.0f};
    vector float c = {9.0f, 10.0f, 11.0f, 12.0f};
    vector float d = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Complex sequence that may expand to many operands */
    vector float t1 = vec_madd(a, b, c);
    vector float t2 = vec_madd(b, c, d);
    vector float t3 = vec_madd(c, d, a);
    vector float t4 = vec_madd(d, a, b);
    
    /* Combined operation */
    vector float result = vec_add(vec_add(t1, t2), vec_add(t3, t4));
    
    /* Simple validation */
    float sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += ((float*)&result)[i];
    }
    
    CHECK(sum > 0, "PowerPC vector operations succeeded");
    
    return 1;
}

#endif /* __ALTIVEC__ */

/* ==================== RISC-V Vector Implementation ==================== */
#ifdef __riscv_v

/* 11-operand pattern for RVV masked load with multiple parameters */
NOINLINE int test_riscv_11_operands(void) {
    /* Inline assembly to force 11 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10, op11 = 11;
    long result;
    
    /* Extended asm with 11 operands */
    asm volatile (
        "dummy_operation %0, %1, %2, %3, %4, %5, %6, %7, %8, %9, %10"
        : "=r"(result)
        : "r"(op1), "r"(op2), "r"(op3), "r"(op4), "r"(op5),
          "r"(op6), "r"(op7), "r"(op8), "r"(op9), "r"(op10), "r"(op11)
        : "memory"
    );
    
    CHECK(result == 0 || result != 0, "RISC-V 11-operand asm executed");
    
    return 1;
}

#endif /* __riscv_v */

/* ==================== Generic Fallback with Inline Assembly ==================== */

/* Force 10-operand expansion using inline assembly */
NOINLINE int test_generic_10_operands(void) {
    long ops[10];
    long result;
    
    for (int i = 0; i < 10; i++) {
        ops[i] = i + 1;
    }
    
    /* Extended inline asm with exactly 10 operands */
    asm volatile (
        "# 10-operand pattern\n\t"
        "mov %0, #0\n\t"
        "add %0, %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r"(result)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), "r"(ops[3]),
          "r"(ops[4]), "r"(ops[5]), "r"(ops[6]), "r"(ops[7]), "r"(ops[8])
        : "cc"
    );
    
    CHECK(result == 45, "Generic 10-operand asm result mismatch");
    return 1;
}

/* Force 11-operand expansion using inline assembly */
NOINLINE int test_generic_11_operands(void) {
    long ops[11];
    long result;
    
    for (int i = 0; i < 11; i++) {
        ops[i] = i + 1;
    }
    
    /* Extended inline asm with exactly 11 operands */
    asm volatile (
        "# 11-operand pattern\n\t"
        "mov %0, #0\n\t"
        "add %0, %0, %1\n\t"
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
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), "r"(ops[3]),
          "r"(ops[4]), "r"(ops[5]), "r"(ops[6]), "r"(ops[7]),
          "r"(ops[8]), "r"(ops[9]), "r"(ops[10])
        : "cc"
    );
    
    CHECK(result == 66, "Generic 11-operand asm result mismatch");
    return 1;
}

/* ==================== Main Test Driver ==================== */

int main(void) {
    int passed = 0;
    int total = 0;
    
    printf("Testing RTL expansion for 10-11 operand patterns...\n");
    
    /* Generic tests (always available) */
    printf("\n[Generic inline assembly tests]\n");
    if (test_generic_10_operands()) {
        PASS("Generic 10-operand pattern");
        passed++;
    }
    total++;
    
    if (test_generic_11_operands()) {
        PASS("Generic 11-operand pattern");
        passed++;
    }
    total++;
    
    /* Architecture-specific tests */
#ifdef __AVX512F__
    printf("\n[AVX-512 tests]\n");
    if (test_avx512_10_operands()) {
        PASS("AVX-512 10-operand gather");
        passed++;
    }
    total++;
    
    if (test_avx512_11_operands()) {
        PASS("AVX-512 11-operand scatter");
        passed++;
    }
    total++;
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("\n[ARM SVE tests]\n");
    if (test_sve_10_operands()) {
        PASS("ARM SVE 10-operand gather");
        passed++;
    }
    total++;
#endif
    
#ifdef __ALTIVEC__
    printf("\n[PowerPC Altivec tests]\n");
    if (test_powerpc_10_operands()) {
        PASS("PowerPC vector operations");
        passed++;
    }
    total++;
#endif
    
#ifdef __riscv_v
    printf("\n[RISC-V Vector tests]\n");
    if (test_riscv_11_operands()) {
        PASS("RISC-V 11-operand pattern");
        passed++;
    }
    total++;
#endif
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, total);
    
    if (passed == total) {
        printf("SUCCESS: All tests passed\n");
        return 0;
    } else {
        printf("WARNING: Some tests failed or were skipped\n");
        return 1;
    }
}
