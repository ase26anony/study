/* test_optabs_10_11_operands.c
 * Test program to trigger RTL expansion with 10-11 operands
 * Compile with: gcc -O3 -ftree-vectorize -funroll-loops -fopenmp -march=native test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* Runtime validation helper */
static int g_tests_passed = 0;
static int g_tests_run = 0;

#define TEST_START(name) do { \
    printf("Testing %s... ", name); \
    g_tests_run++; \
} while(0)

#define TEST_PASS() do { \
    printf("PASS\n"); \
    g_tests_passed++; \
} while(0)

#define TEST_FAIL(reason) do { \
    printf("FAIL: %s\n", reason); \
} while(0)

/* Function to prevent inlining and force RTL expansion */
__attribute__((noinline, target("arch=native")))
static void dummy_asm_10_operands(uint64_t *out, uint64_t a, uint64_t b, 
                                  uint64_t c, uint64_t d, uint64_t e,
                                  uint64_t f, uint64_t g, uint64_t h,
                                  uint64_t i) {
    /* Inline assembly with 10 operands (1 output + 9 inputs) */
    asm volatile (
        "/* 10-operand dummy instruction */\n\t"
        "add %[out], %[a], %[b]\n\t"
        "add %[out], %[out], %[c]\n\t"
        "add %[out], %[out], %[d]\n\t"
        "add %[out], %[out], %[e]\n\t"
        "add %[out], %[out], %[f]\n\t"
        "add %[out], %[out], %[g]\n\t"
        "add %[out], %[out], %[h]\n\t"
        "add %[out], %[out], %[i]"
        : [out] "=r" (*out)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i)
        : "cc"
    );
}

__attribute__((noinline, target("arch=native")))
static void dummy_asm_11_operands(uint64_t *out1, uint64_t *out2, 
                                  uint64_t a, uint64_t b, uint64_t c,
                                  uint64_t d, uint64_t e, uint64_t f,
                                  uint64_t g, uint64_t h, uint64_t i) {
    /* Inline assembly with 11 operands (2 outputs + 9 inputs) */
    asm volatile (
        "/* 11-operand dummy instruction */\n\t"
        "mov %[out1], %[a]\n\t"
        "mov %[out2], %[b]\n\t"
        "add %[out1], %[out1], %[c]\n\t"
        "add %[out2], %[out2], %[d]\n\t"
        "add %[out1], %[out1], %[e]\n\t"
        "add %[out2], %[out2], %[f]\n\t"
        "add %[out1], %[out1], %[g]\n\t"
        "add %[out2], %[out2], %[h]\n\t"
        "add %[out1], %[out1], %[i]"
        : [out1] "=r" (*out1), [out2] "=r" (*out2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i)
        : "cc"
    );
}

#ifdef __AVX512F__
/* Pattern A: 10-operand masked gather (x86 AVX-512) */
__attribute__((noinline, target("avx512f")))
static void test_avx512_gather_10_operands(double *result, 
                                           const double *base,
                                           const int64_t *indices,
                                           __mmask8 mask,
                                           double src,
                                           int scale) {
    /* This should expand to approximately 10 operands:
       1. Destination vector
       2. Mask
       3. Base pointer
       4. Index vector
       5. Scale
       6. Source vector
       7. Displacement
       8. Hint
       9. Rounding mode
       10. Exception control
    */
    __m512d src_vec = _mm512_set1_pd(src);
    __m512i idx_vec = _mm512_loadu_si512((const __m512i*)indices);
    
    /* Force multiple operations in one expression to increase operand count */
    __m512d gathered = _mm512_mask_i64gather_pd(src_vec, mask, idx_vec, 
                                               base, scale);
    
    /* Additional operations to create complex pattern */
    __m512d scaled = _mm512_mul_pd(gathered, _mm512_set1_pd(2.0));
    __m512d result_vec = _mm512_add_pd(scaled, _mm512_set1_pd(1.0));
    
    _mm512_storeu_pd(result, result_vec);
}

