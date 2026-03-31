/* test_condition_codes.c - Cover GCC i386.cc condition code output routines */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to ensure code generation */
#define NOINLINE __attribute__((noinline))

/* Global to prevent optimization */
volatile int global_result = 0;

/* Function 1: Scalar builtins with various condition codes */
NOINLINE int test_scalar_builtins(float a, float b) {
    int result = 0;
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* ORDERED case */
    if (!__builtin_isunordered(a, b)) {
        result |= 2;
    }
    
    /* UNEQ (unordered or equal) - using isnan and equality */
    if (__builtin_isunordered(a, b) || a == b) {
        result |= 4;
    }
    
    /* UNGE (not less than) - using !__builtin_isless */
    if (!__builtin_isless(a, b)) {
        result |= 8;
    }
    
    /* UNGT (not less than or equal) - using !__builtin_islessequal */
    if (!__builtin_islessequal(a, b)) {
        result |= 16;
    }
    
    /* UNLE (unordered or less than or equal) */
    if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
        result |= 32;
    }
    
    /* UNLT (unordered or less than) */
    if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
        result |= 64;
    }
    
    /* LTGT (less than or greater than, but not equal and not unordered) */
    if ((a < b) || (a > b)) {
        result |= 128;
    }
    
    return result;
}

/* Function 2: SSE intrinsics with explicit comparison predicates */
NOINLINE __m128 test_sse_intrinsics(__m128 a, __m128 b) {
    __m128 result = _mm_setzero_ps();
    
    /* _CMP_UNORD_Q - UNORDERED */
    result = _mm_or_ps(result, _mm_cmpunord_ps(a, b));
    
    /* _CMP_ORD_Q - ORDERED */
    result = _mm_or_ps(result, _mm_cmpord_ps(a, b));
    
    /* _CMP_EQ_UQ - UNEQ (unordered or equal) */
    result = _mm_or_ps(result, _mm_cmpneq_ps(a, b));
    
    /* _CMP_NLT_UQ - UNGE (not less than) */
    result = _mm_or_ps(result, _mm_cmpnlt_ps(a, b));
    
    /* _CMP_NLE_UQ - UNGT (not less than or equal) */
    result = _mm_or_ps(result, _mm_cmpnle_ps(a, b));
    
    /* _CMP_LE_OQ - UNLE (ordered less than or equal) */
    result = _mm_or_ps(result, _mm_cmple_ps(a, b));
    
    /* _CMP_LT_OQ - UNLT (ordered less than) */
    result = _mm_or_ps(result, _mm_cmplt_ps(a, b));
    
    /* _CMP_NEQ_OQ - LTGT (ordered not equal) */
    result = _mm_or_ps(result, _mm_cmpneq_ps(a, b));
    
    return result;
}

/* Function 3: Inline assembly with condition code constraints */
NOINLINE int test_inline_asm(float a, float b) {
    int result_u, result_o, result_ueq, result_nlt, result_nle;
    int result_ule, result_ult, result_une;
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(result_u) : : "cc");
    
    /* ORDERED - "no" flag (not overflow, but we need ordered) */
    /* Using fucomip to set flags, then testing ordered condition */
    int ordered_result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "setnp %0\n\t"
        : "=r"(ordered_result)
        : 
        : "cc"
    );
    
    /* UNEQ - "e" flag after unordered check */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0\n\t"
        : "=r"(result_ueq)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* UNGE - "ae" flag (above or equal) for not less than */
    asm volatile ("setae %0" : "=r"(result_nlt) : : "cc");
    
    /* UNGT - "a" flag (above) for not less than or equal */
    asm volatile ("seta %0" : "=r"(result_nle) : : "cc");
    
    /* UNLE - "be" flag (below or equal) */
    asm volatile ("setbe %0" : "=r"(result_ule) : : "cc");
    
    /* UNLT - "b" flag (below) */
    asm volatile ("setb %0" : "=r"(result_ult) : : "cc");
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(result_une) : : "cc");
    
    return result_u + ordered_result + result_ueq + result_nlt + 
           result_nle + result_ule + result_ult + result_une;
}

