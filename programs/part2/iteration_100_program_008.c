/* Test program to cover condition code output in i386.cc */
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
    
    /* UNEQ case (unordered or equal) - use with fast-math */
    if (!(a > b) && !(a < b)) result |= 4;  /* May become UNEQ with fast-math */
    
    /* UNGE case (not less than) */
    if (__builtin_isgreaterequal(a, b)) result |= 8;
    
    /* UNGT case (not less than or equal) */
    if (__builtin_isgreater(a, b)) result |= 16;
    
    /* UNLE case (unordered or less than or equal) */
    if (__builtin_islessequal(a, b)) result |= 32;
    
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(a, b)) result |= 64;
    
    /* LTGT case (less than or greater than, ordered) */
    if (a != b && !__builtin_isunordered(a, b)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_vector_intrinsics(float *a, float *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _CMP_UNORD_Q */
        __m128 mask_unord = _mm_cmpunord_ps(va, vb);
        sum += _mm_movemask_ps(mask_unord);
        
        /* ORDERED - _CMP_ORD_Q */
        __m128 mask_ord = _mm_cmpord_ps(va, vb);
        sum += _mm_movemask_ps(mask_ord);
        
        /* UNGE - _CMP_NLT_UQ (not less than, unordered quiet) */
        __m128 mask_nlt = _mm_cmpnlt_ps(va, vb);
        sum += _mm_movemask_ps(mask_nlt);
        
        /* UNGT - _CMP_NLE_UQ (not less than or equal, unordered quiet) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        sum += _mm_movemask_ps(mask_nle);
        
        /* UNLE - _CMP_LE_UQ (less than or equal, unordered quiet) */
        __m128 mask_ule = _mm_cmple_ps(va, vb);
        sum += _mm_movemask_ps(mask_ule);
        
        /* UNLT - _CMP_LT_UQ (less than, unordered quiet) */
        __m128 mask_ult = _mm_cmplt_ps(va, vb);
        sum += _mm_movemask_ps(mask_ult);
        
        /* LTGT - _CMP_NEQ_UQ (not equal, unordered quiet) */
        __m128 mask_une = _mm_cmpneq_ps(va, vb);
        sum += _mm_movemask_ps(mask_une);
        
        /* UNEQ - _CMP_EQ_UQ (equal, unordered quiet) */
        __m128 mask_ueq = _mm_cmpeq_ps(va, vb);
        sum += _mm_movemask_ps(mask_ueq);
    }
    
    return sum;
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result_u = 0, result_g = 0, result_l = 0;
    int result_ge = 0, result_le = 0, result_ne = 0;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED - "u" flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setu %0"
        : "=r"(result_u)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* UNGT - "a" flag (above, for unordered comparisons) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(result_g)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* UNLT - "b" flag (below, for unordered comparisons) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(result_l)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* UNGE - "ae" flag (above or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r"(result_ge)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* UNLE - "be" flag (below or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(result_le)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(result_ne)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    return result_u + result_g + result_l + result_ge + result_le + result_ne;
}

__attribute__((noinline))
int test_mixed_comparisons(float *arr, int n) {
    int checksum = 0;
    
    for (int i = 0; i < n - 1; i++) {
        /* Mix of comparisons that should generate different condition codes */
        checksum += (__builtin_isunordered(arr[i], arr[i+1]) ? 1 : 0);
        checksum += (__builtin_isgreater(arr[i], arr[i+1]) ? 2 : 0);
        checksum += (__builtin_isless(arr[i], arr[i+1]) ? 4 : 0);
        checksum += (__builtin_isgreaterequal(arr[i], arr[i+1]) ? 8 : 0);
        checksum += (__builtin_islessequal(arr[i], arr[i+1]) ? 16 : 0);
        
        /* Use fpclassify to trigger ordered/unordered checks */
        if (fpclassify(arr[i]) == FP_NAN) checksum += 32;
        if (fpclassify(arr[i+1]) == FP_INFINITE) checksum += 64;
    }
    
    return checksum;
}

int main() {
    const int SIZE = 64;
    float arr1[SIZE], arr2[SIZE];
    
    /* Initialize with mix of normal values, NaN, and Inf */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i % 8 == 0) ? NAN : 
                  (i % 8 == 1) ? INFINITY :
                  (i % 8 == 2) ? -INFINITY :
                  (float)(i * 1.5);
        
        arr2[i] = (i % 7 == 0) ? NAN :
                  (i % 7 == 1) ? INFINITY :
                  (float)(i * 0.7);
    }
    
    int total = 0;
    
    /* Test scalar builtins with various inputs */
    total += test_scalar_builtins(arr1[0], arr2[0], arr1[1], arr2[1]);
    total += test_scalar_builtins(1.0f, 2.0f, NAN, NAN);
    total += test_scalar_builtins(INFINITY, -INFINITY, 0.0f, -0.0f);
    
    /* Test vector intrinsics */
    total += test_vector_intrinsics(arr1, arr2, SIZE);
    
    /* Test inline assembly */
    total += test_inline_asm(arr1[3], arr2[3]);
    total += test_inline_asm(NAN, 1.0f);
    total += test_inline_asm(1.0f, NAN);
    
    /* Test mixed comparisons */
    total += test_mixed_comparisons(arr1, SIZE);
    total += test_mixed_comparisons(arr2, SIZE);
    
    /* Use the result to prevent optimization */
    printf("Result checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
