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
    
    /* ORDERED case - __builtin_islessgreater with fast-math may generate ORDERED */
    if (!__builtin_isunordered(a, b)) result |= 2;  /* Ordered check */
    
    /* UNEQ case - unordered or equal */
    if (__builtin_isunordered(a, b) || a == b) result |= 4;
    
    /* UNGE case - not less than (nlt) */
    if (!(a < b)) result |= 8;
    
    /* UNGT case - not less than or equal (nle) */
    if (!(a <= b)) result |= 16;
    
    /* UNLE case - unordered or less than or equal */
    if (__builtin_isunordered(a, b) || a <= b) result |= 32;
    
    /* UNLT case - unordered or less than */
    if (__builtin_isunordered(a, b) || a < b) result |= 64;
    
    /* LTGT case - less than or greater than (une) */
    if (a < b || a > b) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *a, float *b, int n) {
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
        
        /* UNGT - _mm_cmpnle_ps (not less than or equal) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        
        /* UNLE - _mm_cmple_ps (less than or equal) with unordered semantics */
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
    int r;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(r) : : "cc");
    result |= (r & 1);
    
    /* ORDERED - "no" flag (not overflow, but we need ordered) */
    /* Using fucomip to set flags, then testing */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "setnp %0\n\t"
        : "=r"(r) : : "cc"
    );
    result |= (r & 1) << 1;
    
    /* UNEQ - "e" flag after unordered check */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "sete %0\n\t"
        : "=r"(r) : : "cc"
    );
    result |= (r & 1) << 2;
    
    /* UNGE - "ae" flag (above or equal) */
    asm volatile ("setae %0" : "=r"(r) : : "cc");
    result |= (r & 1) << 3;
    
    /* UNGT - "a" flag (above) */
    asm volatile ("seta %0" : "=r"(r) : : "cc");
    result |= (r & 1) << 4;
    
    /* UNLE - "be" flag (below or equal) */
    asm volatile ("setbe %0" : "=r"(r) : : "cc");
    result |= (r & 1) << 5;
    
    /* UNLT - "b" flag (below) */
    asm volatile ("setb %0" : "=r"(r) : : "cc");
    result |= (r & 1) << 6;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(r) : : "cc");
    result |= (r & 1) << 7;
    
    return result;
}

__attribute__((noinline))
int test_double_comparisons(double a, double b) {
    int result = 0;
    
    /* Generate various condition codes with doubles */
    if (__builtin_isgreater(a, b)) result |= 1;
    if (__builtin_isless(a, b)) result |= 2;
    if (__builtin_isgreaterequal(a, b)) result |= 4;
    if (__builtin_islessequal(a, b)) result |= 8;
    if (__builtin_islessgreater(a, b)) result |= 16;
    
    /* Force unordered comparisons with NaN */
    double nan = NAN;
    if (__builtin_isunordered(a, nan)) result |= 32;
    if (!__builtin_isunordered(b, nan)) result |= 64;
    
    return result;
}

int main() {
    /* Create test data with NaN values to trigger unordered comparisons */
    float fa[] = {1.0f, 2.0f, NAN, 4.0f, 5.0f, NAN, 7.0f, 8.0f};
    float fb[] = {2.0f, 1.0f, 3.0f, NAN, NAN, 6.0f, 8.0f, 7.0f};
    
    double da = 1.5;
    double db = 2.5;
    double dnan = NAN;
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(fa[0], fb[0], da, db);
    checksum += test_scalar_builtins(fa[2], fb[2], dnan, db);  /* With NaN */
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, 8);
    
    /* Test inline assembly */
    checksum += test_inline_asm(fa[0], fb[0]);
    checksum += test_inline_asm(fa[2], fb[2]);  /* With NaN */
    
    /* Test double comparisons */
    checksum += test_double_comparisons(da, db);
    checksum += test_double_comparisons(dnan, db);
    checksum += test_double_comparisons(da, dnan);
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
