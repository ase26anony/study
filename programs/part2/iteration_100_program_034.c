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
    
    /* UNORDERED case - __builtin_isunordered */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case - __builtin_isordered */
    if (__builtin_isordered(c, d)) result |= 2;
    
    /* UNEQ case - unordered or equal */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) result |= 4;
    
    /* UNGE case - not less than (nlt) */
    if (!__builtin_isless(a, b)) result |= 8;
    
    /* UNGT case - not less or equal (nle) */
    if (!__builtin_islessequal(a, b)) result |= 16;
    
    /* UNLE case - unordered or less or equal (ule) */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) result |= 32;
    
    /* UNLT case - unordered or less than (ult) */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) result |= 64;
    
    /* LTGT case - less or greater (une) */
    if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float* arr1, float* arr2, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 a = _mm_loadu_ps(&arr1[i]);
        __m128 b = _mm_loadu_ps(&arr2[i]);
        
        /* UNORDERED - _mm_cmpunord_ps */
        __m128 cmp_unord = _mm_cmpunord_ps(a, b);
        
        /* ORDERED - _mm_cmpord_ps */
        __m128 cmp_ord = _mm_cmpord_ps(a, b);
        
        /* UNGE - _mm_cmpnlt_ps (not less than) */
        __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);
        
        /* UNGT - _mm_cmpnle_ps (not less or equal) */
        __m128 cmp_nle = _mm_cmpnle_ps(a, b);
        
        /* UNLE - _mm_cmple_ps (less or equal) - compiler may use ule */
        __m128 cmp_le = _mm_cmple_ps(a, b);
        
        /* UNLT - _mm_cmplt_ps (less than) - compiler may use ult */
        __m128 cmp_lt = _mm_cmplt_ps(a, b);
        
        /* LTGT - _mm_cmpneq_ps (not equal) - compiler may use une */
        __m128 cmp_neq = _mm_cmpneq_ps(a, b);
        
        /* Mix results to prevent optimization */
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_unord, a));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_ord, b));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_nlt, a));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_nle, b));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_le, a));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_lt, b));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_neq, a));
    }
    
    /* Extract result */
    float result[4];
    _mm_storeu_ps(result, sum);
    return (int)(result[0] + result[1] + result[2] + result[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int tmp;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED - "u" flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setu %0"
        : "=r"(tmp) : "x"(a), "x"(b) : "cc"
    );
    result += tmp;
    
    /* ORDERED - "o" flag (actually "no" for not overflow, but ordered) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(tmp) : "x"(a), "x"(b) : "cc"
    );
    result += tmp;
    
    /* UNEQ - "e" flag (equal) with unordered */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(tmp) : "x"(a), "x"(b) : "cc"
    );
    result += tmp;
    
    /* UNGE - "ae" flag (above or equal) - nlt */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r"(tmp) : "x"(a), "x"(b) : "cc"
    );
    result += tmp;
    
    /* UNGT - "a" flag (above) - nle */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(tmp) : "x"(a), "x"(b) : "cc"
    );
    result += tmp;
    
    /* UNLE - "be" flag (below or equal) - ule */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(tmp) : "x"(a), "x"(b) : "cc"
    );
    result += tmp;
    
    /* UNLT - "b" flag (below) - ult */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(tmp) : "x"(a), "x"(b) : "cc"
    );
    result += tmp;
    
    /* LTGT - "ne" flag (not equal) - une */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(tmp) : "x"(a), "x"(b) : "cc"
    );
    result += tmp;
    
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(float* farr, double* darr, int n) {
    int checksum = 0;
    
    for (int i = 0; i < n; i++) {
        float f1 = farr[i];
        float f2 = farr[(i + 1) % n];
        double d1 = darr[i];
        double d2 = darr[(i + 1) % n];
        
        /* Mix of different comparison types */
        checksum += (__builtin_isunordered(f1, f2) ? 1 : 0);
        checksum += (__builtin_isordered(d1, d2) ? 2 : 0);
        checksum += (!__builtin_isgreater(f1, f2) ? 4 : 0);
        checksum += (!__builtin_islessequal(d1, d2) ? 8 : 0);
        checksum += (__builtin_isless(f1, f2) || __builtin_isunordered(f1, f2) ? 16 : 0);
        checksum += (__builtin_islessequal(d1, d2) || __builtin_isunordered(d1, d2) ? 32 : 0);
        checksum += (__builtin_isless(f1, f2) || __builtin_isgreater(f1, f2) ? 64 : 0);
    }
    
    return checksum;
}

int main() {
    const int N = 64;
    float farr[N];
    double darr[N];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < N; i++) {
        if (i % 8 == 0) {
            farr[i] = NAN;
            darr[i] = NAN;
        } else if (i % 8 == 1) {
            farr[i] = -NAN;
            darr[i] = -NAN;
        } else if (i % 8 == 2) {
            farr[i] = INFINITY;
            darr[i] = INFINITY;
        } else if (i % 8 == 3) {
            farr[i] = -INFINITY;
            darr[i] = -INFINITY;
        } else {
            farr[i] = (float)(i * 1.2345);
            darr[i] = i * 1.23456789;
        }
    }
    
    int total = 0;
    
    /* Test scalar builtins */
    total += test_scalar_builtins(farr[0], farr[1], darr[0], darr[1]);
    
    /* Test SSE intrinsics */
    total += test_sse_intrinsics(farr, farr + 1, N - 4);
    
    /* Test inline assembly */
    total += test_inline_asm(farr[2], farr[3]);
    
    /* Test mixed comparisons */
    total += test_mixed_comparisons(farr, darr, N);
    
    /* Use result to prevent optimization */
    printf("Result checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
