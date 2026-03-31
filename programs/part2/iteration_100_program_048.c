/* test_condition_codes.c - Cover GCC i386 condition code output routines */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining to ensure code generation */
#define NOINLINE __attribute__((noinline))

/* Global to prevent optimization */
volatile int global_result = 0;

/* ========== Scalar builtins ========== */
NOINLINE int test_scalar_unordered(float a, float b) {
    int result = 0;
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) result |= 1;
    /* ORDERED case */
    if (!__builtin_isunordered(a, b)) result |= 2;
    /* UNEQ case (unordered or equal) - use isnan check */
    if (isnan(a) || isnan(b) || a == b) result |= 4;
    /* UNGE case (not less than) */
    if (__builtin_isgreaterequal(a, b)) result |= 8;
    /* UNGT case (not less or equal) */
    if (__builtin_isgreater(a, b)) result |= 16;
    /* UNLE case (unordered or less or equal) */
    if (__builtin_islessequal(a, b)) result |= 32;
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(a, b)) result |= 64;
    /* LTGT case (less than or greater than, but not equal) */
    if (a != b && !isnan(a) && !isnan(b)) result |= 128;
    return result;
}

/* ========== SSE intrinsics ========== */
NOINLINE int test_sse_intrinsics(float *a, float *b, int n) {
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED: _mm_cmpunord_ps */
        __m128 unord_mask = _mm_cmpunord_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(unord_mask, _mm_set1_ps(1.0f)));
        
        /* ORDERED: _mm_cmpord_ps */
        __m128 ord_mask = _mm_cmpord_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(ord_mask, _mm_set1_ps(2.0f)));
        
        /* UNGE: _mm_cmpnlt_ps (not less than) */
        __m128 nlt_mask = _mm_cmpnlt_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(nlt_mask, _mm_set1_ps(4.0f)));
        
        /* UNGT: _mm_cmpnle_ps (not less or equal) */
        __m128 nle_mask = _mm_cmpnle_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(nle_mask, _mm_set1_ps(8.0f)));
        
        /* UNLE: _mm_cmple_ps (less or equal) - unordered version */
        __m128 le_mask = _mm_cmple_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(le_mask, _mm_set1_ps(16.0f)));
        
        /* UNLT: _mm_cmplt_ps (less than) - unordered version */
        __m128 lt_mask = _mm_cmplt_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(lt_mask, _mm_set1_ps(32.0f)));
        
        /* LTGT: _mm_cmpneq_ps (not equal) - but we need une/ltgt */
        __m128 neq_mask = _mm_cmpneq_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(neq_mask, _mm_set1_ps(64.0f)));
    }
    
    /* Horizontal sum */
    float temp[4];
    _mm_storeu_ps(temp, sum);
    return (int)(temp[0] + temp[1] + temp[2] + temp[3]);
}

/* ========== Inline assembly with condition codes ========== */
NOINLINE int test_inline_asm(float a, float b) {
    int result = 0;
    int r;
    
    /* UNORDERED: "u" flag */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setu %0"
                  : "=r"(r) : "x"(a), "x"(b));
    if (r) result |= 1;
    
    /* ORDERED: "no" flag (not unordered) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setno %0"
                  : "=r"(r) : "x"(a), "x"(b));
    if (r) result |= 2;
    
    /* UNEQ: "e" flag (equal) - unordered equal */
    asm volatile ("ucomiss %1, %2\n\t"
                  "sete %0"
                  : "=r"(r) : "x"(a), "x"(b));
    if (r) result |= 4;
    
    /* UNGE: "ae" flag (above or equal) - not less than */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setae %0"
                  : "=r"(r) : "x"(a), "x"(b));
    if (r) result |= 8;
    
    /* UNGT: "a" flag (above) - not less or equal */
    asm volatile ("ucomiss %1, %2\n\t"
                  "seta %0"
                  : "=r"(r) : "x"(a), "x"(b));
    if (r) result |= 16;
    
    /* UNLE: "be" flag (below or equal) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setbe %0"
                  : "=r"(r) : "x"(a), "x"(b));
    if (r) result |= 32;
    
    /* UNLT: "b" flag (below) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setb %0"
                  : "=r"(r) : "x"(a), "x"(b));
    if (r) result |= 64;
    
    /* LTGT: "ne" flag (not equal) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setne %0"
                  : "=r"(r) : "x"(a), "x"(b));
    if (r) result |= 128;
    
    return result;
}

/* ========== Mixed tests with NaN values ========== */
NOINLINE int test_mixed_nan(float *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            /* Mix different comparisons */
            if (__builtin_isunordered(arr[i], arr[j])) sum++;
            if (!__builtin_isunordered(arr[i], arr[j])) sum++;
            if (__builtin_isgreater(arr[i], arr[j])) sum++;
            if (__builtin_isless(arr[i], arr[j])) sum++;
            if (__builtin_isgreaterequal(arr[i], arr[j])) sum++;
            if (__builtin_islessequal(arr[i], arr[j])) sum++;
        }
    }
    return sum;
}

/* ========== Main test driver ========== */
int main() {
    /* Create arrays with NaN values to trigger unordered comparisons */
    const int SIZE = 64;
    float arr1[SIZE], arr2[SIZE];
    
    /* Fill with mixed values: normal numbers, NaN, infinity */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i % 10 == 0) ? NAN : 
                  (i % 7 == 0) ? INFINITY : 
                  (float)(i * 1.5f);
        arr2[i] = (i % 11 == 0) ? NAN : 
                  (i % 5 == 0) ? -INFINITY : 
                  (float)(i * 0.7f);
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_unordered(arr1[0], arr2[0]);
    checksum += test_scalar_unordered(NAN, 1.0f);
    checksum += test_scalar_unordered(1.0f, NAN);
    checksum += test_scalar_unordered(INFINITY, -INFINITY);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(arr1, arr2, SIZE);
    
    /* Test inline assembly */
    checksum += test_inline_asm(arr1[1], arr2[1]);
    checksum += test_inline_asm(NAN, arr2[2]);
    checksum += test_inline_asm(arr1[3], NAN);
    
    /* Test mixed NaN comparisons */
    checksum += test_mixed_nan(arr1, 8);
    
    /* Store to volatile to prevent optimization */
    global_result = checksum;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    return 0;
}
