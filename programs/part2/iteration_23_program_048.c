/* Test program to cover condition code output in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;
volatile double checksum = 0.0;

/* Function with specific optimization attributes */
__attribute__((optimize("O0"), target("sse2")))
void test_scalar_conditions(double a, double b, float fa, float fb) {
    volatile int res;
    
    /* Test UNORDERED (unord) */
    res = isunordered(a, b);
    if (res) checksum += 1.0;
    
    /* Test ORDERED (ord) */
    res = !isunordered(a, b);
    if (res) checksum += 2.0;
    
    /* Test UNEQ (ueq) - unordered or equal */
    /* Use complex expression to force condition code */
    volatile double tmp = a;
    if ((isunordered(tmp, b) || (tmp == b))) checksum += 3.0;
    
    /* Test UNGE (nlt) - unordered or greater-or-equal */
    if ((isunordered(a, b) || (a >= b))) checksum += 4.0;
    
    /* Test UNGT (nle) - unordered or greater */
    if ((isunordered(a, b) || (a > b))) checksum += 5.0;
    
    /* Test UNLE (ule) - unordered or less-or-equal */
    if ((isunordered(a, b) || (a <= b))) checksum += 6.0;
    
    /* Test UNLT (ult) - unordered or less */
    if ((isunordered(a, b) || (a < b))) checksum += 7.0;
    
    /* Test LTGT (une) - less or greater (ordered and not equal) */
    if ((!isunordered(a, b) && (a != b))) checksum += 8.0;
    
    /* Float versions to test different mode */
    if (isunordered(fa, fb)) checksum += 9.0;
    if (!isunordered(fa, fb)) checksum += 10.0;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    __m128d cmp_res;
    __m128 fcmp_res;
    volatile double store[2];
    volatile float fstore[4];
    
    /* Test UNORDERED - _CMP_UNORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_store_pd((double*)store, cmp_res);
    checksum += store[0] + store[1];
    
    /* Test ORDERED - _CMP_ORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_store_pd((double*)store, cmp_res);
    checksum += store[0] + store[1];
    
    /* Test UNEQ - _CMP_EQ_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_store_pd((double*)store, cmp_res);
    checksum += store[0] + store[1];
    
    /* Test UNGE - _CMP_NLT_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_store_pd((double*)store, cmp_res);
    checksum += store[0] + store[1];
    
    /* Test UNGT - _CMP_NLE_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_store_pd((double*)store, cmp_res);
    checksum += store[0] + store[1];
    
    /* Test UNLE - _CMP_LE_OS (ordered, signaling) - will still test condition */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_OS);
    _mm_store_pd((double*)store, cmp_res);
    checksum += store[0] + store[1];
    
    /* Test UNLT - _CMP_LT_OS */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_OS);
    _mm_store_pd((double*)store, cmp_res);
    checksum += store[0] + store[1];
    
    /* Test LTGT - _CMP_NEQ_OS */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OS);
    _mm_store_pd((double*)store, cmp_res);
    checksum += store[0] + store[1];
    
    /* Float vector tests */
    fcmp_res = _mm_cmp_ps(f1, f2, _CMP_UNORD_Q);
    _mm_store_ps((float*)fstore, fcmp_res);
    checksum += fstore[0] + fstore[1] + fstore[2] + fstore[3];
    
    fcmp_res = _mm_cmp_ps(f1, f2, _CMP_ORD_Q);
    _mm_store_ps((float*)fstore, fcmp_res);
    checksum += fstore[0] + fstore[1] + fstore[2] + fstore[3];
}