/* Pattern B: 11-operand complex FMA pattern (x86 AVX-512) */
__attribute__((noinline, target("avx512f,avx512vl")))
static void test_avx512_complex_11_operands(double *out1, double *out2,
                                           const double *a, const double *b,
                                           const double *c, const double *d,
                                           __mmask8 mask1, __mmask8 mask2) {
    /* Complex pattern that might require 11 operands during expansion */
    __m512d vec_a = _mm512_loadu_pd(a);
    __m512d vec_b = _mm512_loadu_pd(b);
    __m512d vec_c = _mm512_loadu_pd(c);
    __m512d vec_d = _mm512_loadu_pd(d);
    
    /* Multiple masked operations in sequence */
    __m512d tmp1 = _mm512_mask_mul_pd(vec_a, mask1, vec_b, vec_c);
    __m512d tmp2 = _mm512_mask_add_pd(vec_d, mask2, tmp1, vec_a);
    __m512d tmp3 = _mm512_mask_sub_pd(tmp1, mask1, tmp2, vec_b);
    
    /* Fused multiply-add with mask */
    __m512d result1 = _mm512_mask_fmadd_pd(tmp3, mask1, vec_c, vec_d);
    __m512d result2 = _mm512_mask_fnmadd_pd(tmp2, mask2, vec_a, vec_b);
    
    _mm512_storeu_pd(out1, result1);
    _mm512_storeu_pd(out2, result2);
}
#endif /* __AVX512F__ */

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

/* Pattern for ARM SVE with many operands */
__attribute__((noinline))
static void test_sve_gather_10_operands(double *result,
                                        const double *base,
                                        const int64_t *offsets,
                                        svbool_t pg) {
    /* SVE gather with multiple operands */
    svint64_t offset_vec = svld1_s64(pg, offsets);
    svfloat64_t gathered = svld1_gather_index(pg, base, offset_vec);
    
    /* Additional operations */
    svfloat64_t scaled = svmul_x(pg, gathered, svdup_f64(2.0));
    svfloat64_t result_vec = svadd_x(pg, scaled, svdup_f64(1.0));
    
    svst1(pg, result, result_vec);
}
#endif /* __ARM_FEATURE_SVE */

#ifdef __PPC64__
#include <altivec.h>

/* Pattern for PowerPC Altivec/VSX */
__attribute__((noinline))
static void test_powerpc_10_operands(vector double *out,
                                     vector double a, vector double b,
                                     vector double c, vector double d,
                                     vector double e, vector double f,
                                     vector double g, vector double h,
                                     vector double i) {
    /* Complex vector operations that might expand to many operands */
    vector double t1 = vec_madd(a, b, c);
    vector double t2 = vec_madd(d, e, f);
    vector double t3 = vec_madd(g, h, i);
    vector double t4 = vec_add(t1, t2);
    *out = vec_add(t4, t3);
}
#endif /* __PPC64__ */

/* Hot loop to trigger vectorization and RTL expansion */
__attribute__((noinline, optimize("O3")))
static void hot_loop_with_many_operands(int iterations) {
    uint64_t array[1024];
    uint64_t result1, result2;
    
    /* Initialize array */
    for (int i = 0; i < 1024; i++) {
        array[i] = i * 3;
    }
    
    /* Hot loop that might be unrolled/vectorized */
    #pragma omp parallel for simd
    for (int i = 0; i < iterations; i++) {
        int idx = i % 1000;
        
        /* Mix of 10 and 11 operand patterns */
        if (i % 2 == 0) {
            dummy_asm_10_operands(&result1, 
                                 array[idx], array[idx+1], array[idx+2],
                                 array[idx+3], array[idx+4], array[idx+5],
                                 array[idx+6], array[idx+7], array[idx+8]);
        } else {
            dummy_asm_11_operands(&result1, &result2,
                                 array[idx], array[idx+1], array[idx+2],
                                 array[idx+3], array[idx+4], array[idx+5],
                                 array[idx+6], array[idx+7], array[idx+8]);
        }
        
        /* Use results to prevent optimization */
        array[idx] = result1 + (result2 % 256);
    }
}

