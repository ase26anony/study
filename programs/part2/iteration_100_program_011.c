/* test_condition_codes.c - Cover i386.cc condition code output routines */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b) {
    int result = 0;
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case - via !unordered */
    if (!__builtin_isunordered(a, b)) result |= 2;
    
    /* UNEQ case - unordered or equal */
    if (__builtin_isunordered(a, b) || a == b) result |= 4;
    
    /* UNGE case - not less than (nlt) */
    if (!(a < b)) result |= 8;
    
    /* UNGT case - not less or equal (nle) */
    if (!(a <= b)) result |= 16;
    
    /* UNLE case - unordered or less or equal (ule) */
    if (__builtin_isunordered(a, b) || a <= b) result |= 32;
    
    /* UNLT case - unordered or less than (ult) */
    if (__builtin_isunordered(a, b) || a < b) result |= 64;
    
    /* LTGT case - less than or greater than (une) */
    if (a < b || a > b) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_vector_intrinsics(float *a, float *b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _mm_cmpunord_ps */
        __m128 mask_unord = _mm_cmpunord_ps(va, vb);
        
        /* ORDERED - _mm_cmpord_ps */
        __m128 mask_ord = _mm_cmpord_ps(va, vb);
        
        /* UNGE - _mm_cmpnlt_ps (not less than) */
        __m128 mask_nlt = _mm_cmpnlt_ps(va, vb);
        
        /* UNGT - _mm_cmpnle_ps (not less or equal) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        
        /* UNLE - _mm_cmple_ps (less or equal) with unordered semantics */
        __m128 mask_ule = _mm_cmple_ps(va, vb);
        
        /* UNLT - _mm_cmplt_ps (less than) with unordered semantics */
        __m128 mask_ult = _mm_cmplt_ps(va, vb);
        
        /* LTGT - _mm_cmpneq_ps (not equal) */
        __m128 mask_une = _mm_cmpneq_ps(va, vb);
        
        /* Combine masks */
        __m128 combined = _mm_add_ps(mask_unord, mask_ord);
        combined = _mm_add_ps(combined, mask_nlt);
        combined = _mm_add_ps(combined, mask_nle);
        combined = _mm_add_ps(combined, mask_ule);
        combined = _mm_add_ps(combined, mask_ult);
        combined = _mm_add_ps(combined, mask_une);
        
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
    int tmp;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* ORDERED - "no" flag (not overflow, but we need ordered) */
    /* Using "np" (parity) for ordered comparison */
    asm volatile ("setnp %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNEQ - "e" flag (equal) */
    asm volatile ("sete %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNGE - "ge" flag (greater or equal) */
    asm volatile ("setge %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNGT - "g" flag (greater) */
    asm volatile ("setg %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNLE - "le" flag (less or equal) */
    asm volatile ("setle %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNLT - "l" flag (less) */
    asm volatile ("setl %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(double x, double y) {
    int result = 0;
    
    /* Generate various condition codes through different comparisons */
    
    /* Using fpclassify to trigger condition codes */
    int cx = fpclassify(x);
    int cy = fpclassify(y);
    
    if (cx == FP_NAN || cy == FP_NAN) result |= 1;  /* UNORDERED */
    if (cx != FP_NAN && cy != FP_NAN) result |= 2;  /* ORDERED */
    
    /* Comparisons that may generate UNEQ, UNGE, etc. */
    if (!(x < y)) result |= 4;   /* UNGE (nlt) */
    if (!(x <= y)) result |= 8;  /* UNGT (nle) */
    
    /* Using isnan with comparisons */
    if (isnan(x) || isnan(y) || x <= y) result |= 16;  /* UNLE */
    if (isnan(x) || isnan(y) || x < y) result |= 32;   /* UNLT */
    
    /* LTGT - not equal and ordered */
    if (x != y && !isnan(x) && !isnan(y)) result |= 64;  /* LTGT */
    
    return result;
}

int main() {
    /* Create test data with NaN values to trigger unordered comparisons */
    float a[16], b[16];
    double x, y;
    
    /* Initialize with mix of normal and NaN values */
    for (int i = 0; i < 16; i++) {
        a[i] = (i % 3 == 0) ? NAN : (float)i;
        b[i] = (i % 4 == 0) ? NAN : (float)(15 - i);
    }
    
    x = NAN;
    y = 3.14159;
    
    int checksum = 0;
    
    /* Test 1: Scalar builtins */
    checksum += test_scalar_builtins(a[0], b[0]);
    checksum += test_scalar_builtins(1.0f, 2.0f);
    checksum += test_scalar_builtins(NAN, NAN);
    
    /* Test 2: Vector intrinsics */
    checksum += test_vector_intrinsics(a, b, 16);
    
    /* Test 3: Inline assembly */
    checksum += test_inline_asm(a[1], b[1]);
    
    /* Test 4: Mixed comparisons with doubles */
    checksum += test_mixed_comparisons(x, y);
    checksum += test_mixed_comparisons(1.0, 2.0);
    checksum += test_mixed_comparisons(NAN, NAN);
    
    /* Use the result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
