/* test_condition_codes.c - Cover GCC i386.cc condition code output */
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
    
    /* UNEQ case - unordered or equal */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) result |= 4;
    
    /* UNGE case - not less than (greater or equal or unordered) */
    if (!__builtin_isless(a, b)) result |= 8;
    
    /* UNGT case - not less or equal (greater or unordered) */
    if (!__builtin_islessequal(a, b)) result |= 16;
    
    /* UNLE case - less or equal or unordered */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) result |= 32;
    
    /* UNLT case - less than or unordered */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) result |= 64;
    
    /* LTGT case - less or greater (ordered and not equal) */
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
        
        /* UNORDERED - _CMP_UNORD_Q */
        cmp = _mm_cmpunord_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* ORDERED - _CMP_ORD_Q */
        cmp = _mm_cmpord_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNGE - _CMP_NLT_UQ (not less than) */
        cmp = _mm_cmpnlt_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNGT - _CMP_NLE_UQ (not less or equal) */
        cmp = _mm_cmpnle_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNLE - _CMP_LE_OQ (less or equal) with unordered inputs */
        cmp = _mm_cmple_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNLT - _CMP_LT_OQ (less than) with unordered inputs */
        cmp = _mm_cmplt_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* LTGT - _CMP_NEQ_OQ (not equal) */
        cmp = _mm_cmpneq_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNEQ - _CMP_EQ_UQ (equal) with unordered inputs */
        cmp = _mm_cmpeq_ps(a, b);
        result += _mm_movemask_ps(cmp);
    }
    
    return result;
}

__attribute__((noinline))
int test_inline_asm(float x, float y) {
    int result = 0;
    int8_t out;
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(out) : : "cc");
    result += out;
    
    /* ORDERED - "no" flag (not overflow) - used with floating point */
    asm volatile ("setno %0" : "=r"(out) : : "cc");
    result += out;
    
    /* UNEQ - "e" flag (equal) with unordered inputs */
    asm volatile ("sete %0" : "=r"(out) : : "cc");
    result += out;
    
    /* UNGE - "ae" flag (above or equal) */
    asm volatile ("setae %0" : "=r"(out) : : "cc");
    result += out;
    
    /* UNGT - "a" flag (above) */
    asm volatile ("seta %0" : "=r"(out) : : "cc");
    result += out;
    
    /* UNLE - "be" flag (below or equal) */
    asm volatile ("setbe %0" : "=r"(out) : : "cc");
    result += out;
    
    /* UNLT - "b" flag (below) */
    asm volatile ("setb %0" : "=r"(out) : : "cc");
    result += out;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(out) : : "cc");
    result += out;
    
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(float *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n - 1; i++) {
        float a = arr[i];
        float b = arr[i + 1];
        
        /* Mix of comparisons that should generate different condition codes */
        sum += (__builtin_isunordered(a, b) ? 1 : 0);
        sum += (__builtin_isgreater(a, b) ? 2 : 0);
        sum += (__builtin_isless(a, b) ? 3 : 0);
        sum += (__builtin_islessequal(a, b) ? 4 : 0);
        sum += (__builtin_isgreaterequal(a, b) ? 5 : 0);
        
        /* Use fpclassify to trigger condition codes */
        int ca = fpclassify(a);
        int cb = fpclassify(b);
        sum += (ca == FP_NAN || cb == FP_NAN) ? 6 : 0;
        sum += (ca == FP_INFINITE || cb == FP_INFINITE) ? 7 : 0;
    }
    
    return sum;
}

int main(void) {
    /* Create arrays with mix of normal values and NaN */
    float fa[32], fb[32];
    double da[16], db[16];
    
    /* Initialize with pattern including NaN */
    for (int i = 0; i < 32; i++) {
        fa[i] = (i % 5 == 0) ? NAN : (float)(i * 1.1);
        fb[i] = (i % 7 == 0) ? NAN : (float)(i * 0.9);
    }
    
    for (int i = 0; i < 16; i++) {
        da[i] = (i % 3 == 0) ? NAN : (double)(i * 1.5);
        db[i] = (i % 4 == 0) ? NAN : (double)(i * 0.5);
    }
    
    int checksum = 0;
    
    /* Test scalar builtins with various inputs */
    checksum += test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    checksum += test_scalar_builtins(1.0f, NAN, 2.0, NAN);
    checksum += test_scalar_builtins(NAN, 3.0f, NAN, 4.0);
    checksum += test_scalar_builtins(5.0f, 5.0f, 6.0, 6.0);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, 32);
    
    /* Test inline assembly */
    checksum += test_inline_asm(fa[1], fb[1]);
    checksum += test_inline_asm(NAN, fb[2]);
    
    /* Test mixed comparisons */
    checksum += test_mixed_comparisons(fa, 32);
    
    /* Use results to prevent optimization */
    volatile int final_result = checksum;
    printf("Result: %d\n", final_result);
    
    return 0;
}
