/* test_multi_operand_expansion.c
 * 
 * This program generates RTL patterns requiring 10-11 operands to trigger
 * specific uncovered code paths in GCC's optabs.cc (lines 8254-8263).
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

#define ARRAY_SIZE 64

/* ==================== x86 AVX-512 Implementation ==================== */
#ifdef __AVX512F__

#include <immintrin.h>

__attribute__((noinline, target("avx512f,avx512vl")))
int test_avx512_10_operands(void) {
    /* Pattern A: 10 operands - masked gather with multiple parameters */
    double src[ARRAY_SIZE];
    double dst[ARRAY_SIZE];
    int64_t indices[ARRAY_SIZE];
    __mmask8 mask = 0xFF;
    
    /* Initialize test data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = (double)i * 1.5;
        indices[i] = (i * 2) % ARRAY_SIZE;
        dst[i] = 0.0;
    }
    
    /* This gather intrinsic typically expands to many operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Index vector
     * 4. Base pointer
     * 5. Scale
     * 6. Vector length
     * 7. Source (for merge-masked)
     * Plus additional implicit operands for addressing modes
     */
    __m512d result = _mm512_mask_i64gather_pd(
        _mm512_setzero_pd(),    /* src operand for merge */
        mask,                   /* mask */
        _mm512_loadu_si512((__m512i*)indices), /* indices */
        src,                    /* base address */
        8,                      /* scale (sizeof(double)) */
        _MM_SCALE_1             /* scale enum */
    );
    
    _mm512_storeu_pd(dst, result);
    
    /* Validate */
    double checksum = 0.0;
    for (int i = 0; i < 8; i++) {
        if (mask & (1 << i)) {
            int idx = indices[i];
            checksum += src[idx];
        }
    }
    
    double result_sum = 0.0;
    for (int i = 0; i < 8; i++) {
        result_sum += dst[i];
    }
    
    VALIDATE(fabs(result_sum - checksum) < 1e-10, 
             "AVX-512 10-operand gather failed");
    
    return 1;
}

__attribute__((noinline, target("avx512f")))
int test_avx512_11_operands(void) {
    /* Pattern B: 11 operands - complex masked scatter with update */
    double data[ARRAY_SIZE * 2];
    double updates[ARRAY_SIZE];
    int64_t scatter_indices[ARRAY_SIZE];
    __mmask16 mask = 0xAAAA;  /* Alternating pattern */
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)i;
        updates[i] = (double)i * 2.0;
        scatter_indices[i] = (i * 3) % ARRAY_SIZE;
    }
    
    /* Scatter operation with mask, base, index, scale, and update source
     * This often requires 11 operands when fully expanded:
     * 1. Base address
     * 2. Mask
     * 3. Index vector
     * 4. Source data vector
     * 5. Scale
     * 6. Displacement
     * 7. Broadcast control
     * 8. Rounding mode
     * 9. Exception control
     * 10. Memory type
     * 11. Alignment hint
     */
    _mm512_mask_i64scatter_pd(
        data,                   /* base address */
        mask,                   /* mask */
        _mm512_loadu_si512((__m512i*)scatter_indices), /* indices */
        _mm512_loadu_pd(updates), /* source data */
        8,                      /* scale */
        _MM_SCALE_1
    );
    
    /* Validate scattered values */
    int errors = 0;
    for (int i = 0; i < 16; i++) {
        if (mask & (1 << i)) {
            int idx = scatter_indices[i];
            if (fabs(data[idx] - updates[i]) > 1e-10) {
                errors++;
            }
        }
    }
    
    VALIDATE(errors == 0, "AVX-512 11-operand scatter failed");
    
    return 1;
}

#endif /* __AVX512F__ */

/* ==================== ARM SVE Implementation ==================== */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

