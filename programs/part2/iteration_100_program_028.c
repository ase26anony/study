/* Test program to cover condition code output in i386.cc */
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <stdint.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b) {
    int result = 0;
    
    /* UNORDERED - __builtin_isunordered */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED - __builtin_isordered */
    if (__builtin_isordered(a, b)) result |= 2;
    
    /* UNEQ - unordered or equal */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) result |= 4;
    
    /* UNGE - not less than (greater or equal or unordered) */
    if (!__builtin_isless(a, b)) result |= 8;
    
    /* UNGT - not less or equal (greater or unordered) */
    if (!__builtin_islessequal(a, b)) result |= 16;
    
    /* UNLE - less or equal or unordered */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) result |= 32;
    
    /* UNLT - less than or unordered */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) result |= 64;
    
    /* LTGT - less than or greater than (ordered and not equal) */
    if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *a, float *b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _mm_cmpunord_ps */
        __m128 unord_mask = _mm_cmpunord_ps(va, vb);
        
        /* ORDERED - _mm_cmpord_ps */
        __m128 ord_mask = _mm_cmpord_ps(va, vb);
        
        /* UNGE - _mm_cmpnlt_ps (not less than) */
        __m128 nlt_mask = _mm_cmpnlt_ps(va, vb);
        
        /* UNGT - _mm_cmpnle_ps (not less or equal) */
        __m128 nle_mask = _mm_cmpnle_ps(va, vb);
        
        /* UNLE - _mm_cmple_ps (less or equal) - will be used with unordered */
        __m128 le_mask = _mm_cmple_ps(va, vb);
        
        /* UNLT - _mm_cmplt_ps (less than) - will be used with unordered */
        __m128 lt_mask = _mm_cmplt_ps(va, vb);
        
        /* UNEQ - _mm_cmpeq_ps (equal) - combined with unordered */
        __m128 eq_mask = _mm_cmpeq_ps(va, vb);
        
        /* LTGT - _mm_cmpneq_ps (not equal) for ordered */
        __m128 neq_mask = _mm_cmpneq_ps(va, vb);
        
        /* Combine results */
        __m128 combined = _mm_add_ps(unord_mask, ord_mask);
        combined = _mm_add_ps(combined, nlt_mask);
        combined = _mm_add_ps(combined, nle_mask);
        combined = _mm_add_ps(combined, le_mask);
        combined = _mm_add_ps(combined, lt_mask);
        combined = _mm_add_ps(combined, eq_mask);
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
    int tmp;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* ORDERED - "no" flag (not overflow, but we need ordered) 
       Using fucomip to set flags then testing */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "setnp %0"
        : "=r"(tmp) : : "cc", "st"
    );
    result += tmp;
    
    /* UNGE - "ae" flag (above or equal) for unordered semantics */
    asm volatile ("setae %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNGT - "a" flag (above) for unordered semantics */
    asm volatile ("seta %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNLE - "be" flag (below or equal) for unordered semantics */
    asm volatile ("setbe %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNLT - "b" flag (below) for unordered semantics */
    asm volatile ("setb %0" : "=r"(tmp) : : "cc");
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
    
    /* Comparisons that might generate UNEQ, LTGT, etc. */
    if (!(x < y) && !(x > y)) result |= 4;  /* UNEQ or ORDERED EQ */
    if (!(x < y)) result |= 8;              /* UNGE */
    if (!(x <= y)) result |= 16;            /* UNGT */
    if (x <= y || x != x || y != y) result |= 32;  /* UNLE */
    if (x < y || x != x || y != y) result |= 64;   /* UNLT */
    if ((x < y) || (x > y)) result |= 128;         /* LTGT */
    
    return result;
}

int main() {
    /* Create test data with NaN values to ensure unordered comparisons */
    float fa[16], fb[16];
    double da[4], db[4];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < 16; i++) {
        fa[i] = (float)(i * 1.1);
        fb[i] = (float)(i * 0.9);
        if (i % 3 == 0) fa[i] = NAN;
        if (i % 5 == 0) fb[i] = NAN;
    }
    
    for (int i = 0; i < 4; i++) {
        da[i] = i * 2.2;
        db[i] = i * 1.8;
        if (i % 2 == 0) da[i] = NAN;
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(fa[0], fb[0]);
    checksum += test_scalar_builtins(1.0f, 2.0f);
    checksum += test_scalar_builtins(NAN, 3.0f);
    checksum += test_scalar_builtins(4.0f, NAN);
    checksum += test_scalar_builtins(NAN, NAN);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, 16);
    
    /* Test inline assembly */
    checksum += test_inline_asm(fa[1], fb[1]);
    checksum += test_inline_asm(NAN, fb[2]);
    
    /* Test mixed comparisons with doubles */
    checksum += test_mixed_comparisons(da[0], db[0]);
    checksum += test_mixed_comparisons(1.0, 2.0);
    checksum += test_mixed_comparisons(NAN, 3.0);
    
    /* Print checksum to prevent optimization */
    printf("Result: %d\n", checksum);
    
    return 0;
}
