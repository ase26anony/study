/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger the 10-11 operand RTL expansion paths
 * in GCC's optabs.cc (lines 8254-8263) by using architecture-specific
 * intrinsics and inline assembly that require many operands.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation utilities */
#define ARRAY_SIZE 256
#define VALIDATE(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        return 0; \
    } \
} while(0)

static int tests_passed = 0;
static int tests_run = 0;

/* ============================================================
 * x86 AVX-512 Implementation (10-11 operand patterns)
 * ============================================================ */
#ifdef __AVX512F__

#include <immintrin.h>
#include <x86intrin.h>

/* Pattern A: 10 operands - Masked gather with multiple parameters */
__attribute__((noinline, target("avx512f,avx512vl")))
int test_avx512_10_operands(void) {
    tests_run++;
    
    /* Setup test data */
    double base[ARRAY_SIZE] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        base[i] = (double)(i * 2);
    }
    for (int i = 0; i < 8; i++) {
        indices[i] = i * 4;
        result[i] = 0.0;
    }
    
    /* This gather intrinsic expands to many operands:
     * 1. Destination vector
     * 2. Mask
     * 3. Base pointer
     * 4. Index vector
     * 5. Scale
     * 6. Vector length hint
     * 7. Source (for scatter, not used here)
     * Plus implicit operands for addressing modes
     */
    __m512i vindex = _mm512_load_epi64(indices);
    __mmask8 mask = 0xFF;
    __m512d src = _mm512_setzero_pd();
    
    /* The actual gather - should generate 10+ operand RTL */
    __m512d gathered = _mm512_mask_i64gather_pd(src, mask, vindex, base, 8);
    
    /* Store to force computation */
    _mm512_store_pd(result, gathered);
    
    /* Validate */
    for (int i = 0; i < 8; i++) {
        double expected = base[indices[i] / 8];
        VALIDATE(result[i] == expected, 
                "AVX-512 10-operand gather result mismatch");
    }
    
    tests_passed++;
    return 1;
}

/* Pattern B: 11 operands - Complex masked scatter with update */
__attribute__((noinline, target("avx512f")))
int test_avx512_11_operands(void) {
    tests_run++;
    
    /* Setup scatter data */
    double target[ARRAY_SIZE] __attribute__((aligned(64)));
    double source[8] __attribute__((aligned(64)));
    int64_t scatter_idx[8] __attribute__((aligned(64)));
    
    memset(target, 0, sizeof(target));
    for (int i = 0; i < 8; i++) {
        source[i] = (double)(100 + i);
        scatter_idx[i] = i * 8;
    }
    
    __m512d vsrc = _mm512_load_pd(source);
    __m512i vidx = _mm512_load_epi64(scatter_idx);
    __mmask8 mask = 0xFF;
    
    /* Scatter operation with many implicit operands */
    _mm512_mask_i64scatter_pd(target, mask, vidx, vsrc, 8);
    
    /* Validate scatter */
    for (int i = 0; i < 8; i++) {
        VALIDATE(target[scatter_idx[i] / 8] == source[i],
                "AVX-512 11-operand scatter result mismatch");
    }
    
    tests_passed++;
    return 1;
}

/* Inline assembly with exactly 10 operands */
__attribute__((noinline, target("avx512f")))
int test_avx512_inline_asm_10(void) {
    tests_run++;
    
    double a = 1.0, b = 2.0, c = 3.0, d = 4.0;
    double e = 5.0, f = 6.0, g = 7.0, h = 8.0;
    double i = 9.0, j = 10.0;
    double result[4] __attribute__((aligned(64)));
    
    /* 10-operand inline assembly pattern */
    __asm__ volatile (
        "vmovapd %[res], %%zmm0\n\t"
        "vbroadcastsd %[a], %%zmm1\n\t"
        "vbroadcastsd %[b], %%zmm2\n\t"
        "vbroadcastsd %[c], %%zmm3\n\t"
        "vbroadcastsd %[d], %%zmm4\n\t"
        "vbroadcastsd %[e], %%zmm5\n\t"
        "vbroadcastsd %[f], %%zmm6\n\t"
        "vbroadcastsd %[g], %%zmm7\n\t"
        "vbroadcastsd %[h], %%zmm8\n\t"
        "vbroadcastsd %[i], %%zmm9\n\t"
        "vfmadd231pd %%zmm1, %%zmm2, %%zmm0\n\t"
        "vfmadd231pd %%zmm3, %%zmm4, %%zmm0\n\t"
        "vfmadd231pd %%zmm5, %%zmm6, %%zmm0\n\t"
        "vfmadd231pd %%zmm7, %%zmm8, %%zmm0\n\t"
        "vaddpd %%zmm9, %%zmm0, %%zmm0\n\t"
        "vmovapd %%zmm0, %[res]"
        : [res] "=m" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i)
        : "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5",
          "zmm6", "zmm7", "zmm8", "zmm9", "memory"
    );
    
    /* Simple validation */
    double expected = a*b + c*d + e*f + g*h + i;
    VALIDATE(fabs(result[0] - expected) < 1e-10,
            "AVX-512 inline asm 10-operand result mismatch");
    
    tests_passed++;
    return 1;
}

