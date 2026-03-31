/* Test program to cover floating-point condition code output in i386.cc */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b, double c, double d) {
    int result = 0;
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case */
    if (!__builtin_isunordered(a, b)) result |= 2;
    
    /* UNEQ case (unordered or equal) - use with fast-math */
    if (a == b) result |= 4;  /* May generate UNEQ with fast-math */
    
    /* UNGE case (not less than) */
    if (__builtin_isgreaterequal(a, b)) result |= 8;
    
    /* UNGT case (not less than or equal) */
    if (__builtin_isgreater(a, b)) result |= 16;
    
    /* UNLE case (unordered or less than or equal) */
    if (__builtin_islessequal(a, b)) result |= 32;
    
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(a, b)) result |= 64;
    
    /* LTGT case (less than or greater than, ordered) */
    if (a != b && !__builtin_isunordered(a, b)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_vector_intrinsics(const float* arr1, const float* arr2, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 v1 = _mm_loadu_ps(&arr1[i]);
        __m128 v2 = _mm_loadu_ps(&arr2[i]);
        
        /* UNORDERED */
        __m128 cmp_unord = _mm_cmpunord_ps(v1, v2);
        
        /* ORDERED */
        __m128 cmp_ord = _mm_cmpord_ps(v1, v2);
        
        /* UNGE (nlt) */
        __m128 cmp_nlt = _mm_cmpnlt_ps(v1, v2);
        
        /* UNGT (nle) */
        __m128 cmp_nle = _mm_cmpnle_ps(v1, v2);
        
        /* UNLE (unordered or less than or equal) */
        __m128 cmp_ule = _mm_cmple_ps(v1, v2);
        
        /* UNLT (unordered or less than) */
        __m128 cmp_ult = _mm_cmplt_ps(v1, v2);
        
        /* LTGT (unequal, ordered) - use cmpneq and andnot with unord */
        __m128 cmp_neq = _mm_cmpneq_ps(v1, v2);
        __m128 cmp_ltgt = _mm_andnot_ps(_mm_cmpunord_ps(v1, v2), cmp_neq);
        
        /* Combine results */
        sum = _mm_add_ps(sum, cmp_unord);
        sum = _mm_add_ps(sum, cmp_ord);
        sum = _mm_add_ps(sum, cmp_nlt);
        sum = _mm_add_ps(sum, cmp_nle);
        sum = _mm_add_ps(sum, cmp_ule);
        sum = _mm_add_ps(sum, cmp_ult);
        sum = _mm_add_ps(sum, cmp_ltgt);
    }
    
    /* Extract result */
    float temp[4];
    _mm_storeu_ps(temp, sum);
    return (int)(temp[0] + temp[1] + temp[2] + temp[3]);
}

/* Inline assembly tests for condition code output */
__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int out;
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(out) : : "cc");
    result |= out << 0;
    
    /* ORDERED - "no" flag (not unordered) */
    asm volatile ("setno %0" : "=r"(out) : : "cc");
    result |= out << 1;
    
    /* UNEQ - "e" flag (equal) - may be used with floating point */
    asm volatile ("sete %0" : "=r"(out) : : "cc");
    result |= out << 2;
    
    /* UNGE - "nl" flag (not less than) */
    asm volatile ("setnl %0" : "=r"(out) : : "cc");
    result |= out << 3;
    
    /* UNGT - "nle" flag (not less than or equal) */
    asm volatile ("setnle %0" : "=r"(out) : : "cc");
    result |= out << 4;
    
    /* UNLE - "le" flag (less than or equal) */
    asm volatile ("setle %0" : "=r"(out) : : "cc");
    result |= out << 5;
    
    /* UNLT - "l" flag (less than) */
    asm volatile ("setl %0" : "=r"(out) : : "cc");
    result |= out << 6;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(out) : : "cc");
    result |= out << 7;
    
    return result;
}

/* Test with double precision */
__attribute__((noinline))
int test_double_comparisons(double a, double b) {
    int result = 0;
    
    /* Generate various condition codes with doubles */
    result |= fpclassify(a) == FP_NAN ? 1 : 0;
    result |= isunordered(a, b) ? 2 : 0;
    result |= isgreater(a, b) ? 4 : 0;
    result |= isless(a, b) ? 8 : 0;
    result |= isgreaterequal(a, b) ? 16 : 0;
    result |= islessequal(a, b) ? 32 : 0;
    
    /* Force LTGT case */
    if (!isunordered(a, b) && a != b) result |= 64;
    
    return result;
}

/* Main test driver */
int main() {
    /* Create test data with NaN values to trigger unordered comparisons */
    float farr1[16], farr2[16];
    double darr1[8], darr2[8];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < 16; i++) {
        farr1[i] = (float)i * 1.5f;
        farr2[i] = (float)(i % 3) * 2.0f;
        if (i % 5 == 0) farr1[i] = NAN;
        if (i % 7 == 0) farr2[i] = NAN;
    }
    
    for (int i = 0; i < 8; i++) {
        darr1[i] = (double)i * 1.7;
        darr2[i] = (double)(i % 4) * 3.2;
        if (i % 3 == 0) darr1[i] = NAN;
        if (i % 5 == 0) darr2[i] = NAN;
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(farr1[0], farr2[0], darr1[0], darr2[0]);
    checksum += test_scalar_builtins(1.0f, 2.0f, 3.0, 4.0);
    checksum += test_scalar_builtins(NAN, 1.0f, NAN, 2.0);
    
    /* Test vector intrinsics */
    checksum += test_vector_intrinsics(farr1, farr2, 16);
    
    /* Test inline assembly */
    checksum += test_inline_asm(farr1[1], farr2[1]);
    checksum += test_inline_asm(NAN, 1.0f);
    
    /* Test double precision */
    checksum += test_double_comparisons(darr1[0], darr2[0]);
    checksum += test_double_comparisons(1.0, NAN);
    checksum += test_double_comparisons(NAN, NAN);
    
    /* Use checksum to prevent optimization */
    printf("Result checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
