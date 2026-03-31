/* test_condition_codes.c - Cover GCC i386 condition code output routines */
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
    
    /* UNORDERED case - __builtin_isunordered() */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case - __builtin_isordered() */
    if (__builtin_isordered(c, d)) result |= 2;
    
    /* UNEQ case - unordered or equal */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) result |= 4;
    
    /* UNGE case - not less than (nlt) */
    if (__builtin_isgreaterequal(a, b)) result |= 8;
    
    /* UNGT case - not less than or equal (nle) */
    if (__builtin_isgreater(a, b)) result |= 16;
    
    /* UNLE case - unordered or less than or equal (ule) */
    if (__builtin_islessequal(a, b)) result |= 32;
    
    /* UNLT case - unordered or less than (ult) */
    if (__builtin_isless(a, b)) result |= 64;
    
    /* LTGT case - less than or greater than (une) */
    if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(const float* arr1, const float* arr2, int n) {
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
        
        /* UNGT - _mm_cmpnle_ps (not less than or equal) */
        __m128 cmp_nle = _mm_cmpnle_ps(a, b);
        
        /* UNLE - _mm_cmple_ps (less than or equal) */
        __m128 cmp_ule = _mm_cmple_ps(a, b);
        
        /* UNLT - _mm_cmplt_ps (less than) */
        __m128 cmp_ult = _mm_cmplt_ps(a, b);
        
        /* LTGT - _mm_cmpneq_ps (not equal) */
        __m128 cmp_une = _mm_cmpneq_ps(a, b);
        
        /* UNEQ - _mm_cmpeq_ps (equal) */
        __m128 cmp_ueq = _mm_cmpeq_ps(a, b);
        
        /* Combine results */
        __m128 combined = _mm_add_ps(cmp_unord, cmp_ord);
        combined = _mm_add_ps(combined, cmp_nlt);
        combined = _mm_add_ps(combined, cmp_nle);
        combined = _mm_add_ps(combined, cmp_ule);
        combined = _mm_add_ps(combined, cmp_ult);
        combined = _mm_add_ps(combined, cmp_une);
        combined = _mm_add_ps(combined, cmp_ueq);
        
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
    
    /* ORDERED - "no" flag (not overflow) used differently */
    asm volatile ("setno %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNEQ - "e" flag (equal) */
    asm volatile ("sete %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNGE - "nl" flag (not less) */
    asm volatile ("setnl %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNGT - "nle" flag (not less or equal) */
    asm volatile ("setnle %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNLE - "le" flag (less or equal) */
    asm volatile ("setle %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNLT - "l" flag (less) */
    asm volatile ("setl %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(temp) : : "cc");
    result += temp;
    
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(double* darr1, double* darr2, int n) {
    int checksum = 0;
    
    for (int i = 0; i < n; i++) {
        double a = darr1[i];
        double b = darr2[i];
        
        /* Generate various condition codes through different comparisons */
        checksum += __builtin_isunordered(a, b) ? 1 : 0;
        checksum += __builtin_isordered(a, b) ? 2 : 0;
        checksum += (a == b) ? 4 : 0;  /* May generate UNEQ */
        checksum += (a >= b) ? 8 : 0;  /* May generate UNGE */
        checksum += (a > b) ? 16 : 0;  /* May generate UNGT */
        checksum += (a <= b) ? 32 : 0; /* May generate UNLE */
        checksum += (a < b) ? 64 : 0;  /* May generate UNLT */
        checksum += (a != b) ? 128 : 0; /* May generate LTGT */
    }
    
    return checksum;
}

/* Initialize arrays with NaN values to ensure unordered comparisons */
void init_arrays(float* arr1, float* arr2, double* darr1, double* darr2, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix of normal numbers and NaN */
        if (i % 5 == 0) {
            arr1[i] = NAN;
            arr2[i] = (float)i;
            darr1[i] = NAN;
            darr2[i] = (double)i;
        } else if (i % 5 == 1) {
            arr1[i] = (float)i;
            arr2[i] = NAN;
            darr1[i] = (double)i;
            darr2[i] = NAN;
        } else if (i % 5 == 2) {
            arr1[i] = NAN;
            arr2[i] = NAN;
            darr1[i] = NAN;
            darr2[i] = NAN;
        } else {
            arr1[i] = (float)(i * 1.1f);
            arr2[i] = (float)(i * 0.9f);
            darr1[i] = (double)(i * 1.1);
            darr2[i] = (double)(i * 0.9);
        }
    }
}

int main() {
    const int N = 256;
    float arr1[N], arr2[N];
    double darr1[N], darr2[N];
    
    /* Initialize with mixed values including NaN */
    init_arrays(arr1, arr2, darr1, darr2, N);
    
    int total = 0;
    
    /* Test scalar builtins with various inputs */
    total += test_scalar_builtins(arr1[0], arr2[0], darr1[0], darr2[0]);
    total += test_scalar_builtins(1.0f, NAN, 2.0, NAN);
    total += test_scalar_builtins(NAN, 1.0f, NAN, 2.0);
    total += test_scalar_builtins(1.0f, 2.0f, 3.0, 4.0);
    
    /* Test SSE intrinsics */
    total += test_sse_intrinsics(arr1, arr2, N);
    
    /* Test inline assembly */
    total += test_inline_asm(arr1[0], arr2[0]);
    total += test_inline_asm(1.0f, NAN);
    total += test_inline_asm(NAN, 1.0f);
    
    /* Test mixed comparisons */
    total += test_mixed_comparisons(darr1, darr2, N);
    
    /* Use result to prevent optimization */
    printf("Result checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