#endif /* __AVX512F__ */

/* ============================================================
 * ARM SVE Implementation (10-11 operand patterns)
 * ============================================================ */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Pattern A for ARM SVE: 10+ operand gather */
__attribute__((noinline))
int test_arm_sve_10_operands(void) {
    tests_run++;
    
    uint64_t base[ARRAY_SIZE];
    uint64_t indices[svcntd()];
    uint64_t result[svcntd()];
    
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        base[i] = i * 3;
    }
    for (size_t i = 0; i < svcntd(); i++) {
        indices[i] = i * 2;
    }
    
    svbool_t pg = svptrue_b64();
    svuint64_t vindex = svld1_u64(pg, indices);
    
    /* SVE gather with multiple operands */
    svuint64_t gathered = svld1_gather_u64index_u64(pg, base, vindex);
    
    svst1_u64(pg, result, gathered);
    
    /* Validate */
    for (size_t i = 0; i < svcntd(); i++) {
        VALIDATE(result[i] == base[indices[i]],
                "ARM SVE 10-operand gather result mismatch");
    }
    
    tests_passed++;
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ============================================================
 * PowerPC Altivec/VSX Implementation
 * ============================================================ */
#ifdef __ALTIVEC__

#include <altivec.h>

/* Complex permutation requiring many operands */
__attribute__((noinline))
int test_powerpc_10_operands(void) {
    tests_run++;
    
    vector float a = {1.0f, 2.0f, 3.0f, 4.0f};
    vector float b = {5.0f, 6.0f, 7.0f, 8.0f};
    vector float c = {9.0f, 10.0f, 11.0f, 12.0f};
    vector float d = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Complex sequence of operations that may expand to many operands */
    vector float t1 = vec_madd(a, b, c);
    vector float t2 = vec_madd(b, c, d);
    vector float t3 = vec_madd(c, d, a);
    vector float t4 = vec_madd(d, a, b);
    
    /* Permutation with many inputs */
    vector unsigned char perm = {0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15};
    vector float result = vec_perm(t1, t2, perm);
    result = vec_madd(result, t3, t4);
    
    /* Store and validate */
    float res_arr[4];
    vec_st(result, 0, res_arr);
    
    /* Simple validation - just check it's not zero */
    float sum = res_arr[0] + res_arr[1] + res_arr[2] + res_arr[3];
    VALIDATE(sum > 0.0f, "PowerPC 10-operand result is zero");
    
    tests_passed++;
    return 1;
}

#endif /* __ALTIVEC__ */

/* ============================================================
 * RISC-V Vector Extension
 * ============================================================ */
#ifdef __riscv_v

/* Generic RISC-V vector intrinsic placeholder */
__attribute__((noinline))
int test_riscv_10_operands(void) {
    tests_run++;
    
    /* RISC-V vector operations with many operands */
    long a[16] __attribute__((aligned(64)));
    long b[16] __attribute__((aligned(64)));
    long result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    
    /* Inline assembly simulating complex vector operation */
    asm volatile (
        "vsetvli zero, %[vl], e64, m8, ta, ma\n\t"
        "vle64.v v0, (%[a])\n\t"
        "vle64.v v8, (%[b])\n\t"
        "vadd.vv v16, v0, v8\n\t"
        "vadd.vv v24, v16, v0\n\t"
        "vmul.vv v0, v24, v8\n\t"
        "vse64.v v0, (%[result])\n\t"
        : 
        : [a] "r" (a), [b] "r" (b), [result] "r" (result),
          [vl] "r" (16)
        : "v0", "v8", "v16", "v24", "memory"
    );
    
    /* Validate */
    for (int i = 0; i < 16; i++) {
        long expected = (a[i] + b[i] + a[i]) * b[i];
        VALIDATE(result[i] == expected, 
                "RISC-V vector operation result mismatch");
    }
    
    tests_passed++;
    return 1;
}

#endif /* __riscv_v */

