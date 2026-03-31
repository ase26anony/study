/* Test program to cover condition code output in i386.cc */
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <stdint.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b, double c, double d) {
    int result = 0;
    
    /* UNORDERED case - __builtin_isunordered */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case - __builtin_isordered */
    if (__builtin_isordered(c, d)) result |= 2;
    
    /* UNEQ case - unordered or equal */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) result |= 4;
    
    /* UNGE case - not less than (nlt) */
    if (__builtin_isgreaterequal(a, b)) result |= 8;
    
    /* UNGT case - not less than or equal (nle) */
    if (__builtin_isgreater(a, b)) result |= 16;
    
    /* UNLE case - unordered or less than or equal (ule) */
    if (__builtin_islessequal(a, b)) result |= 32;
    
    /* UNLT case - unordered or less than (ult) */
    if (__builtin_isless(a, b)) result |= 64;
    
    /* LTGT case - less than or greater than (une) */
    if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *fa, float *fb, int n) {
    __m128 a, b, cmp;
    int result = 0;
    
    for (int i = 0; i < n; i += 4) {
        a = _mm_loadu_ps(&fa[i]);
        b = _mm_loadu_ps(&fb[i]);
        
        /* UNORDERED - _mm_cmpunord_ps */
        cmp = _mm_cmpunord_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* ORDERED - _mm_cmpord_ps */
        cmp = _mm_cmpord_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNGE - _mm_cmpnlt_ps (nlt) */
        cmp = _mm_cmpnlt_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNGT - _mm_cmpnle_ps (nle) */
        cmp = _mm_cmpnle_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNLE - _mm_cmple_ps (ule) - Note: SSE doesn't have direct unordered le */
        cmp = _mm_cmple_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNLT - _mm_cmplt_ps (ult) */
        cmp = _mm_cmplt_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* LTGT - _mm_cmpneq_ps (une) */
        cmp = _mm_cmpneq_ps(a, b);
        result += _mm_movemask_ps(cmp);
    }
    
    return result;
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int out;
    
    /* Inline assembly with condition code constraints */
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(out) : :);
    result += out;
    
    /* ORDERED - "no" flag */
    asm volatile ("setno %0" : "=r"(out) : :);
    result += out;
    
    /* UNEQ - "e" flag with unordered semantics */
    asm volatile ("sete %0" : "=r"(out) : :);
    result += out;
    
    /* UNGE - "ge" flag (nlt) */
    asm volatile ("setge %0" : "=r"(out) : :);
    result += out;
    
    /* UNGT - "g" flag (nle) */
    asm volatile ("setg %0" : "=r"(out) : :);
    result += out;
    
    /* UNLE - "le" flag (ule) */
    asm volatile ("setle %0" : "=r"(out) : :);
    result += out;
    
    /* UNLT - "l" flag (ult) */
    asm volatile ("setl %0" : "=r"(out) : :);
    result += out;
    
    /* LTGT - "ne" flag (une) */
    asm volatile ("setne %0" : "=r"(out) : :);
    result += out;
    
    return result;
}

__attribute__((noinline))
int test_mixed_conditions(float *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n - 1; i++) {
        /* Mix of conditions that should generate different codes */
        if (isnan(arr[i])) sum++;
        if (fpclassify(arr[i]) == FP_NAN) sum++;
        if (isgreater(arr[i], arr[i+1])) sum++;
        if (isless(arr[i], arr[i+1])) sum++;
        if (islessgreater(arr[i], arr[i+1])) sum++;
        if (isunordered(arr[i], arr[i+1])) sum++;
        if (isgreaterequal(arr[i], arr[i+1])) sum++;
        if (islessequal(arr[i], arr[i+1])) sum++;
    }
    
    return sum;
}

int main() {
    /* Create arrays with NaN values to ensure unordered comparisons */
    float fa[32], fb[32];
    double da[32], db[32];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < 32; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 5 == 0) ? NAN : (float)(i * 0.7);
        da[i] = (i % 4 == 0) ? NAN : (double)i;
        db[i] = (i % 6 == 0) ? NAN : (double)(i * 1.3);
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, 32);
    
    /* Test inline assembly */
    checksum += test_inline_asm(fa[1], fb[1]);
    
    /* Test mixed conditions */
    checksum += test_mixed_conditions(fa, 32);
    
    /* Use the result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
