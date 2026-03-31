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
    
    /* UNEQ case - unordered or equal */
    if (__builtin_isunordered(a, b) || a == b) result |= 4;
    
    /* UNGE case - not less than (nlt) */
    if (!(a < b)) result |= 8;
    
    /* UNGT case - not less than or equal (nle) */
    if (!(a <= b)) result |= 16;
    
    /* UNLE case - unordered or less than or equal (ule) */
    if (__builtin_isunordered(a, b) || a <= b) result |= 32;
    
    /* UNLT case - unordered or less than (ult) */
    if (__builtin_isunordered(a, b) || a < b) result |= 64;
    
    /* LTGT case - less than or greater than (une) */
    if (a < b || a > b) result |= 128;
    
    return result;
}

/* ========== Vector intrinsics ========== */
NOINLINE int test_vector_intrinsics(float *a, float *b, int n) {
    __m128 va, vb, vcmp;
    int result = 0;
    
    for (int i = 0; i < n; i += 4) {
        va = _mm_loadu_ps(&a[i]);
        vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _mm_cmpunord_ps */
        vcmp = _mm_cmpunord_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* ORDERED - _mm_cmpord_ps */
        vcmp = _mm_cmpord_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNGE - _mm_cmpnlt_ps (not less than) */
        vcmp = _mm_cmpnlt_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNGT - _mm_cmpnle_ps (not less than or equal) */
        vcmp = _mm_cmpnle_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNLE - _mm_cmpnge_ps (not greater than or equal) */
        vcmp = _mm_cmpnge_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNLT - _mm_cmpngt_ps (not greater than) */
        vcmp = _mm_cmpngt_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* LTGT - _mm_cmpneq_ps (not equal) */
        vcmp = _mm_cmpneq_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
    }
    
    return result;
}

/* ========== Inline assembly ========== */
NOINLINE int test_inline_asm(float a, float b) {
    int result = 0;
    int tmp;
    
    /* UNORDERED - "u" flag */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setu %0"
                  : "=r"(tmp) : "x"(a), "x"(b));
    result += tmp;
    
    /* ORDERED - "nu" flag */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setnu %0"
                  : "=r"(tmp) : "x"(a), "x"(b));
    result += tmp;
    
    /* UNEQ - "e" flag (equal or unordered) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "sete %0"
                  : "=r"(tmp) : "x"(a), "x"(b));
    result += tmp;
    
    /* UNGE - "ae" flag (above or equal) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setae %0"
                  : "=r"(tmp) : "x"(a), "x"(b));
    result += tmp;
    
    /* UNGT - "a" flag (above) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "seta %0"
                  : "=r"(tmp) : "x"(a), "x"(b));
    result += tmp;
    
    /* UNLE - "be" flag (below or equal) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setbe %0"
                  : "=r"(tmp) : "x"(a), "x"(b));
    result += tmp;
    
    /* UNLT - "b" flag (below) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setb %0"
                  : "=r"(tmp) : "x"(a), "x"(b));
    result += tmp;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setne %0"
                  : "=r"(tmp) : "x"(a), "x"(b));
    result += tmp;
    
    return result;
}

/* ========== Mixed comparisons ========== */
NOINLINE int test_mixed_comparisons(float *arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n - 1; i++) {
        float a = arr[i];
        float b = arr[i + 1];
        
        /* Use various comparison styles */
        result += (__builtin_isgreater(a, b) ? 1 : 0);
        result += (__builtin_isless(a, b) ? 2 : 0);
        result += (__builtin_isgreaterequal(a, b) ? 4 : 0);
        result += (__builtin_islessequal(a, b) ? 8 : 0);
        
        /* Direct unordered checks */
        result += (isunordered(a, b) ? 16 : 0);
        result += (isnan(a) || isnan(b) ? 32 : 0);
        
        /* fpclassify usage */
        result += (fpclassify(a) == FP_NAN ? 64 : 0);
        result += (fpclassify(b) == FP_INFINITE ? 128 : 0);
    }
    
    return result;
}

/* ========== Main test driver ========== */
int main() {
    const int SIZE = 64;
    float arr1[SIZE], arr2[SIZE];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i % 3 == 0) ? NAN : (float)(i * 1.5);
        arr2[i] = (i % 5 == 0) ? NAN : (float)(i * 0.7);
    }
    
    /* Test scalar builtins */
    int r1 = test_scalar_unordered(arr1[0], arr2[0]);
    int r2 = test_scalar_unordered(NAN, 1.0f);
    int r3 = test_scalar_unordered(1.0f, NAN);
    int r4 = test_scalar_unordered(1.0f, 2.0f);
    int r5 = test_scalar_unordered(2.0f, 1.0f);
    int r6 = test_scalar_unordered(1.0f, 1.0f);
    
    /* Test vector intrinsics */
    int r7 = test_vector_intrinsics(arr1, arr2, SIZE);
    
    /* Test inline assembly */
    int r8 = test_inline_asm(arr1[0], arr2[0]);
    int r9 = test_inline_asm(1.0f, NAN);
    int r10 = test_inline_asm(NAN, 2.0f);
    
    /* Test mixed comparisons */
    int r11 = test_mixed_comparisons(arr1, SIZE);
    int r12 = test_mixed_comparisons(arr2, SIZE);
    
    /* Compute checksum to prevent optimization */
    int checksum = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
    global_result = checksum;
    
    printf("Result checksum: %d\n", checksum);
    return checksum == 0 ? 1 : 0;
}