__attribute__((noinline))
int test_arm_sve_10_operands(void) {
    /* SVE gather with predicate, base, and offsets - often expands to 10 ops */
    double src[ARRAY_SIZE];
    double dst[ARRAY_SIZE];
    int64_t indices[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = (double)i * 2.0;
        indices[i] = i;
        dst[i] = 0.0;
    }
    
    svbool_t pg = svwhilelt_b64(0, ARRAY_SIZE);
    svint64_t offset_vec = svld1_s64(pg, indices);
    
    /* SVE gather intrinsic with multiple parameters */
    svfloat64_t gathered = svld1_gather_offset(pg, src, offset_vec);
    
    svst1(pg, dst, gathered);
    
    /* Validate */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (svptest_any(svptrue_b64(), pg)) {
            checksum += src[i];
        }
    }
    
    double result_sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        result_sum += dst[i];
    }
    
    VALIDATE(fabs(result_sum - checksum) < 1e-10, 
             "ARM SVE 10-operand gather failed");
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ==================== PowerPC Altivec/VSX Implementation ==================== */
#ifdef __ALTIVEC__

#include <altivec.h>

__attribute__((noinline))
int test_powerpc_11_operands(void) {
    /* Complex vector permutation with multiple arguments */
    vector float a = {1.0f, 2.0f, 3.0f, 4.0f};
    vector float b = {5.0f, 6.0f, 7.0f, 8.0f};
    vector float c = {9.0f, 10.0f, 11.0f, 12.0f};
    vector float d = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Complex operation using inline assembly to force 11 operands */
    vector float result;
    
    /* Extended asm with 11 operands: 4 inputs, 1 output, 6 clobbers */
    __asm__ volatile (
        "xxpermdi %x0, %x1, %x2, 0   \n\t"
        "xxpermdi %x0, %x0, %x3, 1   \n\t"
        "xxpermdi %x0, %x0, %x4, 2   \n\t"
        : "=wa" (result)
        : "wa" (a), "wa" (b), "wa" (c), "wa" (d)
        : "v0", "v1", "v2", "v3", "v4", "v5"
    );
    
    /* Validate by checking known pattern */
    float *res = (float*)&result;
    VALIDATE(res[0] == 1.0f && res[1] == 5.0f, 
             "PowerPC 11-operand permutation failed");
    
    return 1;
}

#endif /* __ALTIVEC__ */

/* ==================== RISC-V Vector Extension ==================== */
#ifdef __riscv_v

#include <riscv_vector.h>

__attribute__((noinline))
int test_riscv_10_operands(void) {
    /* RISC-V vector load with multiple parameters */
    double data[ARRAY_SIZE];
    double dst[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)i * 3.0;
        dst[i] = 0.0;
    }
    
    size_t vl = vsetvl_e64m8(ARRAY_SIZE);
    vbool8_t mask = vmseq_vx_u64m8_b8(vle64_v_u64m8((uint64_t*)data, vl), 0, vl);
    
    /* Vector load with mask, stride, and length - can expand to 10 operands */
    vfloat64m8_t loaded = vle64_v_f64m8_m(
        mask,                    /* mask */
        dst,                     /* destination (merge operand) */
        data,                    /* base address */
        vl                       /* vector length */
    );
    
    /* Store result */
    vse64_v_f64m8(dst, loaded, vl);
    
    /* Validate */
    double sum = 0.0;
    for (int i = 0; i < vl; i++) {
        sum += data[i];
    }
    
    double result_sum = 0.0;
    for (int i = 0; i < vl; i++) {
        result_sum += dst[i];
    }
    
    VALIDATE(fabs(result_sum - sum) < 1e-10, 
             "RISC-V 10-operand vector load failed");
    
    return 1;
}

#endif /* __riscv_v */

/* ==================== Generic Inline Assembly Fallback ==================== */
#ifndef __AVX512F__
#ifndef __ARM_FEATURE_SVE
#ifndef __ALTIVEC__
#ifndef __riscv_v

