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

/* Function 1: Scalar builtins for various condition codes */
NOINLINE int test_scalar_builtins(float a, float b) {
    int result = 0;
    
    /* UNORDERED case - __builtin_isunordered */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* ORDERED case - opposite of unordered */
    if (!__builtin_isunordered(a, b)) {
        result |= 2;
    }
    
    /* UNEQ case - unordered or equal */
    if (__builtin_isunordered(a, b) || a == b) {
        result |= 4;
    }
    
    /* UNGE case - not less than (nlt) */
    if (!(a < b)) {
        result |= 8;
    }
    
    /* UNGT case - not less than or equal (nle) */
    if (!(a <= b)) {
        result |= 16;
    }
    
    /* UNLE case - unordered or less than or equal (ule) */
    if (__builtin_isunordered(a, b) || a <= b) {
        result |= 32;
    }
    
    /* UNLT case - unordered or less than (ult) */
    if (__builtin_isunordered(a, b) || a < b) {
        result |= 64;
    }
    
    /* LTGT case - less than or greater than (une) */
    if (a != b) {
        result |= 128;
    }
    
    return result;
}

/* Function 2: Vector intrinsics for SSE condition codes */
NOINLINE int test_vector_intrinsics(float *a, float *b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _mm_cmpunord_ps */
        __m128 unord_mask = _mm_cmpunord_ps(va, vb);
        
        /* ORDERED - _mm_cmpord_ps */
        __m128 ord_mask = _mm_cmpord_ps(va, vb);
        
        /* UNGE - _mm_cmpnlt_ps (not less than) */
        __m128 nlt_mask = _mm_cmpnlt_ps(va, vb);
        
        /* UNGT - _mm_cmpnle_ps (not less than or equal) */
        __m128 nle_mask = _mm_cmpnle_ps(va, vb);
        
        /* UNLE - _mm_cmple_ps (less than or equal) with unordered */
        __m128 le_mask = _mm_cmple_ps(va, vb);
        
        /* UNLT - _mm_cmplt_ps (less than) with unordered */
        __m128 lt_mask = _mm_cmplt_ps(va, vb);
        
        /* LTGT - _mm_cmpneq_ps (not equal) */
        __m128 neq_mask = _mm_cmpneq_ps(va, vb);
        
        /* Combine masks into sum */
        sum = _mm_add_ps(sum, unord_mask);
        sum = _mm_add_ps(sum, ord_mask);
        sum = _mm_add_ps(sum, nlt_mask);
        sum = _mm_add_ps(sum, nle_mask);
        sum = _mm_add_ps(sum, le_mask);
        sum = _mm_add_ps(sum, lt_mask);
        sum = _mm_add_ps(sum, neq_mask);
    }
    
    /* Extract result */
    float temp[4];
    _mm_storeu_ps(temp, sum);
    return (int)(temp[0] + temp[1] + temp[2] + temp[3]);
}

/* Function 3: Inline assembly with condition code constraints */
NOINLINE int test_inline_asm(float a, float b) {
    int result = 0;
    int r;
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(r) : : "cc");
    result += r;
    
    /* ORDERED - "no" flag (not overflow, but we need ordered) */
    /* Using multiple conditions to trigger different codes */
    asm volatile ("setnp %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNEQ - "e" flag (equal) with unordered context */
    asm volatile ("sete %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNGE - "ge" flag (greater or equal) */
    asm volatile ("setge %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNGT - "g" flag (greater) */
    asm volatile ("setg %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNLE - "le" flag (less or equal) */
    asm volatile ("setle %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNLT - "l" flag (less) */
    asm volatile ("setl %0" : "=r"(r) : : "cc");
    result += r;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(r) : : "cc");
    result += r;
    
    return result;
}

/* Function 4: Mixed comparisons with fast-math optimizations */
NOINLINE int test_mixed_comparisons(float *arr, int n) {
    int count = 0;
    
    for (int i = 0; i < n - 1; i++) {
        float x = arr[i];
        float y = arr[i + 1];
        
        /* These should generate various condition codes with -ffast-math */
        if (x > y) count++;          /* May generate UNGT */
        if (x >= y) count++;         /* May generate UNGE */
        if (x < y) count++;          /* May generate UNLT */
        if (x <= y) count++;         /* May generate UNLE */
        if (x == y) count++;         /* May generate UNEQ */
        if (x != y) count++;         /* May generate LTGT */
        
        /* Explicit unordered checks */
        if (isunordered(x, y)) count++;  /* UNORDERED */
        if (!isunordered(x, y)) count++; /* ORDERED */
        
        /* fpclassify usage */
        if (fpclassify(x) == FP_NAN) count++;
        if (fpclassify(y) == FP_INFINITE) count++;
    }
    
    return count;
}

/* Main test driver */
int main() {
    const int N = 256;
    float arr1[N], arr2[N];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < N; i++) {
        arr1[i] = (i % 10 == 0) ? NAN : (float)(i * 0.1);
        arr2[i] = (i % 7 == 0) ? NAN : (float)(i * 0.2);
    }
    
    /* Test 1: Scalar builtins */
    int r1 = test_scalar_builtins(arr1[0], arr2[0]);
    r1 += test_scalar_builtins(1.0f, 2.0f);
    r1 += test_scalar_builtins(NAN, 3.0f);
    
    /* Test 2: Vector intrinsics */
    int r2 = test_vector_intrinsics(arr1, arr2, N);
    
    /* Test 3: Inline assembly */
    int r3 = test_inline_asm(arr1[1], arr2[1]);
    
    /* Test 4: Mixed comparisons */
    int r4 = test_mixed_comparisons(arr1, N);
    
    /* Combine results to prevent optimization */
    global_result = r1 + r2 + r3 + r4;
    
    /* Print result to ensure execution */
    printf("Result: %d\n", global_result);
    
    return 0;
}
