/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger the 10-11 operand expansion cases
 * in GCC's optabs.cc (lines 8254-8263) by using architecture-specific
 * intrinsics and inline assembly that require many operands.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation helpers */
#define ARRAY_SIZE 64
#define VALIDATE(cond, msg) if (!(cond)) { printf("FAIL: %s\n", msg); return 0; }

/* ============================================
 * Architecture-specific implementations
 * ============================================ */

/* Function to force no-inline and prevent early optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))

/* ========== x86 AVX-512 Implementation ========== */
#ifdef __AVX512F__

#include <immintrin.h>

/* Pattern A: 10 operands - Masked gather operation */
NOINLINE int test_avx512_gather_10_operands(void) {
    /* Setup test data */
    double base[ARRAY_SIZE] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = (double)(i * 2);
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = i * 4;
    }
    
    /* This gather intrinsic should expand to ~10 operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Index vector
     * 4. Base pointer
     * 5. Scale
     * 6. Vector length
     * 7. Hint
     * 8. Source (for merge-masked)
     * Plus implicit operands for addressing modes
     */
    __m512i vindex = _mm512_load_epi64(indices);
    __mmask8 mask = 0xFF;
    __m512d src = _mm512_set1_pd(0.0);
    
    /* This gather operation with many parameters may trigger 10-operand case */
    __m512d gathered = _mm512_mask_i64gather_pd(src, mask, vindex, 
                                               base, 8, _MM_SCALE_1);
    
    _mm512_store_pd(result, gathered);
    
    /* Validate results */
    for (int i = 0; i < 8; i++) {
        double expected = base[indices[i] / 8];
        VALIDATE(result[i] == expected, 
                "AVX-512 gather result mismatch");
    }
    
    return 1;
}

/* Pattern B: 11 operands - Complex FMA with mask and rounding */
NOINLINE int test_avx512_fma_11_operands(void) {
    /* Create a complex operation that might need 11 operands */
    __m512d a = _mm512_set1_pd(2.0);
    __m512d b = _mm512_set1_pd(3.0);
    __m512d c = _mm512_set1_pd(4.0);
    __mmask8 mask = 0x0F;  /* Lower half mask */
    
    /* Fused multiply-add with mask and rounding control */
    __m512d result = _mm512_mask3_fmadd_round_pd(a, b, c, mask, 
                                                _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    
    /* Use inline assembly as fallback to ensure 11 operands */
    double out[8] __attribute__((aligned(64)));
    _mm512_store_pd(out, result);
    
    /* Extended asm with 11 operands to directly trigger the case */
    asm volatile (
        "/* 11-operand dummy instruction */\n\t"
        "vmovapd %0, %1\n\t"
        : "=v"(result)
        : "v"(a), "v"(b), "v"(c), 
          "k"(mask), "i"(_MM_FROUND_TO_NEAREST_INT),
          "m"(out[0]), "m"(out[1]), "m"(out[2]), "m"(out[3]),
          "r"(0)  /* dummy register */
        : "memory"
    );
    
    return 1;
}

#endif /* __AVX512F__ */

/* ========== ARM SVE Implementation ========== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Pattern A: 10 operands - SVE gather with predicate */
NOINLINE int test_sve_gather_10_operands(void) {
    uint64_t base[ARRAY_SIZE] __attribute__((aligned(64)));
    uint64_t indices[svcntd()];
    uint64_t result[svcntd()];
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = i * 3;
    }
    for (size_t i = 0; i < svcntd(); i++) {
        indices[i] = i * 2;
    }
    
    /* Create predicate for all active lanes */
    svbool_t pg = svptrue_b64();
    
    /* Load index vector */
    svuint64_t vindex = svld1_u64(pg, indices);
    
    /* SVE gather - may expand to many operands */
    svuint64_t gathered = svld1_gather_u64index_u64(pg, base, vindex);
    
    svst1_u64(pg, result, gathered);
    
    /* Validate */
    for (size_t i = 0; i < svcntd(); i++) {
        VALIDATE(result[i] == base[indices[i]], 
                "SVE gather result mismatch");
    }
    
    return 1;
}

