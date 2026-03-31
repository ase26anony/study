/* test_condition_codes.c - Cover GCC i386 condition code output routines */
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
    
    /* UNORDERED case - __builtin_isunordered */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case - __builtin_isless/greater with fast-math may generate this */
    if (!__builtin_isunordered(a, b)) result |= 2;
    
    /* UNEQ case - unordered or equal */
    if (__builtin_isunordered(a, b) || a == b) result |= 4;
    
    /* UNGE case - not less than (nlt) */
    if (!__builtin_isless(a, b)) result |= 8;
    
    /* UNGT case - not less or equal (nle) */
    if (!__builtin_islessequal(a, b)) result |= 16;
    
    /* UNLE case - unordered or less or equal (ule) */
    if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) result |= 32;
    
    /* UNLT case - unordered or less than (ult) */
    if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) result |= 64;
    
    /* LTGT case - less than or greater than (une) */
    if (a != b) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *arr1, float *arr2, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 a = _mm_loadu_ps(&arr1[i]);
        __m128 b = _mm_loadu_ps(&arr2[i]);
        
        /* UNORDERED - _mm_cmpunord_ps */
        __m128 unord_mask = _mm_cmpunord_ps(a, b);
        
        /* ORDERED - _mm_cmpord_ps */
        __m128 ord_mask = _mm_cmpord_ps(a, b);
        
        /* UNGE - _mm_cmpnlt_ps (not less than) */
        __m128 nlt_mask = _mm_cmpnlt_ps(a, b);
        
        /* UNGT - _mm_cmpnle_ps (not less or equal) */
        __m128 nle_mask = _mm_cmpnle_ps(a, b);
        
        /* UNLE - _mm_cmple_ps with unordered handling */
        __m128 le_mask = _mm_cmple_ps(a, b);
        
        /* UNLT - _mm_cmplt_ps with unordered handling */
        __m128 lt_mask = _mm_cmplt_ps(a, b);
        
        /* LTGT - _mm_cmpneq_ps (not equal) */
        __m128 neq_mask = _mm_cmpneq_ps(a, b);
        
        /* Combine masks to prevent optimization */
        __m128 combined = _mm_add_ps(unord_mask, ord_mask);
        combined = _mm_add_ps(combined, nlt_mask);
        combined = _mm_add_ps(combined, nle_mask);
        combined = _mm_add_ps(combined, le_mask);
        combined = _mm_add_ps(combined, lt_mask);
        combined = _mm_add_ps(combined, neq_mask);
        
        sum = _mm_add_ps(sum, combined);
    }
    
    /* Extract result */
    float result[4];
    _mm_storeu_ps(result, sum);
    return (int)(result[0] + result[1] + result[2] + result[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int temp;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* ORDERED - "no" flag (not overflow, but we need ordered) */
    /* Using fucomip to set flags, then testing ordered condition */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "setnp %0\n\t"
        : "=r"(temp) : : "cc", "st"
    );
    result += temp;
    
    /* UNGE - "nb" (not below) for floating point */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "setnb %0\n\t"
        : "=r"(temp) : : "cc", "st"
    );
    result += temp;
    
    /* UNGT - "nbe" (not below or equal) */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "setnbe %0\n\t"
        : "=r"(temp) : : "cc", "st"
    );
    result += temp;
    
    /* UNLE - "na" (not above) */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "setna %0\n\t"
        : "=r"(temp) : : "cc", "st"
    );
    result += temp;
    
    /* UNLT - "b" (below) - unordered less than */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "setb %0\n\t"
        : "=r"(temp) : : "cc", "st"
    );
    result += temp;
    
    /* LTGT - "ne" (not equal) */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "setne %0\n\t"
        : "=r"(temp) : : "cc", "st"
    );
    result += temp;
    
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(double *darr1, double *darr2, int n) {
    int checksum = 0;
    
    for (int i = 0; i < n; i++) {
        double a = darr1[i];
        double b = darr2[i];
        
        /* Generate various condition codes through different comparisons */
        
        /* UNORDERED and ORDERED */
        if (isunordered(a, b)) checksum ^= 1;
        if (!isunordered(a, b)) checksum ^= 2;
        
        /* UNEQ - Using fpclassify to force specific patterns */
        if (fpclassify(a) == FP_NAN || fpclassify(b) == FP_NAN || a == b) 
            checksum ^= 4;
        
        /* UNGE and UNGT with fast-math optimizations */
        if (a >= b) checksum ^= 8;      /* May generate UNGE with fast-math */
        if (a > b) checksum ^= 16;      /* May generate UNGT with fast-math */
        
        /* UNLE and UNLT */
        if (a <= b) checksum ^= 32;
        if (a < b) checksum ^= 64;
        
        /* LTGT - not equal */
        if (a != b) checksum ^= 128;
    }
    
    return checksum;
}

int main() {
    const int SIZE = 256;
    float farr1[SIZE], farr2[SIZE];
    double darr1[SIZE], darr2[SIZE];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < SIZE; i++) {
        farr1[i] = (i % 2 == 0) ? (float)i : NAN;
        farr2[i] = (i % 3 == 0) ? (float)(i * 2) : NAN;
        darr1[i] = (i % 5 == 0) ? (double)i : NAN;
        darr2[i] = (i % 7 == 0) ? (double)(i * 3) : NAN;
    }
    
    int total = 0;
    
    /* Test scalar builtins with various inputs */
    total += test_scalar_builtins(1.0f, 2.0f, 3.0, 4.0);
    total += test_scalar_builtins(NAN, 2.0f, NAN, 4.0);
    total += test_scalar_builtins(1.0f, NAN, 3.0, NAN);
    total += test_scalar_builtins(NAN, NAN, NAN, NAN);
    
    /* Test SSE intrinsics */
    total += test_sse_intrinsics(farr1, farr2, SIZE);
    
    /* Test inline assembly */
    total += test_inline_asm(1.0f, 2.0f);
    total += test_inline_asm(NAN, 2.0f);
    total += test_inline_asm(1.0f, NAN);
    
    /* Test mixed comparisons */
    total += test_mixed_comparisons(darr1, darr2, SIZE);
    
    /* Use result to prevent optimization */
    printf("Result checksum: %d\n", total);
    
    return 0;
}
