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
    
    /* UNEQ case (unordered or equal) - use with fast-math optimizations */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) result |= 4;
    
    /* UNGE case (not less than) */
    if (!__builtin_isless(a, b)) result |= 8;
    
    /* UNGT case (not less than or equal) */
    if (!__builtin_islessequal(a, b)) result |= 16;
    
    /* UNLE case (unordered or less than or equal) */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) result |= 32;
    
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) result |= 64;
    
    /* LTGT case (less than or greater than, but not unordered) */
    if ((__builtin_isless(c, d) || __builtin_isgreater(c, d)) && 
        !__builtin_isunordered(c, d)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(const float* a, const float* b, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED: _CMP_UNORD_Q */
        __m128 mask_unord = _mm_cmpunord_ps(va, vb);
        result += _mm_movemask_ps(mask_unord);
        
        /* ORDERED: _CMP_ORD_Q */
        __m128 mask_ord = _mm_cmpord_ps(va, vb);
        result += _mm_movemask_ps(mask_ord);
        
        /* UNGE: _CMP_NLT_UQ (not less than) */
        __m128 mask_nlt = _mm_cmpnlt_ps(va, vb);
        result += _mm_movemask_ps(mask_nlt);
        
        /* UNGT: _CMP_NLE_UQ (not less than or equal) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        result += _mm_movemask_ps(mask_nle);
        
        /* UNLE: _CMP_LE_UQ (less than or equal or unordered) */
        __m128 mask_ule = _mm_cmple_ps(va, vb);
        result += _mm_movemask_ps(mask_ule);
        
        /* UNLT: _CMP_LT_UQ (less than or unordered) */
        __m128 mask_ult = _mm_cmplt_ps(va, vb);
        result += _mm_movemask_ps(mask_ult);
        
        /* LTGT: _CMP_NEQ_UQ (not equal and not unordered) */
        __m128 mask_une = _mm_cmpneq_ps(va, vb);
        result += _mm_movemask_ps(mask_une);
        
        /* UNEQ: _CMP_EQ_UQ (equal or unordered) */
        __m128 mask_ueq = _mm_cmpeq_ps(va, vb);
        result += _mm_movemask_ps(mask_ueq);
    }
    
    return result;
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int r1, r2, r3, r4, r5, r6, r7, r8;
    
    /* Use inline assembly with different condition codes */
    
    /* UNORDERED: "u" flag */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setu %0"
                  : "=r"(r1) : "x"(a), "x"(b) : "cc");
    
    /* ORDERED: "no" flag (not overflow) - but we need ordered specifically */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setnp %0"
                  : "=r"(r2) : "x"(a), "x"(b) : "cc");
    
    /* UNEQ: "e" flag (equal) - unordered equal */
    asm volatile ("ucomiss %1, %2\n\t"
                  "sete %0"
                  : "=r"(r3) : "x"(a), "x"(b) : "cc");
    
    /* UNGE: "ae" flag (above or equal) - not less than */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setae %0"
                  : "=r"(r4) : "x"(a), "x"(b) : "cc");
    
    /* UNGT: "a" flag (above) - not less than or equal */
    asm volatile ("ucomiss %1, %2\n\t"
                  "seta %0"
                  : "=r"(r5) : "x"(a), "x"(b) : "cc");
    
    /* UNLE: "be" flag (below or equal) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setbe %0"
                  : "=r"(r6) : "x"(a), "x"(b) : "cc");
    
    /* UNLT: "b" flag (below) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setb %0"
                  : "=r"(r7) : "x"(a), "x"(b) : "cc");
    
    /* LTGT: "ne" flag (not equal) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setne %0"
                  : "=r"(r8) : "x"(a), "x"(b) : "cc");
    
    result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(double* arr1, double* arr2, int n) {
    int checksum = 0;
    
    for (int i = 0; i < n; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Generate various condition codes through different comparisons */
        if (__builtin_isunordered(a, b)) checksum += 1;      /* UNORDERED */
        if (!__builtin_isunordered(a, b)) checksum += 2;     /* ORDERED */
        if (!(a > b) && !(a < b)) checksum += 4;            /* UNEQ with fast-math */
        if (!(a < b)) checksum += 8;                        /* UNGE */
        if (!(a <= b)) checksum += 16;                      /* UNGT */
        if ((a <= b) || __builtin_isunordered(a, b)) checksum += 32; /* UNLE */
        if ((a < b) || __builtin_isunordered(a, b)) checksum += 64;  /* UNLT */
        if ((a < b || a > b) && !__builtin_isunordered(a, b)) checksum += 128; /* LTGT */
    }
    
    return checksum;
}

int main() {
    /* Create arrays with NaN values to trigger unordered comparisons */
    const int SIZE = 64;
    float fa[SIZE], fb[SIZE];
    double da[SIZE], db[SIZE];
    
    /* Initialize with mix of normal numbers and NaN */
    for (int i = 0; i < SIZE; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 5 == 0) ? NAN : (float)(i * 0.7);
        da[i] = (i % 4 == 0) ? NAN : (double)i;
        db[i] = (i % 6 == 0) ? NAN : (double)(i * 1.3);
    }
    
    int total = 0;
    
    /* Test scalar builtins */
    total += test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    total += test_scalar_builtins(fa[1], fb[1], da[1], db[1]);
    
    /* Test SSE intrinsics */
    total += test_sse_intrinsics(fa, fb, SIZE);
    
    /* Test inline assembly */
    total += test_inline_asm(fa[2], fb[2]);
    total += test_inline_asm(fa[3], fb[3]);
    
    /* Test mixed comparisons */
    total += test_mixed_comparisons(da, db, SIZE);
    
    /* Use the result to prevent optimization */
    printf("Result checksum: %d\n", total);
    
    return 0;
}
