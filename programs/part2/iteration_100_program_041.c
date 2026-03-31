/* test_condition_codes.c - Cover all floating-point condition code outputs */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

/* SSE/AVX intrinsics */
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b, double c, double d) {
    int result = 0;
    
    /* UNORDERED - __builtin_isunordered */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED - __builtin_isordered */
    if (__builtin_isordered(c, d)) result |= 2;
    
    /* UNEQ - unordered or equal */
    if (!(a > b) && !(a < b)) result |= 4;  /* May generate UNEQ with fast-math */
    
    /* UNGE - not less than (nlt) */
    if (__builtin_isgreaterequal(a, b)) result |= 8;
    
    /* UNGT - not less than or equal (nle) */
    if (__builtin_isgreater(a, b)) result |= 16;
    
    /* UNLE - unordered or less than or equal (ule) */
    if (__builtin_islessequal(a, b)) result |= 32;
    
    /* UNLT - unordered or less than (ult) */
    if (__builtin_isless(a, b)) result |= 64;
    
    /* LTGT - less than or greater than (une) */
    if (__builtin_islessgreater(c, d)) result |= 128;
    
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
        
        /* UNGT - _mm_cmpnle_ps (not less than or equal) */
        __m128 cmp_nle = _mm_cmpnle_ps(a, b);
        
        /* UNLE - _mm_cmple_ps (less than or equal) - may generate ule */
        __m128 cmp_ule = _mm_cmple_ps(a, b);
        
        /* UNLT - _mm_cmplt_ps (less than) - may generate ult */
        __m128 cmp_ult = _mm_cmplt_ps(a, b);
        
        /* LTGT - _mm_cmpneq_ps (not equal) - may generate une */
        __m128 cmp_une = _mm_cmpneq_ps(a, b);
        
        /* UNEQ - _mm_cmpeq_ps (equal) - may generate ueq with fast-math */
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
    
    /* ORDERED - "no" flag (not unordered) */
    asm volatile ("setno %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNEQ - "e" flag (equal) - may be used with unordered semantics */
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
    
    /* UNLT - "l" flag (less than) */
    asm volatile ("setl %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(temp) : : "cc");
    result += temp;
    
    return result;
}

__attribute__((noinline))
int test_mixed_conditions(float* farr, double* darr, int n) {
    int checksum = 0;
    
    for (int i = 0; i < n; i++) {
        float f1 = farr[i];
        float f2 = farr[(i + 1) % n];
        double d1 = darr[i];
        double d2 = darr[(i + 1) % n];
        
        /* Mix of conditions that may generate different codes */
        checksum += (__builtin_isunordered(f1, f2) ? 1 : 0);
        checksum += (__builtin_isordered(d1, d2) ? 2 : 0);
        checksum += (__builtin_isgreaterequal(f1, f2) ? 4 : 0);
        checksum += (__builtin_isgreater(f1, f2) ? 8 : 0);
        checksum += (__builtin_islessequal(f1, f2) ? 16 : 0);
        checksum += (__builtin_isless(f1, f2) ? 32 : 0);
        checksum += (__builtin_islessgreater(d1, d2) ? 64 : 0);
        
        /* Force generation of UNEQ through equality test with NaN possibility */
        if (!(f1 > f2) && !(f1 < f2)) checksum += 128;
    }
    
    return checksum;
}

int main() {
    const int N = 64;
    float farr[N];
    double darr[N];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < N; i++) {
        farr[i] = (i % 8 == 0) ? NAN : (float)(i * 0.1);
        darr[i] = (i % 7 == 0) ? NAN : (double)(i * 0.2);
    }
    
    int total = 0;
    
    /* Test scalar builtins */
    total += test_scalar_builtins(farr[0], farr[1], darr[0], darr[1]);
    
    /* Test SSE intrinsics */
    total += test_sse_intrinsics(farr, farr + 1, N - 4);
    
    /* Test inline assembly */
    total += test_inline_asm(farr[2], farr[3]);
    
    /* Test mixed conditions */
    total += test_mixed_conditions(farr, darr, N);
    
    printf("Result checksum: %d\n", total);
    
    /* Additional volatile use to prevent optimization */
    volatile int dummy = total;
    
    return 0;
}
