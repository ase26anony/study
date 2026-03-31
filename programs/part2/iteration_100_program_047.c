/* Test program to cover floating-point condition code output in i386.cc */
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

/* Function using scalar builtins */
NOINLINE int test_scalar_builtins(float a, float b, double c, double d) {
    int result = 0;
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* ORDERED case */
    if (!__builtin_isunordered(c, d)) {
        result |= 2;
    }
    
    /* UNEQ (unordered or equal) - using isnan check */
    if (isnan(a) || isnan(b) || a == b) {
        result |= 4;
    }
    
    /* UNGE (not less than) */
    if (__builtin_isgreaterequal(a, b)) {
        result |= 8;
    }
    
    /* UNGT (greater than, unordered allowed) */
    if (__builtin_isgreater(c, d)) {
        result |= 16;
    }
    
    /* UNLE (less than or equal, unordered allowed) */
    if (__builtin_islessequal(a, b)) {
        result |= 32;
    }
    
    /* UNLT (less than, unordered allowed) */
    if (__builtin_isless(c, d)) {
        result |= 64;
    }
    
    /* LTGT (less than or greater than, but not equal and not unordered) */
    if (a != b && !isnan(a) && !isnan(b)) {
        result |= 128;
    }
    
    return result;
}

/* Function using SSE intrinsics */
NOINLINE int test_sse_intrinsics(float *fa, float *fb, int n) {
    __m128 a, b, cmp;
    int result = 0;
    
    for (int i = 0; i < n; i += 4) {
        a = _mm_loadu_ps(&fa[i]);
        b = _mm_loadu_ps(&fb[i]);
        
        /* UNORDERED */
        cmp = _mm_cmpunord_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* ORDERED */
        cmp = _mm_cmpord_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNGE (not less than) */
        cmp = _mm_cmpnlt_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNGT (not less than or equal) */
        cmp = _mm_cmpnle_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNLE (unordered or less than or equal) */
        cmp = _mm_cmple_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNLT (unordered or less than) */
        cmp = _mm_cmplt_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* LTGT (unordered or not equal) - using _mm_cmpneq_ps */
        cmp = _mm_cmpneq_ps(a, b);
        result += _mm_movemask_ps(cmp);
    }
    
    return result;
}

/* Function using inline assembly with condition codes */
NOINLINE int test_inline_asm(float a, float b) {
    int result = 0;
    int r;
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(r) : : "cc");
    result += r;
    
    /* ORDERED - "no" flag (not overflow) but we need ordered FP */
    /* Using multiple asm statements to cover different cases */
    
    /* UNGE - "ae" or "nb" (not below/not less) */
    asm volatile ("setae %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNGT - "a" or "nbe" (not below or equal/not less or equal) */
    asm volatile ("seta %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNLE - "be" (below or equal/less or equal) */
    asm volatile ("setbe %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNLT - "b" or "nae" (below/not above or equal) */
    asm volatile ("setb %0" : "=r"(r) : : "cc");
    result += r;
    
    /* LTGT - "ne" (not equal) */
    asm volatile ("setne %0" : "=r"(r) : : "cc");
    result += r;
    
    return result;
}

/* Function using double precision and mixed operations */
NOINLINE int test_double_mixed(double *da, double *db, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        double a = da[i];
        double b = db[i];
        
        /* Generate various condition codes through complex expressions */
        if ((a > b) && !isnan(a) && !isnan(b)) {
            result++;  /* UNGT or GT */
        }
        
        if ((a < b) && !isnan(a) && !isnan(b)) {
            result++;  /* UNLT or LT */
        }
        
        if (isnan(a) || isnan(b)) {
            result++;  /* UNORDERED */
        }
        
        if (!isnan(a) && !isnan(b) && a != b) {
            result++;  /* LTGT */
        }
        
        if (a >= b) {
            result++;  /* UNGE */
        }
        
        if (a <= b) {
            result++;  /* UNLE */
        }
    }
    
    return result;
}

/* Main test driver */
int main() {
    const int N = 256;
    float fa[N], fb[N];
    double da[N], db[N];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < N; i++) {
        fa[i] = (i % 10 == 0) ? NAN : (float)(i * 0.1);
        fb[i] = (i % 7 == 0) ? NAN : (float)(i * 0.2 + 0.5);
        da[i] = (i % 11 == 0) ? NAN : (double)(i * 0.3);
        db[i] = (i % 13 == 0) ? NAN : (double)(i * 0.4 + 1.0);
    }
    
    /* Test scalar builtins */
    int r1 = test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    global_result += r1;
    
    /* Test SSE intrinsics */
    int r2 = test_sse_intrinsics(fa, fb, N);
    global_result += r2;
    
    /* Test inline assembly */
    int r3 = test_inline_asm(fa[1], fb[1]);
    global_result += r3;
    
    /* Test double precision mixed */
    int r4 = test_double_mixed(da, db, N);
    global_result += r4;
    
    /* Additional tests with specific values to trigger edge cases */
    float special_a[] = {NAN, INFINITY, -INFINITY, 0.0f, -0.0f, 1.0f, -1.0f};
    float special_b[] = {NAN, 0.0f, INFINITY, -INFINITY, 1.0f, -1.0f, 0.0f};
    
    for (int i = 0; i < 7; i++) {
        r1 = test_scalar_builtins(special_a[i], special_b[i], 
                                  (double)special_a[i], (double)special_b[i]);
        global_result += r1;
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", global_result);
    
    return 0;
}
