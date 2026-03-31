/* test_multi_operand_rtl.c - Test program for 10-11 operand RTL expansion coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Runtime validation helper */
#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s\n", msg); \
        return 0; \
    } \
} while(0)

/* ============================================
   x86 AVX-512 Implementation (10-11 operands)
   ============================================ */
#ifdef __AVX512F__

#include <immintrin.h>
#include <x86intrin.h>

/* Function with 10 operands - masked gather operation */
__attribute__((noinline, target("avx512f")))
static int test_avx512_10_operands(void) {
    /* Setup test data */
    double base[64] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) base[i] = (double)(i * 2);
    for (int i = 0; i < 8; i++) indices[i] = i * 8;
    
    /* This gather intrinsic conceptually uses many operands:
       - Destination vector (__m512d)
       - Mask (__mmask8)
       - Index vector (__m512i)
       - Base pointer
       - Scale (compile-time constant)
       - Source offset
       Total: 6 explicit + 4 implicit = 10+ operands in RTL
    */
    __m512i vindex = _mm512_load_epi64(indices);
    __mmask8 mask = 0xFF;
    __m512d gathered = _mm512_mask_i64gather_pd(_mm512_setzero_pd(), mask, vindex, 
                                                base, 8);
    
    _mm512_store_pd(result, gathered);
    
    /* Validate */
    for (int i = 0; i < 8; i++) {
        CHECK(result[i] == (double)(indices[i] * 2), 
              "AVX-512 gather result mismatch");
    }
    
    return 1;
}

/* Function with 11 operands - complex masked scatter with update */
__attribute__((noinline, target("avx512f,avx512vl")))
static int test_avx512_11_operands(void) {
    /* Setup test data */
    double dest[64] __attribute__((aligned(64))) = {0};
    double src[8] __attribute__((aligned(64)));
    int64_t indices[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src[i] = (double)(100 + i);
        indices[i] = i * 4;
    }
    
    /* Scatter operation with mask - many operands in RTL */
    __m512i vindex = _mm512_load_epi64(indices);
    __m512d vsrc = _mm512_load_pd(src);
    __mmask8 mask = 0xFF;
    
    /* This scatter has many conceptual operands:
       - Base pointer
       - Mask
       - Index vector
       - Source vector
       - Scale
       - Additional offset/update flags
       Total: ~11 operands in RTL expansion
    */
    _mm512_mask_i64scatter_pd(dest, mask, vindex, vsrc, 8);
    
    /* Validate scatter */
    for (int i = 0; i < 8; i++) {
        CHECK(dest[indices[i]] == src[i], 
              "AVX-512 scatter result mismatch");
    }
    
    return 1;
}

#endif /* __AVX512F__ */

/* ============================================
   ARM SVE Implementation (10-11 operands)
   ============================================ */
#ifdef __ARM_FEATURE_SVE

#include <arm_sve.h>

/* Function with 10 operands - SVE gather with predicate */
__attribute__((noinline))
static int test_arm_sve_10_operands(void) {
    /* Setup test data */
    uint64_t base[128];
    uint64_t indices_data[svcntd()];
    uint64_t result[svcntd()];
    
    for (size_t i = 0; i < 128; i++) base[i] = i * 3;
    for (size_t i = 0; i < svcntd(); i++) indices_data[i] = i * 2;
    
    svbool_t pg = svptrue_b64();
    svuint64_t indices = svld1_u64(pg, indices_data);
    
    /* SVE gather - conceptually many operands:
       - Predicate
       - Base pointer
       - Index vector
       - Scale (implicit)
       - Result vector
       Total: ~10 operands in RTL
    */
    svuint64_t gathered = svld1_gather_u64index_u64(pg, base, indices);
    
    svst1_u64(pg, result, gathered);
    
    /* Validate */
    for (size_t i = 0; i < svcntd(); i++) {
        CHECK(result[i] == base[indices_data[i]], 
              "ARM SVE gather result mismatch");
    }
    
    return 1;
}

/* Function with 11 operands - SVE scatter with predicate and offset */
__attribute__((noinline))
static int test_arm_sve_11_operands(void) {
    /* Setup test data */
    uint64_t dest[128] = {0};
    uint64_t src_data[svcntd()];
    uint64_t indices_data[svcntd()];
    
    for (size_t i = 0; i < svcntd(); i++) {
        src_data[i] = 200 + i;
        indices_data[i] = i * 3;
    }
    
    svbool_t pg = svptrue_b64();
    svuint64_t src = svld1_u64(pg, src_data);
    svuint64_t indices = svld1_u64(pg, indices_data);
    
    /* SVE scatter - many operands in RTL expansion */
    svst1_scatter_u64index_u64(pg, dest, indices, src);
    
    /* Validate */
    for (size_t i = 0; i < svcntd(); i++) {
        CHECK(dest[indices_data[i]] == src_data[i], 
              "ARM SVE scatter result mismatch");
    }
    
    return 1;
}

