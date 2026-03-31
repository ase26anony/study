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
    if ((__builtin_isless(a, b) || __builtin_isgreater(a, b)) && 
        !__builtin_isunordered(a, b)) result |= 128;
    
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
        
        /* ORDERED: _CMP_ORD_Q */
        __m128 mask_ord = _mm_cmpord_ps(va, vb);
        
        /* UNGE: _CMP_NLT_UQ (not less than) */
        __m128 mask_nlt = _mm_cmpnlt_ps(va, vb);
        
        /* UNGT: _CMP_NLE_UQ (not less than or equal) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        
        /* UNLE: _CMP_LE_UQ (unordered or less than or equal) */
        __m128 mask_ule = _mm_cmple_ps(va, vb);
        
        /* UNLT: _CMP_LT_UQ (unordered or less than) */
        __m128 mask_ult = _mm_cmplt_ps(va, vb);
        
        /* UNEQ: _CMP_EQ_UQ (unordered or equal) */
        __m128 mask_ueq = _mm_cmpeq_ps(va, vb);
        
        /* LTGT: _CMP_NEQ_UQ (not equal and ordered) */
        __m128 mask_une = _mm_cmpneq_ps(va, vb);
        
        /* Store results to prevent optimization */
        float temp[16];
        _mm_storeu_ps(&temp[0], mask_unord);
        _mm_storeu_ps(&temp[4], mask_ord);
        _mm_storeu_ps(&temp[8], mask_nlt);
        _mm_storeu_ps(&temp[12], mask_nle);
        
        for (int j = 0; j < 4; j++) {
            if (temp[j] != 0.0f) result++;
        }
    }
    
    return result;
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int r1, r2, r3, r4, r5, r6, r7, r8;
    
    /* Use inline assembly with different condition codes */
    
    /* UNORDERED (u) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setu %0"
        : "=r"(r1) : "x"(a), "x"(b) : "cc"
    );
    
    /* ORDERED (no direct flag, but we can use not unordered) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(r2) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNEQ (unordered or equal) - use 'e' flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(r3) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNGE (not less than) - use 'ae' flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r"(r4) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNGT (not less than or equal) - use 'a' flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(r5) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNLE (unordered or less than or equal) - use 'be' flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(r6) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNLT (unordered or less than) - use 'b' flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(r7) : "x"(a), "x"(b) : "cc"
    );
    
    /* LTGT (not equal and ordered) - use 'ne' flag with ordered check */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(r8) : "x"(a), "x"(b) : "cc"
    );
    
    result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(float* arr1, float* arr2, int n) {
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        float a = arr1[i];
        float b = arr2[i];
        
        /* Mix of comparisons that should generate different condition codes */
        if (a != b && !isnan(a) && !isnan(b)) count++;  /* LTGT-like */
        if (isunordered(a, b)) count++;                  /* UNORDERED */
        if (!isless(a, b)) count++;                      /* UNGE */
        if (!islessequal(a, b)) count++;                 /* UNGT */
        if (islessequal(a, b) || isunordered(a, b)) count++; /* UNLE */
        if (isless(a, b) || isunordered(a, b)) count++;  /* UNLT */
        if (fpclassify(a) == FP_NAN || fpclassify(b) == FP_NAN) count++;
    }
    
    return count;
}

int main() {
    /* Create arrays with mix of normal values and NaN */
    float arr1[64];
    float arr2[64];
    double darr1[32];
    double darr2[32];
    
    /* Initialize with pattern including NaN */
    for (int i = 0; i < 64; i++) {
        if (i % 8 == 0) {
            arr1[i] = NAN;
            arr2[i] = (float)i;
        } else if (i % 8 == 4) {
            arr1[i] = (float)i;
            arr2[i] = NAN;
        } else {
            arr1[i] = (float)i;
            arr2[i] = (float)(i * 2);
        }
    }
    
    for (int i = 0; i < 32; i++) {
        darr1[i] = (double)i;
        darr2[i] = (double)(i * 3);
        if (i % 7 == 0) darr1[i] = NAN;
    }
    
    int checksum = 0;
    
    /* Test scalar builtins with various inputs */
    checksum += test_scalar_builtins(arr1[0], arr2[0], darr1[0], darr2[0]);
    checksum += test_scalar_builtins(1.0f, 2.0f, 3.0, 4.0);
    checksum += test_scalar_builtins(NAN, 5.0f, NAN, 6.0);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(arr1, arr2, 64);
    
    /* Test inline assembly */
    checksum += test_inline_asm(arr1[1], arr2[1]);
    checksum += test_inline_asm(NAN, arr2[2]);
    checksum += test_inline_asm(arr1[3], NAN);
    
    /* Test mixed comparisons */
    checksum += test_mixed_comparisons(arr1, arr2, 64);
    
    /* Use the checksum to prevent optimization */
    printf("Result: %d\n", checksum);
    
    return 0;
}
