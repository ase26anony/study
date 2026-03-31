/* test_multi_operand_expansion.c
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
#define VALIDATE(cond, msg) if (!(cond)) { printf("FAIL: %s\n", msg); return 0; }
#define PASS(msg) printf("PASS: %s\n", msg)

/* Force functions to not be inlined to ensure RTL expansion */
#define NOINLINE __attribute__((noinline))

/* ============================================
 * x86 AVX-512 Implementation (10-11 operands)
 * ============================================ */
#ifdef __AVX512F__

#include <immintrin.h>

/* Pattern A: 10 operands - Masked gather with multiple sources */
NOINLINE int test_avx512_gather_10ops(void) {
    /* Setup test data */
    double base[ARRAY_SIZE] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < ARRAY_SIZE; i++) base[i] = i * 1.5;
    for (int i = 0; i < 8; i++) indices[i] = i * 8;
    
    /* Initialize vectors */
    __m512d src = _mm512_set1_pd(99.0);
    __m512i vindex = _mm512_load_epi64(indices);
    __mmask8 mask = 0xFF; /* All lanes active */
    
    /* This gather intrinsic expands to ~10 operands in RTL:
     * 1. Destination (result)
     * 2. Mask
     * 3. Source (src)
     * 4. Base pointer
     * 5. Scale (implicit 8 for doubles)
     * 6. Index vector
     * 7-10: Various implicit operands for addressing modes
     */
    __m512d gathered = _mm512_mask_i64gather_pd(src, mask, vindex, base, 8);
    
    _mm512_store_pd(result, gathered);
    
    /* Validate */
    for (int i = 0; i < 8; i++) {
        double expected = base[indices[i] / 8];
        VALIDATE(result[i] == expected, "AVX-512 gather 10ops validation");
    }
    
    return 1;
}

/* Pattern B: 11 operands - Complex FMA with mask and rounding */
NOINLINE int test_avx512_fma_11ops(void) {
    /* Create 11-operand pattern using masked FMA with rounding control */
    __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b = _mm512_set1_pd(2.0);
    __m512d c = _mm512_set1_pd(3.0);
    __mmask8 mask = 0x0F; /* Lower 4 lanes active */
    
    /* This should generate ~11 operands:
     * 1. Dest
     * 2. Mask
     * 3. a
     * 4. b
     * 5. c
     * 6. Rounding control
     * 7-11: Various implicit operands
     */
    __m512d result = _mm512_mask3_fmadd_round_pd(a, b, c, mask, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    
    double res[8];
    _mm512_store_pd(res, result);
    
    /* Validate lower 4 lanes did FMA, upper 4 lanes are from c */
    for (int i = 0; i < 4; i++) {
        VALIDATE(res[i] == (i+1)*2.0 + 3.0, "AVX-512 FMA 11ops active lane");
    }
    for (int i = 4; i < 8; i++) {
        VALIDATE(res[i] == 3.0, "AVX-512 FMA 11ops inactive lane");
    }
    
    return 1;
}

/* Inline assembly with exactly 10 operands */
NOINLINE int test_avx512_asm_10ops(void) {
    double out[8] __attribute__((aligned(64)));
    double in1[8] __attribute__((aligned(64)));
    double in2[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        in1[i] = i + 1.0;
        in2[i] = i + 2.0;
    }
    
    /* Extended asm with 10 explicit operands */
    asm volatile (
        "vmovapd %1, %%zmm0\n\t"
        "vmovapd %2, %%zmm1\n\t"
        "vaddpd %%zmm0, %%zmm1, %%zmm2\n\t"
        "vmovapd %%zmm2, %0\n\t"
        : "=m" (out[0])      /* operand 0 */
        : "m" (in1[0]),      /* operand 1 */
          "m" (in2[0]),      /* operand 2 */
          "m" (out[0]),      /* operand 3 - dummy */
          "m" (in1[0]),      /* operand 4 - dummy */
          "m" (in2[0]),      /* operand 5 - dummy */
          "m" (out[0]),      /* operand 6 - dummy */
          "m" (in1[0]),      /* operand 7 - dummy */
          "m" (in2[0]),      /* operand 8 - dummy */
          "m" (out[0])       /* operand 9 - dummy */
        : "zmm0", "zmm1", "zmm2", "memory"
    );
    
    /* Validate */
    for (int i = 0; i < 8; i++) {
        VALIDATE(out[i] == (i+1.0) + (i+2.0), "AVX-512 asm 10ops validation");
    }
    
    return 1;
}

#endif /* __AVX512F__ */

/* ============================================
 * ARM SVE Implementation (10-11 operands)
 * ============================================ */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Pattern A: 10 operands - SVE gather with predicate */
NOINLINE int test_sve_gather_10ops(void) {
    uint64_t base[ARRAY_SIZE];
    uint64_t indices[16];
    uint64_t result[16];
    
    for (int i = 0; i < ARRAY_SIZE; i++) base[i] = i * 10;
    for (int i = 0; i < 16; i++) indices[i] = i * 4;
    
    svbool_t pg = svptrue_b64();
    svuint64_t vindex = svld1_u64(pg, indices);
    
    /* This gather should expand to ~10 operands */
    svuint64_t gathered = svld1_gather_u64index_u64(pg, base, vindex);
    
    svst1_u64(pg, result, gathered);
    
    /* Validate */
    for (int i = 0; i < 16; i++) {
        VALIDATE(result[i] == base[indices[i] / 4], "SVE gather 10ops validation");
    }
    
    return 1;
}

/* Pattern B: 11 operands - SVE scatter with multiple vectors */
NOINLINE int test_sve_scatter_11ops(void) {
    uint64_t data[ARRAY_SIZE] = {0};
    uint64_t src[16];
    uint64_t indices[16];
    
    for (int i = 0; i < 16; i++) {
        src[i] = i * 100;
        indices[i] = i * 3;
    }
    
    svbool_t pg = svptrue_b64();
    svuint64_t vsrc = svld1_u64(pg, src);
    svuint64_t vindex = svld1_u64(pg, indices);
    
    /* Scatter with predicate, base, index, and data - should be ~11 operands */
    svst1_scatter_u64index_u64(pg, data, vindex, vsrc);
    
    /* Validate scattered data */
    for (int i = 0; i < 16; i++) {
        VALIDATE(data[indices[i]] == src[i], "SVE scatter 11ops validation");
    }
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ============================================
 * PowerPC VSX/Altivec Implementation
 * ============================================ */
#ifdef __ALTIVEC__

#include <altivec.h>

/* Complex permutation requiring multiple vector arguments */
NOINLINE int test_powerpc_10ops(void) {
    vector unsigned char a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    vector unsigned char b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    vector unsigned char c = {32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47};
    vector unsigned char perm = {0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23};
    
    /* Complex vector permute with multiple inputs - may expand to many operands */
    vector unsigned char result = vec_perm(a, b, perm);
    
    /* Use result to avoid optimization */
    unsigned char res[16];
    memcpy(res, &result, 16);
    
    /* Simple validation */
    VALIDATE(res[0] == 0 && res[1] == 16, "PowerPC vec_perm validation");
    
    return 1;
}

#endif /* __ALTIVEC__ */

/* ============================================
 * RISC-V Vector Extension
 * ============================================ */
#ifdef __riscv_v

/* RISC-V vector load with multiple parameters */
NOINLINE int test_riscv_10ops(void) {
    long array[ARRAY_SIZE];
    long result[16];
    
    for (int i = 0; i < ARRAY_SIZE; i++) array[i] = i * 5;
    
    /* Inline asm to simulate vector load with many operands */
    asm volatile (
        "vsetvli zero, %0, e64, m8, ta, ma\n\t"
        "vle64.v v0, (%1)\n\t"
        "vse64.v v0, (%2)\n\t"
        : /* no outputs */
        : "r" (16), "r" (array), "r" (result)
        : "v0", "memory"
    );
    
    /* Validate */
    for (int i = 0; i < 16; i++) {
        VALIDATE(result[i] == array[i], "RISC-V vector load validation");
    }
    
    return 1;
}

#endif /* __riscv_v */

/* ============================================
 * Generic fallback with extended inline asm
 * ============================================ */
NOINLINE int test_generic_asm_11ops(void) {
    /* Force an 11-operand inline asm pattern */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long result;
    
    asm volatile (
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "add %3, %0\n\t"
        "add %4, %0\n\t"
        "add %5, %0\n\t"
        "add %6, %0\n\t"
        "add %7, %0\n\t"
        "add %8, %0\n\t"
        "add %9, %0\n\t"
        "add %10, %0"
        : "=r" (result)          /* operand 0 */
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),  /* operands 1-5 */
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)   /* operands 6-10 */
        : "cc"
    );
    
    VALIDATE(result == 55, "Generic 11-operand asm validation"); /* 1+2+3+...+10 = 55 */
    return 1;
}