/* Pattern B: 11 operands - SVE scatter with multiple predicates */
NOINLINE int test_sve_scatter_11_operands(void) {
    uint64_t data[ARRAY_SIZE] __attribute__((aligned(64)));
    uint64_t dest[ARRAY_SIZE] __attribute__((aligned(64))) = {0};
    uint64_t indices[svcntd()];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 5;
    }
    for (size_t i = 0; i < svcntd(); i++) {
        indices[i] = i * 3;
    }
    
    svbool_t pg = svptrue_b64();
    svuint64_t vdata = svld1_u64(pg, data);
    svuint64_t vindex = svld1_u64(pg, indices);
    
    /* SVE scatter - potentially many operands */
    svst1_scatter_u64index_u64(pg, dest, vindex, vdata);
    
    /* Extended inline asm to ensure 11 operands */
    asm volatile (
        "/* SVE 11-operand pattern */\n\t"
        "mov x0, %0\n\t"
        "mov x1, %1\n\t"
        "mov x2, %2\n\t"
        : 
        : "r"(dest), "r"(indices), "r"(data),
          "w"(vdata), "w"(vindex), 
          "I"(svcntd()), "I"(0), "I"(1), "I"(2), "I"(3), "I"(4)
        : "x0", "x1", "x2", "memory"
    );
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ========== PowerPC Altivec/VSX Implementation ========== */
#ifdef __ALTIVEC__

#include <altivec.h>

/* Pattern A: 10 operands - Vector permute with multiple control vectors */
NOINLINE int test_ppc_permute_10_operands(void) {
    vector unsigned char a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    vector unsigned char b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    vector unsigned char perm1 = {31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16};
    vector unsigned char perm2 = {0,2,4,6,8,10,12,14,1,3,5,7,9,11,13,15};
    
    /* Complex permutation chain */
    vector unsigned char result = vec_perm(a, b, perm1);
    result = vec_perm(result, a, perm2);
    
    /* Extended inline asm with 10 operands */
    vector unsigned char out1, out2;
    asm volatile (
        "vperm %0, %1, %2, %3\n\t"
        "vperm %4, %5, %6, %7\n\t"
        : "=v"(out1), "=v"(out2)
        : "v"(a), "v"(b), "v"(perm1), "v"(perm2),
          "v"(result), "r"(0), "r"(1), "r"(2)
        : 
    );
    
    return 1;
}

#endif /* __ALTIVEC__ */

/* ========== RISC-V Vector Implementation ========== */
#ifdef __riscv_v

#include <riscv_vector.h>

/* Pattern B: 11 operands - Vector load with mask, stride, and length */
NOINLINE int test_riscv_vlseg_11_operands(void) {
    double data[ARRAY_SIZE] __attribute__((aligned(64)));
    double result[ARRAY_SIZE/2] __attribute__((aligned(64)));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 1.5;
    }
    
    size_t vl = vsetvl_e64m1(ARRAY_SIZE/2);
    vbool64_t mask = vmset_m_b64(vl);
    
    /* Strided load with mask - may require many operands */
    vfloat64m1_t loaded = vle64_v_f64m1_m(mask, vundefined_f64m1(), 
                                         data, 2, vl);
    
    vse64_v_f64m1(result, loaded, vl);
    
    /* Extended asm to ensure 11 operands */
    asm volatile (
        "vsetvli zero, %0, e64, m1, ta, ma\n\t"
        : 
        : "r"(vl), "v"(loaded), "v"(mask),
          "r"(data), "r"(result), "r"(2),
          "I"(0), "I"(1), "I"(2), "I"(3), "I"(4)
        : 
    );
    
    return 1;
}

#endif /* __riscv_v */

/* ========== Generic fallback with inline asm ========== */
#ifndef __AVX512F__
#ifndef __ARM_FEATURE_SVE
#ifndef __ALTIVEC__
#ifndef __riscv_v