/* Fallback using generic inline assembly with exactly 10 and 11 operands */
__attribute__((noinline))
int test_generic_10_operands(void) {
    /* Inline assembly with exactly 10 explicit operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long result;
    
    __asm__ volatile (
        "/* 10-operand dummy instruction */   \n\t"
        "add %0, %1, %2                       \n\t"
        "add %0, %0, %3                       \n\t"
        "add %0, %0, %4                       \n\t"
        "add %0, %0, %5                       \n\t"
        "add %0, %0, %6                       \n\t"
        "add %0, %0, %7                       \n\t"
        "add %0, %0, %8                       \n\t"
        "add %0, %0, %9                       \n\t"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), 
          "r" (e), "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    VALIDATE(result == 45, "Generic 10-operand assembly failed");
    return 1;
}

__attribute__((noinline))
int test_generic_11_operands(void) {
    /* Inline assembly with exactly 11 explicit operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    long result;
    
    __asm__ volatile (
        "/* 11-operand dummy instruction */   \n\t"
        "add %0, %1, %2                       \n\t"
        "add %0, %0, %3                       \n\t"
        "add %0, %0, %4                       \n\t"
        "add %0, %0, %5                       \n\t"
        "add %0, %0, %6                       \n\t"
        "add %0, %0, %7                       \n\t"
        "add %0, %0, %8                       \n\t"
        "add %0, %0, %9                       \n\t"
        "add %0, %0, %10                      \n\t"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), 
          "r" (e), "r" (f), "r" (g), "r" (h), 
          "r" (i), "r" (j)
        : "cc"
    );
    
    VALIDATE(result == 56, "Generic 11-operand assembly failed");
    return 1;
}

#endif
#endif
#endif
#endif

/* ==================== Main Test Driver ==================== */
int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("Testing multi-operand RTL expansion patterns...\n");
    printf("Target: optabs.cc lines 8254-8263 (10-11 operand cases)\n\n");
    
    /* Run architecture-specific tests */
    
#ifdef __AVX512F__
    printf("Testing x86 AVX-512 paths...\n");
    tests_run++;
    if (test_avx512_10_operands()) {
        printf("  ✓ AVX-512 10-operand pattern passed\n");
        tests_passed++;
    }
    
    tests_run++;
    if (test_avx512_11_operands()) {
        printf("  ✓ AVX-512 11-operand pattern passed\n");
        tests_passed++;
    }
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("Testing ARM SVE paths...\n");
    tests_run++;
    if (test_arm_sve_10_operands()) {
        printf("  ✓ ARM SVE 10-operand pattern passed\n");
        tests_passed++;
    }
#endif
    
#ifdef __ALTIVEC__
    printf("Testing PowerPC Altivec/VSX paths...\n");
    tests_run++;
    if (test_powerpc_11_operands()) {
        printf("  ✓ PowerPC 11-operand pattern passed\n");
        tests_passed++;
    }
#endif
    
#ifdef __riscv_v
    printf("Testing RISC-V Vector paths...\n");
    tests_run++;
    if (test_riscv_10_operands()) {
        printf("  ✓ RISC-V 10-operand pattern passed\n");
        tests_passed++;
    }
#endif

/* Run generic tests only if no specific ISA tests were compiled */
#if !defined(__AVX512F__) && !defined(__ARM_FEATURE_SVE) && \
    !defined(__ALTIVEC__) && !defined(__riscv_v)
    printf("Testing generic inline assembly paths...\n");
    tests_run++;
    if (test_generic_10_operands()) {
        printf("  ✓ Generic 10-operand pattern passed\n");
        tests_passed++;
    }
    
    tests_run++;
    if (test_generic_11_operands()) {
        printf("  ✓ Generic 11-operand pattern passed\n");
        tests_passed++;
    }
#endif
    
    printf("\nSummary: %d/%d tests passed\n", tests_passed, tests_run);
    
    if (tests_passed == tests_run) {
        printf("\nSUCCESS: All multi-operand patterns executed correctly.\n");
        printf("If compiled with appropriate optimization flags, this should\n");
        printf("trigger the 10-11 operand expansion cases in optabs.cc.\n");
        return 0;
    } else {
        printf("\nFAILURE: Some tests failed.\n");
        return 1;
    }
}
