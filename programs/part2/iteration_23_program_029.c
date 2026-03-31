/* Test program for x86 condition code mnemonics coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;
volatile double checksum = 0.0;

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double a, double b, float fa, float fb) {
    volatile int result;
    
    /* UNORDERED/ORDERED tests with NaN */
    double nan_val = 0.0/0.0;
    result = isunordered(a, nan_val);
    checksum += result;
    
    result = isordered(a, b);
    checksum += result;
    
    /* UNEQ (unordered or equal) */
    result = !(isgreater(a, b) || isless(a, b));
    checksum += result;
    
    /* UNGE (not less than) = !(a < b) */
    result = !isless(a, b);
    checksum += result;
    
    /* UNGT (not less than or equal) = !(a <= b) */
    result = !islessequal(a, b);
    checksum += result;
    
    /* UNLE (unordered or less than or equal) */
    result = isunordered(a, b) || islessequal(a, b);
    checksum += result;
    
    /* UNLT (unordered or less than) */
    result = isunordered(a, b) || isless(a, b);
    checksum += result;
    
    /* LTGT (less than or greater than, but not equal) */
    result = islessgreater(a, b);
    checksum += result;
    
    /* Force branching with volatile control */
    volatile int branch_control = global_counter & 7;
    
    if (isunordered(fa, fb) && (branch_control == 0)) {
        checksum += 1.5;
    }
    
    if (isordered(fa, fb) && (branch_control == 1)) {
        checksum += 2.5;
    }
    
    /* Ternary operations to force condition code use */
    double val1 = (isunordered(a, b) || islessequal(a, b)) ? a : b;
    double val2 = (isunordered(a, b) || isless(a, b)) ? a : b;
    checksum += val1 + val2;
}

/* Test with SSE2 vector operations */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    __m128d cmp_result;
    __m128 fcmp_result;
    volatile double store[4];
    
    /* UNORDERED: _CMP_UNORD_Q */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd(store, cmp_result);
    checksum += store[0] + store[1];
    
    /* ORDERED: _CMP_ORD_Q */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd(store + 2, cmp_result);
    checksum += store[2] + store[3];
    
    /* UNEQ: _CMP_EQ_UQ */
    fcmp_result = _mm_cmp_ps(f1, f2, _CMP_EQ_UQ);
    _mm_storeu_ps((float*)store, fcmp_result);
    checksum += store[0];
    
    /* UNGE: _CMP_NLT_UQ (not less than) */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_pd(store, cmp_result);
    checksum += store[0];
    
    /* UNGT: _CMP_NLE_UQ (not less than or equal) */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_storeu_pd(store, cmp_result);
    checksum += store[1];
    
    /* UNLE: _CMP_LE_UQ */
    fcmp_result = _mm_cmp_ps(f1, f2, _CMP_LE_UQ);
    _mm_storeu_ps((float*)store, fcmp_result);
    checksum += store[0];
    
    /* UNLT: _CMP_LT_UQ */
    fcmp_result = _mm_cmp_ps(f1, f2, _CMP_LT_UQ);
    _mm_storeu_ps((float*)store, fcmp_result);
    checksum += store[1];
    
    /* LTGT: _CMP_NEQ_UQ (not equal) */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NEQ_UQ);
    _mm_storeu_pd(store, cmp_result);
    checksum += store[0] + store[1];
    
    /* Conditional moves based on comparisons */
    __m128d mask = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    __m128d blended = _mm_or_pd(_mm_and_pd(mask, v1), 
                                _mm_andnot_pd(mask, v2));
    _mm_storeu_pd(store, blended);
    checksum += store[0];
}

/* Test inline assembly with explicit condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, __m128d v1, __m128d v2) {
    volatile double result;
    volatile __m128d vresult;
    
    /* Test each condition code mnemonic in inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    /* ORDERED */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    /* UNEQ */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ueq}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    /* UNGE (nlt) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nlt}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    /* UNGT (nle) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nle}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    /* UNLE (ule) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ule}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    /* UNLT (ult) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ult}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    /* LTGT (une) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|une}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    /* Vector version with cmppd */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        "movapd %1, %0"
        : "=x"(vresult) : "x"(v1), "x"(v2) : "cc"
    );
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}\n\t"
        "movapd %1, %0"
        : "=x"(vresult) : "x"(v1), "x"(v2) : "cc"
    );
}

/* Test with mixed optimization levels */
__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions(__m256d av1, __m256d av2) {
    __m256d cmp_result;
    volatile double store[8];
    
    /* Test various AVX comparison predicates */
    cmp_result = _mm256_cmp_pd(av1, av2, _CMP_UNORD_Q);
    _mm256_storeu_pd(store, cmp_result);
    checksum += store[0];
    
    cmp_result = _mm256_cmp_pd(av1, av2, _CMP_ORD_Q);
    _mm256_storeu_pd(store + 4, cmp_result);
    checksum += store[4];
    
    /* Complex control flow to force condition code generation */
    volatile int selector = global_counter & 3;
    __m256d result;
    
    switch (selector) {
        case 0:
            result = _mm256_cmp_pd(av1, av2, _CMP_EQ_UQ);  /* UNEQ */
            break;
        case 1:
            result = _mm256_cmp_pd(av1, av2, _CMP_NLT_UQ); /* UNGE */
            break;
        case 2:
            result = _mm256_cmp_pd(av1, av2, _CMP_NLE_UQ); /* UNGT */
            break;
        case 3:
            result = _mm256_cmp_pd(av1, av2, _CMP_NEQ_UQ); /* LTGT */
            break;
    }
    
    _mm256_storeu_pd(store, result);
    checksum += store[0] + store[1] + store[2] + store[3];
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create test data with various values including NaN and Inf */
    double a = 1.5 + (rand() % 100) * 0.01;
    double b = 2.3 + (rand() % 100) * 0.01;
    double c = 0.0/0.0;  /* NaN */
    double d = 1.0/0.0;  /* Inf */
    
    float fa = 1.2f + (rand() % 100) * 0.01f;
    float fb = 3.4f + (rand() % 100) * 0.01f;
    float fc = 0.0f/0.0f;  /* NaN */
    
    /* Vector data */
    __m128d v1 = _mm_set_pd(a, b);
    __m128d v2 = _mm_set_pd(c, d);
    __m128 f1 = _mm_set_ps(fa, fb, fc, 4.5f);
    __m128 f2 = _mm_set_ps(fb, fc, fa, 6.7f);
    
    __m256d av1 = _mm256_set_pd(a, b, c, d);
    __m256d av2 = _mm256_set_pd(d, c, b, a);
    
    /* Run all test functions multiple times with different data */
    for (int i = 0; i < 3; i++) {
        global_counter = i;
        
        test_scalar_conditions(a + i * 0.1, b - i * 0.1, 
                              fa + i * 0.1f, fb - i * 0.1f);
        
        test_vector_conditions(v1, v2, f1, f2);
        
        test_inline_asm_conditions(a, c, v1, v2);
        
        if (i % 2 == 0) {
            test_avx_conditions(av1, av2);
        }
        
        /* Modify data slightly each iteration */
        a += 0.5;
        b -= 0.3;
        fa += 0.2f;
        fb -= 0.1f;
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Test completed. Check assembly output for condition codes:\n");
    printf("  unord, ord, ueq, nlt, nle, ule, ult, une\n");
    
    return 0;
}
