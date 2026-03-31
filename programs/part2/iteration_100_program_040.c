/* test_condition_codes.c - Cover GCC i386 condition code output routines */

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
    if (!__builtin_isgreater(c, d) && !__builtin_isless(c, d)) {
        result |= 4;
    }
    
    /* UNGE case - not less than (nlt) */
    if (!__builtin_isless(a, b)) {
        result |= 8;
    }
    
    /* UNGT case - not less or equal (nle) */
    if (!__builtin_islessequal(c, d)) {
        result |= 16;
    }
    
    /* UNLE case - unordered or less or equal (ule) */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) {
        result |= 32;
    }
    
    /* UNLT case - unordered or less than (ult) */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
        result |= 64;
    }
    
    /* LTGT case - less than or greater than (une/neq) */
    if (__builtin_isless(c, d) || __builtin_isgreater(c, d)) {
        result |= 128;
    }
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float* arr1, float* arr2, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 a = _mm_loadu_ps(&arr1[i]);
        __m128 b = _mm_loadu_ps(&arr2[i]);
        
        /* UNORDERED - _CMP_UNORD_Q */
        __m128 cmp_unord = _mm_cmpunord_ps(a, b);
        
        /* ORDERED - _CMP_ORD_Q */
        __m128 cmp_ord = _mm_cmpord_ps(a, b);
        
        /* UNGE - _CMP_NLT_UQ (not less than) */
        __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);
        
        /* UNGT - _CMP_NLE_UQ (not less or equal) */
        __m128 cmp_nle = _mm_cmpnle_ps(a, b);
        
        /* UNLE - _CMP_LE_UQ (less or equal unordered) */
        __m128 cmp_ule = _mm_cmple_ps(a, b);
        
        /* UNLT - _CMP_LT_UQ (less than unordered) */
        __m128 cmp_ult = _mm_cmplt_ps(a, b);
        
        /* LTGT - _CMP_NEQ_UQ (not equal unordered) */
        __m128 cmp_neq = _mm_cmpneq_ps(a, b);
        
        /* UNEQ - _CMP_EQ_UQ (equal unordered) */
        __m128 cmp_ueq = _mm_cmpeq_ps(a, b);
        
        /* Combine results */
        __m128 combined = _mm_add_ps(cmp_unord, cmp_ord);
        combined = _mm_add_ps(combined, cmp_nlt);
        combined = _mm_add_ps(combined, cmp_nle);
        combined = _mm_add_ps(combined, cmp_ule);
        combined = _mm_add_ps(combined, cmp_ult);
        combined = _mm_add_ps(combined, cmp_neq);
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
    
    /* Use inline assembly with condition code constraints */
    
    /* UNORDERED - "u" flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setu %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* ORDERED - "np" flag (no parity) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNEQ - "e" flag (equal) - unordered equal */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNGE - "ae" flag (above or equal) - not less than */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNGT - "a" flag (above) - not less or equal */
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
int test_mixed_comparisons(double* darr1, double* darr2, int n) {
    int checksum = 0;
    
    for (int i = 0; i < n; i++) {
        double a = darr1[i];
        double b = darr2[i];
        
        /* Generate various condition codes through different comparisons */
        
        /* Using fpclassify to trigger condition code generation */
        int ca = fpclassify(a);
        int cb = fpclassify(b);
        
        if (ca == FP_NAN || cb == FP_NAN) {
            checksum += 1;  /* UNORDERED */
        }
        
        if (ca != FP_NAN && cb != FP_NAN) {
            checksum += 2;  /* ORDERED */
        }
        
        /* Direct comparisons that may generate specific condition codes */
        if (!(a < b) && !(a > b)) {
            checksum += 4;  /* UNEQ */
        }
        
        if (!(a < b)) {
            checksum += 8;  /* UNGE */
        }
        
        if (!(a <= b)) {
            checksum += 16; /* UNGT */
        }
        
        if ((a <= b) || (isnan(a) || isnan(b))) {
            checksum += 32; /* UNLE */
        }
        
        if ((a < b) || (isnan(a) || isnan(b))) {
            checksum += 64; /* UNLT */
        }
        
        if ((a < b) || (a > b)) {
            checksum += 128; /* LTGT */
        }
    }
    
    return checksum;
}

int main() {
    const int SIZE = 64;
    float farr1[SIZE], farr2[SIZE];
    double darr1[SIZE], darr2[SIZE];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < SIZE; i++) {
        farr1[i] = (i % 8 == 0) ? NAN : (float)i;
        farr2[i] = (i % 5 == 0) ? NAN : (float)(SIZE - i);
        
        darr1[i] = (i % 7 == 0) ? NAN : (double)i * 0.5;
        darr2[i] = (i % 6 == 0) ? NAN : (double)(SIZE - i) * 0.5;
    }
    
    int total_checksum = 0;
    
    /* Test scalar builtins */
    total_checksum += test_scalar_builtins(farr1[0], farr2[0], 
                                          darr1[0], darr2[0]);
    
    /* Test SSE intrinsics */
    total_checksum += test_sse_intrinsics(farr1, farr2, SIZE);
    
    /* Test inline assembly */
    total_checksum += test_inline_asm(farr1[1], farr2[1]);
    
    /* Test mixed comparisons */
    total_checksum += test_mixed_comparisons(darr1, darr2, SIZE);
    
    /* Print result to prevent optimization */
    printf("Total checksum: %d\n", total_checksum);
    
    return 0;
}
