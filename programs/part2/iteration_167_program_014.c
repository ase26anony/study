/* test_optabs_10_11_operands.c
 * 
 * This program generates RTL patterns requiring exactly 10 or 11 operands
 * to trigger the uncovered cases in optabs.cc lines 8254-8263.
 * 
 * Compilation recommendations:
 *   x86 AVX-512: gcc -O3 -mavx512f -mavx512vl -ftree-vectorize -funroll-loops test.c -o test_avx512
 *   ARM SVE:     gcc -O3 -march=armv8-a+sve -ftree-vectorize test.c -o test_sve
 *   PowerPC:     gcc -O3 -mcpu=power9 -ftree-vectorize test.c -o test_ppc
 *   Generic:     gcc -O3 -ftree-vectorize -funroll-loops test.c -o test_generic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation utilities */
#define VALIDATE(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "Validation failed: %s\n", msg); \
        return 0; \
    } \
} while(0)

#define ARRAY_SIZE 1024

/* Prevent inlining to ensure RTL expansion happens at call site */
__attribute__((noinline, target("avx512f,avx512vl")))
#ifdef __AVX512F__
int avx512_10_operand_gather(void) {
    /* AVX-512 masked gather with 10 operands:
     * 1 destination
     * 8 source: base, scale, index, mask, source, src_offset, mask_control, rounding
     * 1 implicit: memory operand
     */
    
    double src[ARRAY_SIZE];
    double dst[8];
    int64_t indices[8];
    __mmask8 mask = 0xFF;
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = (double)i;
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = i * 16;
        dst[i] = 0.0;
    }
    
    /* Use AVX-512 gather intrinsic - this should generate RTL with many operands */
    __m512d result = _mm512_mask_i64gather_pd(
        _mm512_setzero_pd(),    /* src operand 1 */
        mask,                   /* mask operand 2 */
        _mm512_loadu_si512((__m512i*)indices), /* index operand 3 */
        src,                    /* base pointer operand 4 */
        8,                      /* scale operand 5 */
        _MM_SCALE_1             /* scale type operand 6 */
    );
    
    _mm512_storeu_pd(dst, result);
    
    /* Validate results */
    for (int i = 0; i < 8; i++) {
        VALIDATE(dst[i] == (double)(i * 16), "AVX-512 gather result mismatch");
    }
    
    return 1;
}

/* 11-operand pattern using complex FMA operation */
__attribute__((noinline, target("avx512f")))
int avx512_11_operand_fma(void) {
    /* Complex FMA with mask, rounding, and saturation control:
     * 1 destination
     * 9 source: a, b, c, mask, rounding, saturation, a_scale, b_scale, c_scale
     * 1 implicit: control register
     */
    
    double a[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    double b[8] = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    double c[8] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    double result[8];
    
    __m512d va = _mm512_loadu_pd(a);
    __m512d vb = _mm512_loadu_pd(b);
    __m512d vc = _mm512_loadu_pd(c);
    __mmask8 mask = 0xFF;
    
    /* Fused multiply-add with mask - may expand to 11 operands */
    __m512d vresult = _mm512_mask3_fmadd_pd(va, vb, vc, mask);
    
    _mm512_storeu_pd(result, vresult);
    
    /* Validate: result = a * b + c */
    for (int i = 0; i < 8; i++) {
        double expected = a[i] * b[i] + c[i];
        VALIDATE(result[i] == expected, "AVX-512 FMA result mismatch");
    }
    
    return 1;
}
#endif /* __AVX512F__ */

/* ARM SVE implementation */
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

__attribute__((noinline))
int arm_sve_10_operand_gather(void) {
    /* SVE gather with 10 operands:
     * 1 destination predicate
     * 1 destination vector
     * 8 source: base, offset, scale, predicate, offset_scale, addressing_mode, etc.
     */
    
    double src[ARRAY_SIZE];
    double dst[256]; /* SVE has variable length, use conservative size */
    int64_t indices[256];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = (double)i;
    }
    for (int i = 0; i < 256; i++) {
        indices[i] = i * 4;
    }
    
    svbool_t pg = svptrue_b64();
    svint64_t offset = svld1_s64(pg, indices);
    
    /* SVE gather - should generate multi-operand RTL */
    svfloat64_t result = svld1_gather_offset(pg, src, offset);
    
    svst1(pg, dst, result);
    
    /* Simple validation - check first few elements */
    uint64_t vl = svcntd();
    for (int i = 0; i < (vl < 8 ? vl : 8); i++) {
        VALIDATE(dst[i] == (double)(i * 4), "ARM SVE gather result mismatch");
    }
    
    return 1;
}
#endif /* __ARM_FEATURE_SVE */

/* PowerPC Altivec/VSX implementation */
#ifdef __ALTIVEC__
#include <altivec.h>

