/* test_condition_codes.c - Cover i386.cc condition code output routines */
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
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* ORDERED case */
    if (__builtin_isordered(c, d)) {
        result |= 2;
    }
    
    /* UNEQ case - unordered or equal */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
        result |= 4;
    }
    
    /* UNGE case - not less than (greater or equal or unordered) */
    if (!__builtin_isless(a, b)) {
        result |= 8;
    }
    
    /* UNGT case - not less or equal (greater or unordered) */
    if (!__builtin_islessequal(a, b)) {
        result |= 16;
    }
    
    /* UNLE case - less or equal or unordered */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) {
        result |= 32;
    }
    
    /* UNLT case - less than or unordered */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
        result |= 64;
    }
    
    /* LTGT case - less or greater (ordered and not equal) */
    if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) {
        result |= 128;
    }
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(const float* arr1, const float* arr2, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 a = _mm_loadu_ps(&arr1[i]);
        __m128 b = _mm_loadu_ps(&arr2[i]);
        
        /* UNORDERED - _CMP_UNORD_Q */
        __m128 cmp_unord = _mm_cmpunord_ps(a, b);
        
        /* ORDERED - _CMP_ORD_Q */
        __m128 cmp_ord = _mm_cmpord_ps(a, b);
        
        /* UNGE - _CMP_NLT_UQ (not less than, unordered quiet) */
        __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);
        
        /* UNGT - _CMP_NLE_UQ (not less or equal, unordered quiet) */
        __m128 cmp_nle = _mm_cmpnle_ps(a, b);
        
        /* UNLE - _CMP_LE_UQ (less or equal, unordered quiet) */
        __m128 cmp_ule = _mm_cmple_ps(a, b);
        
        /* UNLT - _CMP_LT_UQ (less than, unordered quiet) */
        __m128 cmp_ult = _mm_cmplt_ps(a, b);
        
        /* LTGT - _CMP_NEQ_UQ (not equal, unordered quiet) */
        __m128 cmp_une = _mm_cmpneq_ps(a, b);
        
        /* UNEQ - _CMP_EQ_UQ (equal, unordered quiet) */
        __m128 cmp_ueq = _mm_cmpeq_ps(a, b);
        
        /* Combine results */
        sum = _mm_add_ps(sum, cmp_unord);
        sum = _mm_add_ps(sum, cmp_ord);
        sum = _mm_add_ps(sum, cmp_nlt);
        sum = _mm_add_ps(sum, cmp_nle);
        sum = _mm_add_ps(sum, cmp_ule);
        sum = _mm_add_ps(sum, cmp_ult);
        sum = _mm_add_ps(sum, cmp_une);
        sum = _mm_add_ps(sum, cmp_ueq);
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
    
    /* Inline assembly with condition code constraints */
    /* UNORDERED - "u" flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setu %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* ORDERED - "np" flag (no parity = ordered) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNEQ - "e" flag (equal) with unordered */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNGE - "ae" flag (above or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNGT - "a" flag (above) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNLE - "be" flag (below or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNLT - "b" flag (below) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(float* farr, double* darr, int n) {
    int checksum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Use fpclassify to generate condition codes */
        int c1 = fpclassify(farr[i]);
        int c2 = fpclassify(darr[i % 8]);
        
        /* These comparisons may generate various condition codes */
        if (c1 == FP_NAN || c2 == FP_NAN) {
            checksum += 1;  /* May generate UNORDERED */
        }
        
        if (c1 == FP_INFINITE && c2 == FP_INFINITE) {
            checksum += 2;  /* May generate ORDERED comparisons */
        }
        
        /* Direct comparisons with NaN values */
        if (farr[i] != farr[i]) {  /* NaN comparison */
            checksum += 4;  /* Generates UNORDERED condition */
        }
        
        /* Ordered comparisons */
        if (farr[i] > 0.0f && darr[i % 8] > 0.0) {
            checksum += 8;  /* May generate UNGT/UNLT */
        }
    }
    
    return checksum;
}

int main() {
    /* Create arrays with mix of normal values and NaN */
    float farr[64];
    double darr[8];
    
    /* Initialize with pattern: normal, NaN, infinity, etc. */
    for (int i = 0; i < 64; i++) {
        switch (i % 4) {
            case 0: farr[i] = (float)i; break;
            case 1: farr[i] = NAN; break;
            case 2: farr[i] = INFINITY; break;
            case 3: farr[i] = -INFINITY; break;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        darr[i] = (i % 2 == 0) ? (double)i : NAN;
    }
    
    /* Call all test functions to generate various condition codes */
    int result = 0;
    
    /* Test scalar builtins with different value combinations */
    result += test_scalar_builtins(farr[0], farr[1], darr[0], darr[1]);
    result += test_scalar_builtins(farr[2], farr[3], darr[2], darr[3]);
    result += test_scalar_builtins(NAN, 1.0f, NAN, 2.0);
    
    /* Test SSE intrinsics */
    result += test_sse_intrinsics(farr, &farr[32], 32);
    
    /* Test inline assembly */
    result += test_inline_asm(farr[0], farr[1]);
    result += test_inline_asm(NAN, 1.0f);
    result += test_inline_asm(1.0f, NAN);
    
    /* Test mixed comparisons */
    result += test_mixed_comparisons(farr, darr, 64);
    
    /* Use result to prevent optimization */
    printf("Result checksum: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
