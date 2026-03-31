/* test_condition_codes.c - Cover GCC x86 condition code output routines */
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
    if (!__builtin_isunordered(a, b)) result |= 2;
    
    /* UNEQ case - unordered or equal */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) result |= 4;
    
    /* UNGE case - not less than (greater or equal or unordered) */
    if (!__builtin_isless(a, b)) result |= 8;
    
    /* UNGT case - not less or equal (greater or unordered) */
    if (!__builtin_islessequal(a, b)) result |= 16;
    
    /* UNLE case - unordered or less or equal */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) result |= 32;
    
    /* UNLT case - unordered or less than */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) result |= 64;
    
    /* LTGT case - less or greater (ordered and not equal) */
    if (__builtin_isless(c, d) || __builtin_isgreater(c, d)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(const float* arr1, const float* arr2, int n) {
    int sum = 0;
    
    for (int i = 0; i < n - 3; i += 4) {
        __m128 v1 = _mm_loadu_ps(&arr1[i]);
        __m128 v2 = _mm_loadu_ps(&arr2[i]);
        
        /* UNORDERED - _CMP_UNORD_Q */
        __m128 cmp_unord = _mm_cmpunord_ps(v1, v2);
        int mask_unord = _mm_movemask_ps(cmp_unord);
        sum += mask_unord;
        
        /* ORDERED - _CMP_ORD_Q */
        __m128 cmp_ord = _mm_cmpord_ps(v1, v2);
        int mask_ord = _mm_movemask_ps(cmp_ord);
        sum += mask_ord;
        
        /* UNGE - _CMP_NLT_UQ (not less than, unordered quiet) */
        __m128 cmp_nlt = _mm_cmpnlt_ps(v1, v2);
        int mask_nlt = _mm_movemask_ps(cmp_nlt);
        sum += mask_nlt;
        
        /* UNGT - _CMP_NLE_UQ (not less or equal, unordered quiet) */
        __m128 cmp_nle = _mm_cmpnle_ps(v1, v2);
        int mask_nle = _mm_movemask_ps(cmp_nle);
        sum += mask_nle;
        
        /* UNLE - _CMP_LE_UQ (less or equal, unordered quiet) */
        __m128 cmp_ule = _mm_cmple_ps(v1, v2);
        int mask_ule = _mm_movemask_ps(cmp_ule);
        sum += mask_ule;
        
        /* UNLT - _CMP_LT_UQ (less than, unordered quiet) */
        __m128 cmp_ult = _mm_cmplt_ps(v1, v2);
        int mask_ult = _mm_movemask_ps(cmp_ult);
        sum += mask_ult;
        
        /* LTGT - _CMP_NEQ_UQ (not equal, unordered quiet) */
        __m128 cmp_neq = _mm_cmpneq_ps(v1, v2);
        int mask_neq = _mm_movemask_ps(cmp_neq);
        sum += mask_neq;
    }
    
    return sum;
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int out1, out2, out3, out4, out5, out6, out7, out8;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED - "u" flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setu %0"
        : "=r"(out1)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += out1;
    
    /* ORDERED - "no" flag (not overflow) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setno %0"
        : "=r"(out2)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += out2;
    
    /* UNEQ - "e" flag (equal) for unordered equal */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(out3)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += out3;
    
    /* UNGE - "ae" flag (above or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r"(out4)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += out4;
    
    /* UNGT - "a" flag (above) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(out5)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += out5;
    
    /* UNLE - "be" flag (below or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(out6)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += out6;
    
    /* UNLT - "b" flag (below) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(out7)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += out7;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(out8)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += out8;
    
    return result;
}

__attribute__((noinline))
int test_mixed_operations(float* farr, double* darr, int n) {
    int checksum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Mix scalar and vector-like operations */
        float f1 = farr[i];
        float f2 = farr[(i + 1) % n];
        double d1 = darr[i];
        double d2 = darr[(i + 1) % n];
        
        /* Force generation of various condition codes */
        checksum += __builtin_isunordered(f1, f2) ? 1 : 0;
        checksum += __builtin_isgreater(f1, f2) ? 2 : 0;
        checksum += __builtin_isless(f1, f2) ? 4 : 0;
        checksum += __builtin_islessequal(f1, f2) ? 8 : 0;
        checksum += __builtin_islessgreater(d1, d2) ? 16 : 0;
        
        /* Use fpclassify to generate condition codes */
        int c1 = fpclassify(f1);
        int c2 = fpclassify(f2);
        checksum += (c1 == FP_NAN || c2 == FP_NAN) ? 32 : 0;
        checksum += (c1 == FP_INFINITE || c2 == FP_INFINITE) ? 64 : 0;
    }
    
    return checksum;
}

int main(void) {
    const int N = 64;
    float farr[N];
    double darr[N];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < N; i++) {
        farr[i] = (i % 8 == 0) ? NAN : (float)(i * 1.5);
        darr[i] = (i % 7 == 0) ? NAN : (double)(i * 2.3);
    }
    
    /* Test all condition code generation paths */
    int result = 0;
    
    /* 1. Scalar builtins */
    result += test_scalar_builtins(farr[0], farr[1], darr[0], darr[1]);
    
    /* 2. SSE intrinsics */
    result += test_sse_intrinsics(farr, farr + 1, N - 1);
    
    /* 3. Inline assembly */
    result += test_inline_asm(farr[2], farr[3]);
    
    /* 4. Mixed operations */
    result += test_mixed_operations(farr, darr, N);
    
    /* Print result to prevent optimization */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