/* ============================================================
 * Generic fallback with extended inline assembly
 * ============================================================ */
__attribute__((noinline))
int test_generic_11_operand_asm(void) {
    tests_run++;
    
    /* Force 11 operands in inline assembly */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result1, result2;
    
    asm volatile (
        "mov %[r1], %[a1]\n\t"
        "add %[r1], %[r1], %[a2]\n\t"
        "add %[r1], %[r1], %[a3]\n\t"
        "add %[r1], %[r1], %[a4]\n\t"
        "add %[r1], %[r1], %[a5]\n\t"
        "mov %[r2], %[a6]\n\t"
        "add %[r2], %[r2], %[a7]\n\t"
        "add %[r2], %[r2], %[a8]\n\t"
        "add %[r2], %[r2], %[a9]\n\t"
        "add %[r2], %[r2], %[a10]\n\t"
        "imul %[r1], %[r2]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10)
        : "cc"
    );
    
    long expected1 = op1 + op2 + op3 + op4 + op5;
    long expected2 = op6 + op7 + op8 + op9 + op10;
    VALIDATE(result1 == expected1 * expected2,
            "Generic 11-operand asm result mismatch");
    
    tests_passed++;
    return 1;
}

/* ============================================================
 * Hot loop to encourage RTL expansion
 * ============================================================ */
__attribute__((noinline, optimize("O3")))
void hot_loop_with_multi_operand_ops(int iterations) {
    volatile int counter = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix of operations to encourage different expansions */
        counter += i;
        
        #ifdef __AVX512F__
        /* Force AVX-512 expansions in loop */
        if (i % 100 == 0) {
            __m512i v1 = _mm512_set1_epi64(i);
            __m512i v2 = _mm512_set1_epi64(i * 2);
            __m512i v3 = _mm512_add_epi64(v1, v2);
            counter += _mm512_reduce_add_epi64(v3);
        }
        #endif
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r" (counter));
}

/* ============================================================
 * Main test driver
 * ============================================================ */
int main(void) {
    printf("Testing 10-11 operand RTL expansion paths...\n");
    
    /* Run architecture-specific tests */
    int any_tests_run = 0;
    
    #ifdef __AVX512F__
    printf("Running AVX-512 tests...\n");
    if (test_avx512_10_operands()) {
        printf("  AVX-512 10-operand test: PASS\n");
        any_tests_run = 1;
    }
    if (test_avx512_11_operands()) {
        printf("  AVX-512 11-operand test: PASS\n");
        any_tests_run = 1;
    }
    if (test_avx512_inline_asm_10()) {
        printf("  AVX-512 inline asm 10-operand test: PASS\n");
        any_tests_run = 1;
    }
    #endif
    
    #ifdef __ARM_FEATURE_SVE
    printf("Running ARM SVE tests...\n");
    if (test_arm_sve_10_operands()) {
        printf("  ARM SVE 10-operand test: PASS\n");
        any_tests_run = 1;
    }
    #endif
    
    #ifdef __ALTIVEC__
    printf("Running PowerPC tests...\n");
    if (test_powerpc_10_operands()) {
        printf("  PowerPC 10-operand test: PASS\n");
        any_tests_run = 1;
    }
    #endif
    
    #ifdef __riscv_v
    printf("Running RISC-V Vector tests...\n");
    if (test_riscv_10_operands()) {
        printf("  RISC-V 10-operand test: PASS\n");
        any_tests_run = 1;
    }
    #endif
    
    /* Always run generic test */
    printf("Running generic tests...\n");
    if (test_generic_11_operand_asm()) {
        printf("  Generic 11-operand asm test: PASS\n");
        any_tests_run = 1;
    }
    
    /* Run hot loop to encourage RTL expansion during optimization */
    printf("Running hot loop to trigger optimizations...\n");
    hot_loop_with_multi_operand_ops(1000);
    
    /* Summary */
    printf("\nTest Summary:\n");
    printf("  Tests run: %d\n", tests_run);
    printf("  Tests passed: %d\n", tests_passed);
    
    if (tests_run == 0) {
        printf("\nWARNING: No architecture-specific tests were compiled.\n");
        printf("Compile with appropriate flags for your architecture:\n");
        printf("  x86 AVX-512: -mavx512f -mavx512vl\n");
        printf("  ARM SVE: -march=armv8-a+sve\n");
        printf("  PowerPC: -maltivec\n");
        printf("  RISC-V: -march=rv64gcv\n");
    }
    
    return (tests_passed == tests_run && any_tests_run) ? 0 : 1;
}