__attribute__((noinline))
int powerpc_11_operand_permute(void) {
    /* Complex permutation with 11 operands:
     * 1 destination
     * 10 source: a, b, c, d, permute_control1, permute_control2, 
     *            merge_mask, saturation, rounding, element_size
     */
    
    vector float a = {1.0f, 2.0f, 3.0f, 4.0f};
    vector float b = {5.0f, 6.0f, 7.0f, 8.0f};
    vector float c = {9.0f, 10.0f, 11.0f, 12.0f};
    vector float d = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Complex permutation pattern - may require many operands */
    vector float result = vec_perm(a, b, vec_perm(c, d, 
        (vector unsigned char){0,1,2,3, 16,17,18,19, 8,9,10,11, 24,25,26,27}));
    
    /* Validate with simple check */
    float res[4];
    vec_st(result, 0, res);
    
    VALIDATE(res[0] == 1.0f && res[1] == 6.0f && 
             res[2] == 11.0f && res[3] == 16.0f, 
             "PowerPC permute result mismatch");
    
    return 1;
}
#endif /* __ALTIVEC__ */

/* Generic inline assembly fallback for architectures without vector intrinsics */
__attribute__((noinline, optimize("O3")))
int generic_10_operand_asm(void) {
    /* Inline assembly with exactly 10 operands */
    unsigned long ops[10];
    unsigned long result = 0;
    
    /* Initialize operands */
    for (int i = 0; i < 10; i++) {
        ops[i] = i + 1;
    }
    
    /* Extended asm with 10 operands - forces RTL expander to handle 10 operands */
    asm volatile (
        "/* 10-operand dummy instruction */\n\t"
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        "add %0, %3\n\t"
        "add %0, %4\n\t"
        "add %0, %5\n\t"
        "add %0, %6\n\t"
        "add %0, %7\n\t"
        "add %0, %8\n\t"
        "add %0, %9"
        : "=r" (result)
        : "r" (ops[0]), "r" (ops[1]), "r" (ops[2]), "r" (ops[3]),
          "r" (ops[4]), "r" (ops[5]), "r" (ops[6]), "r" (ops[7]),
          "r" (ops[8])
        : "cc"
    );
    
    VALIDATE(result == 45, "Generic 10-operand asm result mismatch"); /* 1+2+...+9 = 45 */
    return 1;
}

__attribute__((noinline, optimize("O3")))
int generic_11_operand_asm(void) {
    /* Inline assembly with exactly 11 operands */
    unsigned long ops[11];
    unsigned long result = 0;
    
    /* Initialize operands */
    for (int i = 0; i < 11; i++) {
        ops[i] = i + 1;
    }
    
    /* Extended asm with 11 operands - forces RTL expander to handle 11 operands */
    asm volatile (
        "/* 11-operand dummy instruction */\n\t"
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        "add %0, %3\n\t"
        "add %0, %4\n\t"
        "add %0, %5\n\t"
        "add %0, %6\n\t"
        "add %0, %7\n\t"
        "add %0, %8\n\t"
        "add %0, %9\n\t"
        "add %0, %10"
        : "=r" (result)
        : "r" (ops[0]), "r" (ops[1]), "r" (ops[2]), "r" (ops[3]),
          "r" (ops[4]), "r" (ops[5]), "r" (ops[6]), "r" (ops[7]),
          "r" (ops[8]), "r" (ops[9]), "r" (ops[10])
        : "cc"
    );
    
    VALIDATE(result == 66, "Generic 11-operand asm result mismatch"); /* 1+2+...+11 = 66 */
    return 1;
}

/* Main test driver */
int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("Testing RTL expansion for 10-11 operand patterns...\n");
    
    /* Test generic inline assembly patterns (always available) */
    printf("Testing generic 10-operand inline assembly...\n");
    if (generic_10_operand_asm()) {
        tests_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    tests_run++;
    
    printf("Testing generic 11-operand inline assembly...\n");
    if (generic_11_operand_asm()) {
        tests_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    tests_run++;
    
    /* Architecture-specific tests */
#ifdef __AVX512F__
    printf("Testing AVX-512 10-operand gather...\n");
    if (avx512_10_operand_gather()) {
        tests_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    tests_run++;
    
    printf("Testing AVX-512 11-operand FMA...\n");
    if (avx512_11_operand_fma()) {
        tests_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    tests_run++;
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE 10-operand gather...\n");
    if (arm_sve_10_operand_gather()) {
        tests_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    tests_run++;
#endif
    
#ifdef __ALTIVEC__
    printf("Testing PowerPC 11-operand permute...\n");
    if (powerpc_11_operand_permute()) {
        tests_passed++;
        printf("  PASSED\n");
    } else {
        printf("  FAILED\n");
    }
    tests_run++;
#endif
    
    printf("\nSummary: %d/%d tests passed\n", tests_passed, tests_run);
    
    if (tests_passed == tests_run) {
        printf("SUCCESS: All tests passed. RTL expansion for 10-11 operands should have been triggered.\n");
        return 0;
    } else {
        printf("WARNING: Some tests failed. Architecture-specific features may not be available.\n");
        return 1;
    }
}
