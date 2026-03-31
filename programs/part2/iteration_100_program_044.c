/* test_condition_codes.c - Cover GCC i386 condition code output routines */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining to ensure code generation */
#define NOINLINE __attribute__((noinline))

/* Global volatile to prevent optimization */
volatile int global_result = 0;

/* ========== Scalar builtins for condition codes ========== */
NOINLINE int test_scalar_unordered(float a, float b) {
    int result = 0;
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) result |= 1;
    /* ORDERED case - via !unordered */
    if (!__builtin_isunordered(a, b)) result |= 2;
    return result;
}

NOINLINE int test_scalar_comparisons(float a, float b) {
    int result = 0;
    /* Various comparisons that may generate different condition codes */
    if (__builtin_isgreater(a, b)) result |= 1;    /* May generate UNGT or GT */
    if (__builtin_isless(a, b)) result |= 2;       /* May generate UNLT or LT */
    if (__builtin_isgreaterequal(a, b)) result |= 4; /* May generate UNGE or GE */
    if (__builtin_islessequal(a, b)) result |= 8;    /* May generate UNLE or LE */
    return result;
}

NOINLINE int test_scalar_unequal(float a, float b) {
    int result = 0;
    /* LTGT case - not equal and ordered */
    if (a != b && !__builtin_isunordered(a, b)) result |= 1;
    /* UNEQ case - equal or unordered */
    if (a == b || __builtin_isunordered(a, b)) result |= 2;
    return result;
}

/* ========== SSE/AVX intrinsics for condition codes ========== */
NOINLINE int test_sse_comparisons(float *a, float *b, int n) {
    int result = 0;
    __m128 va, vb, vcmp;
    
    for (int i = 0; i < n; i += 4) {
        va = _mm_loadu_ps(&a[i]);
        vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _CMP_UNORD_Q */
        vcmp = _mm_cmpunord_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* ORDERED - _CMP_ORD_Q */
        vcmp = _mm_cmpord_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNGE - _CMP_NLT_UQ (not less than, unordered quiet) */
        vcmp = _mm_cmpnlt_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNGT - _CMP_NLE_UQ (not less than or equal, unordered quiet) */
        vcmp = _mm_cmpnle_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNLE - _CMP_LE_UQ (less than or equal, unordered quiet) */
        vcmp = _mm_cmple_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNLT - _CMP_LT_UQ (less than, unordered quiet) */
        vcmp = _mm_cmplt_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* UNEQ - _CMP_EQ_UQ (equal, unordered quiet) */
        vcmp = _mm_cmpeq_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
        
        /* LTGT - _CMP_NEQ_UQ (not equal, unordered quiet) */
        vcmp = _mm_cmpneq_ps(va, vb);
        result += _mm_movemask_ps(vcmp);
    }
    
    return result;
}

/* ========== Inline assembly with condition codes ========== */
NOINLINE int test_asm_condition_codes(float a, float b) {
    int result = 0;
    int r;
    
    /* Test various condition codes via inline asm */
    
    /* UNORDERED - "u" flag */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setu %0"
                  : "=r"(r) : "x"(a), "x"(b));
    result |= (r << 0);
    
    /* ORDERED - "no" flag (not unordered) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setno %0"
                  : "=r"(r) : "x"(a), "x"(b));
    result |= (r << 1);
    
    /* UNEQ - "e" flag (equal or unordered) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "sete %0"
                  : "=r"(r) : "x"(a), "x"(b));
    result |= (r << 2);
    
    /* UNGE - "ae" flag (above or equal) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setae %0"
                  : "=r"(r) : "x"(a), "x"(b));
    result |= (r << 3);
    
    /* UNGT - "a" flag (above) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "seta %0"
                  : "=r"(r) : "x"(a), "x"(b));
    result |= (r << 4);
    
    /* UNLE - "be" flag (below or equal) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setbe %0"
                  : "=r"(r) : "x"(a), "x"(b));
    result |= (r << 5);
    
    /* UNLT - "b" flag (below) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setb %0"
                  : "=r"(r) : "x"(a), "x"(b));
    result |= (r << 6);
    
    /* LTGT - "ne" flag (not equal and ordered) */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setne %0"
                  : "=r"(r) : "x"(a), "x"(b));
    result |= (r << 7);
    
    return result;
}

/* ========== Mixed tests with NaN values ========== */
NOINLINE int test_nan_handling(void) {
    float nan_val = NAN;
    float inf_val = INFINITY;
    float normal_vals[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float nan_vals[] = {NAN, 5.0f, NAN, 6.0f};
    int result = 0;
    
    /* Test scalar with NaN */
    result += test_scalar_unordered(nan_val, 1.0f);
    result += test_scalar_unordered(1.0f, nan_val);
    result += test_scalar_unordered(nan_val, nan_val);
    result += test_scalar_unordered(1.0f, 2.0f);
    
    /* Test comparisons involving NaN */
    result += test_scalar_comparisons(nan_val, 1.0f);
    result += test_scalar_comparisons(1.0f, nan_val);
    result += test_scalar_comparisons(1.0f, 2.0f);
    
    /* Test unequal cases */
    result += test_scalar_unequal(nan_val, 1.0f);
    result += test_scalar_unequal(1.0f, 1.0f);
    result += test_scalar_unequal(1.0f, 2.0f);
    
    /* Test SSE with NaN arrays */
    result += test_sse_comparisons(normal_vals, nan_vals, 4);
    result += test_sse_comparisons(nan_vals, normal_vals, 4);
    result += test_sse_comparisons(nan_vals, nan_vals, 4);
    
    /* Test inline asm with NaN */
    result += test_asm_condition_codes(nan_val, 1.0f);
    result += test_asm_condition_codes(1.0f, nan_val);
    result += test_asm_condition_codes(1.0f, 2.0f);
    result += test_asm_condition_codes(2.0f, 1.0f);
    result += test_asm_condition_codes(1.0f, 1.0f);
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    int checksum = 0;
    
    printf("Testing GCC i386 condition code output routines...\n");
    
    /* Run comprehensive tests */
    checksum += test_nan_handling();
    
    /* Additional tests with different values */
    float test_cases[][2] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {0.0f, 1.0f},
        {-1.0f, 1.0f},
        {INFINITY, 1.0f},
        {1.0f, INFINITY},
        {-INFINITY, INFINITY}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        checksum += test_scalar_unordered(test_cases[i][0], test_cases[i][1]);
        checksum += test_scalar_comparisons(test_cases[i][0], test_cases[i][1]);
        checksum += test_scalar_unequal(test_cases[i][0], test_cases[i][1]);
        checksum += test_asm_condition_codes(test_cases[i][0], test_cases[i][1]);
    }
    
    /* Test with arrays */
    float arr1[16], arr2[16];
    for (int i = 0; i < 16; i++) {
        arr1[i] = (float)i;
        arr2[i] = (i % 4 == 0) ? NAN : (float)(i * 2);
    }
    checksum += test_sse_comparisons(arr1, arr2, 16);
    
    /* Store result to prevent optimization */
    global_result = checksum;
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