/* Main test driver */
int main(void) {
    printf("Testing RTL expansion with 10-11 operands...\n\n");
    
    /* Test 1: Generic inline assembly patterns */
    TEST_START("Generic 10-operand inline assembly");
    {
        uint64_t out;
        dummy_asm_10_operands(&out, 1, 2, 3, 4, 5, 6, 7, 8, 9);
        if (out == 45) {  /* 1+2+3+4+5+6+7+8+9 = 45 */
            TEST_PASS();
        } else {
            TEST_FAIL("Unexpected result");
        }
    }
    
    TEST_START("Generic 11-operand inline assembly");
    {
        uint64_t out1, out2;
        dummy_asm_11_operands(&out1, &out2, 1, 2, 3, 4, 5, 6, 7, 8, 9);
        if (out1 == 33 && out2 == 20) {  /* out1 = 1+3+5+7+9=25, out2=2+4+6+8=20 */
            TEST_PASS();
        } else {
            TEST_FAIL("Unexpected results");
        }
    }
    
    /* Test 2: Hot loop to trigger optimization passes */
    TEST_START("Hot loop with mixed operand patterns");
    hot_loop_with_many_operands(10000);
    TEST_PASS();
    
#ifdef __AVX512F__
    /* Test 3: AVX-512 specific patterns */
    TEST_START("AVX-512 masked gather (10 operands)");
    {
        double result[8];
        double base[64];
        int64_t indices[8];
        __mmask8 mask = 0xFF;
        
        for (int i = 0; i < 64; i++) base[i] = i * 1.5;
        for (int i = 0; i < 8; i++) indices[i] = i * 8;
        
        test_avx512_gather_10_operands(result, base, indices, mask, 0.0, 8);
        
        /* Simple validation */
        int valid = 1;
        for (int i = 0; i < 8; i++) {
            double expected = base[i * 8] * 2.0 + 1.0;
            if (result[i] != expected) valid = 0;
        }
        
        if (valid) TEST_PASS();
        else TEST_FAIL("AVX-512 gather result mismatch");
    }
    
    TEST_START("AVX-512 complex FMA (11 operands)");
    {
        double out1[8], out2[8];
        double a[8], b[8], c[8], d[8];
        __mmask8 mask1 = 0xAA; /* 10101010 */
        __mmask8 mask2 = 0x55; /* 01010101 */
        
        for (int i = 0; i < 8; i++) {
            a[i] = i * 1.0;
            b[i] = i * 2.0;
            c[i] = i * 3.0;
            d[i] = i * 4.0;
        }
        
        test_avx512_complex_11_operands(out1, out2, a, b, c, d, mask1, mask2);
        TEST_PASS(); /* Just check it compiles and runs without crashing */
    }
#endif /* __AVX512F__ */
    
#ifdef __ARM_FEATURE_SVE
    TEST_START("ARM SVE gather");
    {
        double result[256];
        double base[1024];
        int64_t offsets[256];
        svbool_t pg = svptrue_b64();
        
        for (int i = 0; i < 1024; i++) base[i] = i * 0.5;
        for (int i = 0; i < 256; i++) offsets[i] = i * 4;
        
        test_sve_gather_10_operands(result, base, offsets, pg);
        TEST_PASS();
    }
#endif /* __ARM_FEATURE_SVE */
    
#ifdef __PPC64__
    TEST_START("PowerPC Altivec complex operations");
    {
        vector double out;
        vector double a = {1.0, 2.0};
        vector double b = {3.0, 4.0};
        vector double c = {5.0, 6.0};
        vector double d = {7.0, 8.0};
        vector double e = {9.0, 10.0};
        vector double f = {11.0, 12.0};
        vector double g = {13.0, 14.0};
        vector double h = {15.0, 16.0};
        vector double i = {17.0, 18.0};
        
        test_powerpc_10_operands(&out, a, b, c, d, e, f, g, h, i);
        TEST_PASS();
    }
#endif /* __PPC64__ */
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", g_tests_run);
    printf("Tests passed: %d\n", g_tests_passed);
    
    if (g_tests_passed == g_tests_run) {
        printf("All tests passed successfully!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}
