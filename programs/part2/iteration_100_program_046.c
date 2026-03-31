/* test_condition_codes.c - Cover all floating-point condition code cases in i386.cc */
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

/* Function 1: Scalar builtins covering various condition codes */
NOINLINE int test_scalar_builtins(float a, float b) {
    int result = 0;
    
    /* UNORDERED - __builtin_isunordered */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* ORDERED - !__builtin_isunordered */
    if (!__builtin_isunordered(a, b)) {
        result |= 2;
    }
    
    /* UNEQ - unordered or equal (builtin doesn't exist directly, but compiler may generate) */
    if (a == b || __builtin_isunordered(a, b)) {
        result |= 4;
    }
    
    /* UNGE - not less than (greater or equal or unordered) */
    if (!(a < b)) {
        result |= 8;
    }
    
    /* UNGT - not less than or equal (greater or unordered) */
    if (!(a <= b)) {
        result |= 16;
    }
    
    /* UNLE - less than or equal or unordered */
    if (a <= b || __builtin_isunordered(a, b)) {
        result |= 32;
    }
    
    /* UNLT - less than or unordered */
    if (a < b || __builtin_isunordered(a, b)) {
        result |= 64;
    }
    
    /* LTGT - less than or greater than (ordered and not equal) */
    if ((a < b) || (a > b)) {
        result |= 128;
    }
    
    return result;
}

/* Function 2: SSE intrinsics for vector comparisons */
NOINLINE int test_sse_intrinsics(float *a, float *b, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _mm_cmpunord_ps */
        __m128 mask_unord = _mm_cmpunord_ps(va, vb);
        int unord_mask = _mm_movemask_ps(mask_unord);
        result += unord_mask;
        
        /* ORDERED - _mm_cmpord_ps */
        __m128 mask_ord = _mm_cmpord_ps(va, vb);
        int ord_mask = _mm_movemask_ps(mask_ord);
        result += ord_mask;
        
        /* UNGE - _mm_cmpnlt_ps (not less than) */
        __m128 mask_nlt = _mm_cmpnlt_ps(va, vb);
        int nlt_mask = _mm_movemask_ps(mask_nlt);
        result += nlt_mask;
        
        /* UNGT - _mm_cmpnle_ps (not less than or equal) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        int nle_mask = _mm_movemask_ps(mask_nle);
        result += nle_mask;
        
        /* UNLE - _mm_cmple_ps (less than or equal) - compiler may use UNLE */
        __m128 mask_le = _mm_cmple_ps(va, vb);
        int le_mask = _mm_movemask_ps(mask_le);
        result += le_mask;
        
        /* UNLT - _mm_cmplt_ps (less than) - compiler may use UNLT */
        __m128 mask_lt = _mm_cmplt_ps(va, vb);
        int lt_mask = _mm_movemask_ps(mask_lt);
        result += lt_mask;
        
        /* NEQ - _mm_cmpneq_ps (not equal) - may use LTGT or UNEQ */
        __m128 mask_neq = _mm_cmpneq_ps(va, vb);
        int neq_mask = _mm_movemask_ps(mask_neq);
        result += neq_mask;
    }
    
    return result;
}

/* Function 3: Inline assembly with condition code constraints */
NOINLINE int test_inline_asm(float a, float b) {
    int result = 0;
    int r;
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(r) : : "cc");
    result += r;
    
    /* ORDERED - "no" flag (not overflow, but we need ordered) 
       Using multiple conditions to cover different cases */
    asm volatile ("setnp %0" : "=r"(r) : : "cc");  /* not parity (ordered) */
    result += r;
    
    /* UNEQ - unordered or equal */
    asm volatile ("sete %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNGE - not less than */
    asm volatile ("setnl %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNGT - not less than or equal */
    asm volatile ("setnle %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNLE - less than or equal or unordered */
    asm volatile ("setle %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNLT - less than or unordered */
    asm volatile ("setl %0" : "=r"(r) : : "cc");
    result += r;
    
    /* LTGT - less than or greater than */
    asm volatile ("setne %0" : "=r"(r) : : "cc");
    result += r;
    
    return result;
}

/* Function 4: Mixed comparisons with NaN values */
NOINLINE int test_nan_comparisons(float *arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        float x = arr[i];
        
        /* Compare with NaN to trigger unordered conditions */
        if (__builtin_isunordered(x, NAN)) {
            result++;
        }
        
        if (!__builtin_isunordered(x, 0.0f)) {
            result++;
        }
        
        /* Various comparisons that may generate different condition codes */
        if (x > 0.0f) {  /* May generate UNGT with -ffast-math */
            result++;
        }
        
        if (x >= 0.0f) { /* May generate UNGE */
            result++;
        }
        
        if (x < 0.0f) {  /* May generate UNLT */
            result++;
        }
        
        if (x <= 0.0f) { /* May generate UNLE */
            result++;
        }
        
        if (x != 0.0f) { /* May generate LTGT */
            result++;
        }
    }
    
    return result;
}

/* Function 5: Double precision comparisons */
NOINLINE int test_double_comparisons(double a, double b) {
    int result = 0;
    
    /* Force generation of condition codes for doubles */
    result += __builtin_isgreater(a, b) ? 1 : 0;
    result += __builtin_isgreaterequal(a, b) ? 2 : 0;
    result += __builtin_isless(a, b) ? 4 : 0;
    result += __builtin_islessequal(a, b) ? 8 : 0;
    result += __builtin_islessgreater(a, b) ? 16 : 0;
    result += __builtin_isunordered(a, b) ? 32 : 0;
    
    return result;
}

int main() {
    /* Initialize arrays with mix of normal values and NaN */
    const int N = 64;
    float arr1[N], arr2[N];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = (i % 2 == 0) ? (float)i : NAN;
        arr2[i] = (i % 3 == 0) ? (float)(i * 2) : NAN;
    }
    
    /* Test all functions to trigger different condition codes */
    int checksum = 0;
    
    checksum += test_scalar_builtins(arr1[0], arr2[0]);
    checksum += test_scalar_builtins(NAN, 1.0f);
    checksum += test_scalar_builtins(1.0f, NAN);
    checksum += test_scalar_builtins(1.0f, 2.0f);
    checksum += test_scalar_builtins(2.0f, 1.0f);
    checksum += test_scalar_builtins(1.0f, 1.0f);
    
    checksum += test_sse_intrinsics(arr1, arr2, N);
    
    checksum += test_inline_asm(arr1[0], arr2[0]);
    
    checksum += test_nan_comparisons(arr1, N);
    
    checksum += test_double_comparisons(1.0, 2.0);
    checksum += test_double_comparisons(NAN, 2.0);
    checksum += test_double_comparisons(2.0, NAN);
    checksum += test_double_comparisons(NAN, NAN);
    
    /* Store to volatile to prevent optimization */
    global_result = checksum;
    
    printf("Result: %d\n", checksum);
    
    return 0;
}