/* Function 4: Loop with mixed comparisons to prevent optimization */
NOINLINE float test_loop_comparisons(const float* arr1, const float* arr2, int n) {
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        float a = arr1[i];
        float b = arr2[i];
        
        /* Mix different condition checks */
        if (__builtin_isunordered(a, b)) {
            sum += 1.0f;
        }
        if (!__builtin_isless(a, b)) {  /* UNGE */
            sum += 2.0f;
        }
        if (!__builtin_islessequal(a, b)) {  /* UNGT */
            sum += 3.0f;
        }
        if (__builtin_isunordered(a, b) || a == b) {  /* UNEQ */
            sum += 4.0f;
        }
    }
    
    return sum;
}

/* Function 5: Double precision comparisons */
NOINLINE int test_double_comparisons(double a, double b) {
    int result = 0;
    
    /* Generate various condition codes with doubles */
    result |= __builtin_isunordered(a, b) ? 1 : 0;
    result |= (!__builtin_isless(a, b)) ? 2 : 0;      /* UNGE */
    result |= (!__builtin_islessequal(a, b)) ? 4 : 0; /* UNGT */
    result |= (__builtin_isunordered(a, b) || a == b) ? 8 : 0; /* UNEQ */
    
    /* LTGT: not equal and ordered */
    if (a != b && !__builtin_isunordered(a, b)) {
        result |= 16;
    }
    
    return result;
}

int main() {
    /* Create test data with NaN values to trigger unordered comparisons */
    float f1 = 1.0f;
    float f2 = 2.0f;
    float f_nan = NAN;
    float f_inf = INFINITY;
    
    double d1 = 1.0;
    double d2 = 2.0;
    double d_nan = NAN;
    
    /* Test 1: Scalar builtins */
    int r1 = test_scalar_builtins(f1, f2);
    int r2 = test_scalar_builtins(f_nan, f2);
    int r3 = test_scalar_builtins(f1, f_nan);
    int r4 = test_scalar_builtins(f_nan, f_nan);
    
    /* Test 2: SSE intrinsics */
    __m128 sse_a = _mm_setr_ps(1.0f, 2.0f, f_nan, 4.0f);
    __m128 sse_b = _mm_setr_ps(4.0f, 2.0f, 3.0f, f_nan);
    __m128 sse_result = test_sse_intrinsics(sse_a, sse_b);
    
    /* Test 3: Inline assembly */
    int asm_result = test_inline_asm(f1, f2);
    asm_result += test_inline_asm(f_nan, f2);
    
    /* Test 4: Loop with array comparisons */
    float arr1[100];
    float arr2[100];
    for (int i = 0; i < 100; i++) {
        arr1[i] = (i % 10 == 0) ? f_nan : (float)i;
        arr2[i] = (i % 7 == 0) ? f_nan : (float)(i * 2);
    }
    float loop_sum = test_loop_comparisons(arr1, arr2, 100);
    
    /* Test 5: Double precision */
    int d_result1 = test_double_comparisons(d1, d2);
    int d_result2 = test_double_comparisons(d_nan, d2);
    int d_result3 = test_double_comparisons(d1, d_nan);
    
    /* Combine results to prevent optimization */
    global_result = r1 + r2 + r3 + r4 + asm_result + 
                   (int)loop_sum + d_result1 + d_result2 + d_result3;
    
    /* Extract some value from SSE result */
    float sse_vals[4];
    _mm_storeu_ps(sse_vals, sse_result);
    global_result += (int)(sse_vals[0] + sse_vals[1] + sse_vals[2] + sse_vals[3]);
    
    printf("Result: %d\n", global_result);
    
    return 0;
}