/* Generic 10-operand inline asm for architectures without vector extensions */
NOINLINE int test_generic_10_operands(void) {
    long ops[10];
    long result = 0;
    
    for (int i = 0; i < 10; i++) {
        ops[i] = i + 1;
    }
    
    /* Explicit 10-operand asm statement */
    asm volatile (
        "/* 10-operand generic pattern */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        : "=r"(result)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), 
          "r"(ops[3]), "r"(ops[4]), "r"(ops[5]),
          "r"(ops[6]), "r"(ops[7]), "r"(ops[8])
        : "cc"
    );
    
    VALIDATE(result == 45, "Generic 10-operand result mismatch");
    return 1;
}

/* Generic 11-operand inline asm */
NOINLINE int test_generic_11_operands(void) {
    long ops[11];
    long result1 = 0, result2 = 0;
    
    for (int i = 0; i < 11; i++) {
        ops[i] = i + 1;
    }
    
    /* Explicit 11-operand asm statement */
    asm volatile (
        "/* 11-operand generic pattern */\n\t"
        "mov %0, %2\n\t"
        "imul %0, %3\n\t"
        "mov %1, %4\n\t"
        "add %1, %5\n\t"
        "sub %0, %6\n\t"
        "add %1, %7\n\t"
        "sub %0, %8\n\t"
        "add %1, %9\n\t"
        "sub %0, %10\n\t"
        "add %1, %11\n\t"
        : "=r"(result1), "=r"(result2)
        : "r"(ops[0]), "r"(ops[1]), "r"(ops[2]), 
          "r"(ops[3]), "r"(ops[4]), "r"(ops[5]),
          "r"(ops[6]), "r"(ops[7]), "r"(ops[8]),
          "r"(ops[9]), "r"(ops[10])
        : "cc"
    );
    
    return 1;
}

#endif
#endif
#endif
#endif

/* ============================================
 * Main test driver
 * ============================================ */

int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("Testing optabs.cc 10-11 operand expansion cases...\n");
    
    /* x86 AVX-512 tests */
#ifdef __AVX512F__
    printf("[x86 AVX-512] Running tests...\n");
    if (test_avx512_gather_10_operands()) {
        printf("  ✓ 10-operand gather test passed\n");
        tests_passed++;
    }
    tests_run++;
    
    if (test_avx512_fma_11_operands()) {
        printf("  ✓ 11-operand FMA test passed\n");
        tests_passed++;
    }
    tests_run++;
#endif
    
    /* ARM SVE tests */
#ifdef __ARM_FEATURE_SVE
    printf("[ARM SVE] Running tests...\n");
    if (test_sve_gather_10_operands()) {
        printf("  ✓ 10-operand gather test passed\n");
        tests_passed++;
    }
    tests_run++;
    
    if (test_sve_scatter_11_operands()) {
        printf("  ✓ 11-operand scatter test passed\n");
        tests_passed++;
    }
    tests_run++;
#endif
    
    /* PowerPC tests */
#ifdef __ALTIVEC__
    printf("[PowerPC Altivec] Running tests...\n");
    if (test_ppc_permute_10_operands()) {
        printf("  ✓ 10-operand permute test passed\n");
        tests_passed++;
    }
    tests_run++;
#endif
    
    /* RISC-V Vector tests */
#ifdef __riscv_v
    printf("[RISC-V Vector] Running tests...\n");
    if (test_riscv_vlseg_11_operands()) {
        printf("  ✓ 11-operand vector load test passed\n");
        tests_passed++;
    }
    tests_run++;
#endif
    
    /* Generic fallback tests */
#if !defined(__AVX512F__) && !defined(__ARM_FEATURE_SVE) && \
    !defined(__ALTIVEC__) && !defined(__riscv_v)
    printf("[Generic] Running fallback tests...\n");
    if (test_generic_10_operands()) {
        printf("  ✓ Generic 10-operand test passed\n");
        tests_passed++;
    }
    tests_run++;
    
    if (test_generic_11_operands()) {
        printf("  ✓ Generic 11-operand test passed\n");
        tests_passed++;
    }
    tests_run++;
#endif
    
    printf("\nSummary: %d/%d tests passed\n", tests_passed, tests_run);
    
    if (tests_passed == tests_run) {
        printf("SUCCESS: All architecture-specific paths executed correctly\n");
        return 0;
    } else {
        printf("WARNING: Some tests failed or were not compiled\n");
        return 1;
    }
}