/* Inline assembly tests with explicit condition codes */
__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile __m128d va, vb, vres;
    
    va = _mm_set1_pd(a);
    vb = _mm_set1_pd(b);
    
    /* Test each condition code in inline assembly */
    /* Using AT&T syntax with {%%0|cond} pattern */
    
    /* UNORDERED */
    __asm__ volatile ("cmppd %[unord], %1, %0" 
                     : "=x"(vres) 
                     : "x"(va), [unord]"x"(vb), "{%%0|unord}"(0)
                     : "cc");
    _mm_store_sd(&result, vres);
    checksum += result;
    
    /* ORDERED */
    __asm__ volatile ("cmppd %[ord], %1, %0" 
                     : "=x"(vres) 
                     : "x"(va), [ord]"x"(vb), "{%%0|ord}"(0)
                     : "cc");
    _mm_store_sd(&result, vres);
    checksum += result;
    
    /* UNEQ */
    __asm__ volatile ("cmppd %[ueq], %1, %0" 
                     : "=x"(vres) 
                     : "x"(va), [ueq]"x"(vb), "{%%0|ueq}"(0)
                     : "cc");
    _mm_store_sd(&result, vres);
    checksum += result;
    
    /* UNGE (nlt) */
    __asm__ volatile ("cmppd %[nlt], %1, %0" 
                     : "=x"(vres) 
                     : "x"(va), [nlt]"x"(vb), "{%%0|nlt}"(0)
                     : "cc");
    _mm_store_sd(&result, vres);
    checksum += result;
    
    /* UNGT (nle) */
    __asm__ volatile ("cmppd %[nle], %1, %0" 
                     : "=x"(vres) 
                     : "x"(va), [nle]"x"(vb), "{%%0|nle}"(0)
                     : "cc");
    _mm_store_sd(&result, vres);
    checksum += result;
    
    /* UNLE (ule) */
    __asm__ volatile ("cmppd %[ule], %1, %0" 
                     : "=x"(vres) 
                     : "x"(va), [ule]"x"(vb), "{%%0|ule}"(0)
                     : "cc");
    _mm_store_sd(&result, vres);
    checksum += result;
    
    /* UNLT (ult) */
    __asm__ volatile ("cmppd %[ult], %1, %0" 
                     : "=x"(vres) 
                     : "x"(va), [ult]"x"(vb), "{%%0|ult}"(0)
                     : "cc");
    _mm_store_sd(&result, vres);
    checksum += result;
    
    /* LTGT (une) */
    __asm__ volatile ("cmppd %[une], %1, %0" 
                     : "=x"(vres) 
                     : "x"(va), [une]"x"(vb), "{%%0|une}"(0)
                     : "cc");
    _mm_store_sd(&result, vres);
    checksum += result;
}

/* Test with loops to prevent simplification */
__attribute__((optimize("O3"), target("sse2")))
void test_loop_conditions(const double* arr1, const double* arr2, int n) {
    volatile int count_unord = 0, count_ord = 0, count_ueq = 0;
    volatile int count_nlt = 0, count_nle = 0, count_ule = 0;
    volatile int count_ult = 0, count_une = 0;
    
    for (int i = 0; i < n; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Force all condition evaluations */
        count_unord += isunordered(a, b);
        count_ord += !isunordered(a, b);
        count_ueq += (isunordered(a, b) || (a == b));
        count_nlt += (isunordered(a, b) || (a >= b));
        count_nle += (isunordered(a, b) || (a > b));
        count_ule += (isunordered(a, b) || (a <= b));
        count_ult += (isunordered(a, b) || (a < b));
        count_une += (!isunordered(a, b) && (a != b));
    }
    
    checksum += count_unord + count_ord + count_ueq + 
                count_nlt + count_nle + count_ule + 
                count_ult + count_une;
}

int main(int argc, char* argv[]) {
    /* Initialize with non-uniform values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create test arrays with mixed values (normal, NaN, Inf) */
    double darr1[8], darr2[8];
    float farr1[8], farr2[8];
    
    for (int i = 0; i < 8; i++) {
        darr1[i] = (rand() % 100) / 10.0;
        darr2[i] = (rand() % 100) / 10.0;
        farr1[i] = (rand() % 100) / 10.0f;
        farr2[i] = (rand() % 100) / 10.0f;
        
        /* Introduce some special values */
        if (i == 2) darr1[i] = 0.0 / 0.0; /* NaN */
        if (i == 3) darr2[i] = 1.0 / 0.0; /* Inf */
        if (i == 4) farr1[i] = 0.0f / 0.0f;
        if (i == 5) farr2[i] = -1.0f / 0.0f;
    }
    
    /* Test scalar conditions */
    for (int i = 0; i < 8; i++) {
        test_scalar_conditions(darr1[i], darr2[i], farr1[i], farr2[i]);
    }
    
    /* Test vector conditions */
    for (int i = 0; i < 8; i += 2) {
        __m128d v1 = _mm_set_pd(darr1[i+1], darr1[i]);
        __m128d v2 = _mm_set_pd(darr2[i+1], darr2[i]);
        __m128 f1 = _mm_set_ps(farr1[i+3], farr1[i+2], farr1[i+1], farr1[i]);
        __m128 f2 = _mm_set_ps(farr2[i+3], farr2[i+2], farr2[i+1], farr2[i]);
        test_vector_conditions(v1, v2, f1, f2);
    }
    
    /* Test inline assembly */
    for (int i = 0; i < 8; i++) {
        test_inline_asm_conditions(darr1[i], darr2[i]);
    }
    
    /* Test with loops */
    test_loop_conditions(darr1, darr2, 8);
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: %f\n", checksum);
    
    return 0;
}
