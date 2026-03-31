/* Test program to cover floating-point condition code output in i386.cc */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b, double c, double d) {
    int result = 0;
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case */
    if (__builtin_isordered(c, d)) result |= 2;
    
    /* UNEQ case (unordered or equal) */
    if (!__builtin_islessgreater(a, b)) result |= 4;
    
    /* UNGE case (not less than) */
    if (!__builtin_isless(a, b)) result |= 8;
    
    /* UNGT case (not less than or equal) */
    if (!__builtin_islessequal(c, d)) result |= 16;
    
    /* UNLE case (unordered or less than or equal) */
    if (__builtin_islessequal(a, b)) result |= 32;
    
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(a, b)) result |= 64;
    
    /* LTGT case (less than or greater than, but not equal) */
    if (__builtin_islessgreater(c, d)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *arr1, float *arr2, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 a = _mm_loadu_ps(&arr1[i]);
        __m128 b = _mm_loadu_ps(&arr2[i]);
        
        /* UNORDERED */
        __m128 cmp_unord = _mm_cmpunord_ps(a, b);
        
        /* ORDERED */
        __m128 cmp_ord = _mm_cmpord_ps(a, b);
        
        /* UNGE (nlt) */
        __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);
        
        /* UNGT (nle) */
        __m128 cmp_nle = _mm_cmpnle_ps(a, b);
        
        /* UNLE (unordered or less than or equal) - use CMPLEPS */
        __m128 cmp_ule = _mm_cmple_ps(a, b);
        
        /* UNLT (unordered or less than) - use CMPLTPS */
        __m128 cmp_ult = _mm_cmplt_ps(a, b);
        
        /* LTGT (unequal) - use CMPNEQPS */
        __m128 cmp_une = _mm_cmpneq_ps(a, b);
        
        /* Combine results */
        __m128 combined = _mm_add_ps(cmp_unord, cmp_ord);
        combined = _mm_add_ps(combined, cmp_nlt);
        combined = _mm_add_ps(combined, cmp_nle);
        combined = _mm_add_ps(combined, cmp_ule);
        combined = _mm_add_ps(combined, cmp_ult);
        combined = _mm_add_ps(combined, cmp_une);
        
        sum = _mm_add_ps(sum, combined);
    }
    
    /* Extract result */
    float res[4];
    _mm_storeu_ps(res, sum);
    return (int)(res[0] + res[1] + res[2] + res[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int tmp;
    
    /* Use inline assembly with condition code constraints */
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* ORDERED - "no" flag (not overflow) - but we need ordered FP */
    /* We'll use a different approach for ordered */
    
    /* UNGE - "ge" flag (greater or equal) with FP */
    asm volatile ("setge %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNGT - "g" flag (greater) with FP */
    asm volatile ("setg %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNLE - "le" flag (less or equal) with FP */
    asm volatile ("setle %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNLT - "l" flag (less) with FP */
    asm volatile ("setl %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(float *farr, double *darr, int n) {
    int checksum = 0;
    
    for (int i = 0; i < n; i++) {
        float f1 = farr[i];
        float f2 = farr[(i + 1) % n];
        double d1 = darr[i];
        double d2 = darr[(i + 1) % n];
        
        /* Mix different comparison types */
        checksum += (__builtin_isunordered(f1, f2) ? 1 : 0);
        checksum += (__builtin_isordered(d1, d2) ? 2 : 0);
        checksum += (!__builtin_islessgreater(f1, f2) ? 4 : 0);
        checksum += (!__builtin_isless(f1, f2) ? 8 : 0);
        checksum += (!__builtin_islessequal(d1, d2) ? 16 : 0);
        checksum += (__builtin_islessequal(f1, f2) ? 32 : 0);
        checksum += (__builtin_isless(f1, f2) ? 64 : 0);
        checksum += (__builtin_islessgreater(d1, d2) ? 128 : 0);
    }
    
    return checksum;
}

int main() {
    /* Create arrays with NaN values to trigger unordered comparisons */
    float farr[16];
    double darr[16];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < 16; i++) {
        if (i % 3 == 0) {
            farr[i] = NAN;
            darr[i] = NAN;
        } else if (i % 3 == 1) {
            farr[i] = (float)i * 1.5f;
            darr[i] = (double)i * 1.5;
        } else {
            farr[i] = (float)(i * -0.7f);
            darr[i] = (double)(i * -0.7);
        }
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(farr[0], farr[1], darr[0], darr[1]);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(farr, farr + 8, 8);
    
    /* Test inline assembly */
    checksum += test_inline_asm(farr[2], farr[3]);
    
    /* Test mixed comparisons */
    checksum += test_mixed_comparisons(farr, darr, 16);
    
    /* Use the checksum to prevent optimization */
    printf("Result: %d\n", checksum);
    
    return 0;
}