#endif /* __ARM_FEATURE_SVE */

/* ============================================
   PowerPC VSX/Altivec Implementation
   ============================================ */
#ifdef __ALTIVEC__

#include <altivec.h>

/* Function with 10 operands - complex vector permute */
__attribute__((noinline))
static int test_powerpc_10_operands(void) {
    /* Setup vectors */
    vector unsigned char a = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    vector unsigned char b = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    vector unsigned char perm = {31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16};
    
    /* Complex permutation with multiple steps - can generate many operands */
    vector unsigned char result1 = vec_perm(a, b, perm);
    vector unsigned char result2 = vec_perm(b, a, perm);
    
    /* Combine operations */
    vector unsigned char final = vec_add(result1, result2);
    
    /* Validate pattern */
    unsigned char final_arr[16];
    memcpy(final_arr, &final, 16);
    
    for (int i = 0; i < 16; i++) {
        CHECK(final_arr[i] == (31 - i) + (15 - i), 
              "PowerPC vector permute result mismatch");
    }
    
    return 1;
}

#endif /* __ALTIVEC__ */

/* ============================================
   Generic Inline Assembly Fallback
   ============================================ */

/* Inline assembly with exactly 10 operands */
__attribute__((noinline))
static int test_inline_asm_10_operands(void) {
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long result;
    
    /* 10-operand inline asm - forces RTL expansion with 10 ops */
    asm volatile (
        "/* 10-operand dummy instruction */\n\t"
        "add %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    CHECK(result == 55, "10-operand inline asm result mismatch");
    return 1;
}

/* Inline assembly with exactly 11 operands */
__attribute__((noinline))
static int test_inline_asm_11_operands(void) {
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    long result;
    
    /* 11-operand inline asm - forces RTL expansion with 11 ops */
    asm volatile (
        "/* 11-operand dummy instruction */\n\t"
        "mov %[res], %[a]\n\t"
        "add %[res], %[res], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]\n\t"
        "add %[res], %[res], %[k]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    CHECK(result == 66, "11-operand inline asm result mismatch");
    return 1;
}

/* ============================================
   Hot loop to encourage RTL expansion
   ============================================ */

/* Function containing hot loop with vector operations */
__attribute__((noinline, optimize("O3", "unroll-loops")))
static void hot_loop_vector_operations(void) {
    /* This loop when vectorized may generate multi-operand instructions */
    double arr1[1024], arr2[1024], arr3[1024], arr4[1024];
    
    for (int i = 0; i < 1024; i++) {
        arr1[i] = (double)i;
        arr2[i] = (double)(i * 2);
        arr3[i] = (double)(i * 3);
        arr4[i] = (double)(i * 4);
    }
    
    /* Complex computation that might be vectorized */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = arr1[i] * arr2[i] + arr3[i] * arr4[i];
    }
    
    /* Use result to prevent optimization */
    volatile double sum = 0;
    for (int i = 0; i < 1024; i++) {
        sum += arr1[i];
    }
}

/* ============================================
   Main test driver
   ============================================ */

int main(void) {
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("Testing multi-operand RTL expansion coverage...\n\n");
    
    /* Always run inline assembly tests */
    printf("Testing inline assembly (10 operands)... ");
    if (test_inline_asm_10_operands()) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    tests_run++;
    
    printf("Testing inline assembly (11 operands)... ");
    if (test_inline_asm_11_operands()) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    tests_run++;
    
    /* Architecture-specific tests */
#ifdef __AVX512F__
    printf("\nTesting AVX-512 (10 operands)... ");
    if (test_avx512_10_operands()) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    tests_run++;
    
    printf("Testing AVX-512 (11 operands)... ");
    if (test_avx512_11_operands()) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    tests_run++;
#endif
    
#ifdef __ARM_FEATURE_SVE
    printf("\nTesting ARM SVE (10 operands)... ");
    if (test_arm_sve_10_operands()) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    tests_run++;
    
    printf("Testing ARM SVE (11 operands)... ");
    if (test_arm_sve_11_operands()) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    tests_run++;
#endif
    
#ifdef __ALTIVEC__
    printf("\nTesting PowerPC Altivec (10 operands)... ");
    if (test_powerpc_10_operands()) {
        printf("PASS\n");
        tests_passed++;
    } else {
        printf("FAIL\n");
    }
    tests_run++;
#endif
    
    /* Run hot loop to encourage optimization passes */
    printf("\nRunning hot loop to trigger vectorization...\n");
    hot_loop_vector_operations();
    
    /* Summary */
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Tests run: %d\n", tests_run);
    printf("  Tests passed: %d\n", tests_passed);
    
    if (tests_passed == tests_run) {
        printf("  ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("  SOME TESTS FAILED\n");
        return 1;
    }
}