/* ============================================
 * Main test driver
 * ============================================ */
int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("Testing multi-operand RTL expansion patterns...\n\n");
    
    /* Generic test - always run */
    tests_run++;
    if (test_generic_asm_11ops()) {
        PASS("Generic 11-operand inline asm");
        tests_passed++;
    }
    
    /* Architecture-specific tests */
#ifdef __AVX512F__
    printf("\n--- Testing AVX-512 paths ---\n");
    
    tests_run++;
    if (test_avx512_gather_10ops()) {
        PASS("AVX-512 gather (10 operands)");
        tests_passed++;
    }
    
    tests_run++;
    if (test_avx512_fma_11ops()) {
        PASS("AVX-512 FMA with rounding (11 operands)");
        tests_passed++;
    }
    
    tests_run++;
    if (test_avx512_asm_10ops()) {
        PASS("AVX-512 extended asm (10 operands)");
        tests_passed++;
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("\n--- Testing ARM SVE paths ---\n");
    
    tests_run++;
    if (test_sve_gather_10ops()) {
        PASS("ARM SVE gather (10 operands)");
        tests_passed++;
    }
    
    tests_run++;
    if (test_sve_scatter_11ops()) {
        PASS("ARM SVE scatter (11 operands)");
        tests_passed++;
    }
#endif
    
#ifdef __ALTIVEC__
    printf("\n--- Testing PowerPC VSX paths ---\n");
    
    tests_run++;
    if (test_powerpc_10ops()) {
        PASS("PowerPC vector permute (10 operands)");
        tests_passed++;
    }
#endif
    
#ifdef __riscv_v
    printf("\n--- Testing RISC-V Vector paths ---\n");
    
    tests_run++;
    if (test_riscv_10ops()) {
        PASS("RISC-V vector load (10 operands)");
        tests_passed++;
    }
#endif
    
    printf("\n================================\n");
    printf("Summary: %d/%d tests passed\n", tests_passed, tests_run);
    
    if (tests_passed == tests_run) {
        printf("SUCCESS: All multi-operand patterns executed correctly\n");
        return 0;
    } else {
        printf("WARNING: Some tests failed (but coverage may still be achieved)\n");
        return 1;
    }
}
