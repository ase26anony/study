/* test_condition_codes.c - Cover all floating-point condition code cases in i386.cc */
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
    if (!__builtin_isunordered(a, b)) result |= 2;
    
    /* UNEQ case (unordered or equal) - use with fast-math */
    if (a == b) result |= 4;  /* May generate UNEQ with fast-math */
    
    /* UNGE case (not less than) */
    if (__builtin_isgreaterequal(a, b)) result |= 8;
    
    /* UNGT case (not less than or equal) */
    if (__builtin_isgreater(a, b)) result |= 16;
    
    /* UNLE case (unordered or less than or equal) */
    if (__builtin_islessequal(a, b)) result |= 32;
    
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(a, b)) result |= 64;
    
    /* LTGT case (less than or greater than, ordered) */
    if (__builtin_islessgreater(c, d)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *a, float *b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _CMP_UNORD_Q */
        __m128 cmp_unord = _mm_cmpunord_ps(va, vb);
        
        /* ORDERED - _CMP_ORD_Q */
        __m128 cmp_ord = _mm_cmpord_ps(va, vb);
        
        /* UNGE - _CMP_NLT_UQ (not less than, unordered quiet) */
        __m128 cmp_nlt = _mm_cmpnlt_ps(va, vb);
        
        /* UNGT - _CMP_NLE_UQ (not less than or equal, unordered quiet) */
        __m128 cmp_nle = _mm_cmpnle_ps(va, vb);
        
        /* UNLE - _CMP_LE_UQ (less than or equal, unordered quiet) */
        __m128 cmp_ule = _mm_cmple_ps(va, vb);
        
        /* UNLT - _CMP_LT_UQ (less than, unordered quiet) */
        __m128 cmp_ult = _mm_cmplt_ps(va, vb);
        
        /* LTGT - _CMP_NEQ_UQ (not equal, unordered quiet) */
        __m128 cmp_une = _mm_cmpneq_ps(va, vb);
        
        /* Combine results */
        sum = _mm_add_ps(sum, cmp_unord);
        sum = _mm_add_ps(sum, cmp_ord);
        sum = _mm_add_ps(sum, cmp_nlt);
        sum = _mm_add_ps(sum, cmp_nle);
        sum = _mm_add_ps(sum, cmp_ule);
        sum = _mm_add_ps(sum, cmp_ult);
        sum = _mm_add_ps(sum, cmp_une);
    }
    
    /* Extract result */
    float res[4];
    _mm_storeu_ps(res, sum);
    return (int)(res[0] + res[1] + res[2] + res[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int out;
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(out) : : "cc");
    result += out;
    
    /* ORDERED - "no" flag (not overflow) but we need ordered FP */
    /* Use alternative: test for ordered via fucomip */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "setnp %0"
        : "=r"(out) : : "cc"
    );
    result += out;
    
    /* UNGE - "ae" (above or equal) for FP with unordered */
    asm volatile ("setae %0" : "=r"(out) : : "cc");
    result += out;
    
    /* UNGT - "a" (above) for FP with unordered */
    asm volatile ("seta %0" : "=r"(out) : : "cc");
    result += out;
    
    /* UNLE - "be" (below or equal) for FP with unordered */
    asm volatile ("setbe %0" : "=r"(out) : : "cc");
    result += out;
    
    /* UNLT - "b" (below) for FP with unordered */
    asm volatile ("setb %0" : "=r"(out) : : "cc");
    result += out;
    
    /* LTGT - "ne" (not equal) for ordered comparison */
    asm volatile ("setne %0" : "=r"(out) : : "cc");
    result += out;
    
    return result;
}

__attribute__((noinline))
int test_mixed_conditions(float *arr, int n) {
    int count = 0;
    
    for (int i = 0; i < n - 1; i++) {
        float x = arr[i];
        float y = arr[i + 1];
        
        /* Mix of conditions that may generate different codes */
        if (isnan(x) || isnan(y)) {
            count++;  /* UNORDERED path */
        } else if (x > y) {
            count += 2;  /* UNGT path with fast-math */
        } else if (x < y) {
            count += 3;  /* UNLT path with fast-math */
        } else if (x == y) {
            count += 4;  /* UNEQ path */
        }
        
        /* Use fpclassify to trigger various conditions */
        int cx = fpclassify(x);
        int cy = fpclassify(y);
        
        if (cx == FP_NAN || cy == FP_NAN) {
            count += 5;
        } else if (cx == FP_INFINITE && cy == FP_INFINITE) {
            count += 6;
        }
    }
    
    return count;
}

/* Global volatile to prevent optimization */
volatile float global_trigger = 0.0f;

int main() {
    /* Create arrays with NaN values to ensure unordered comparisons */
    float fa[16], fb[16];
    double da[8], db[8];
    
    /* Initialize with mix of normal and NaN values */
    for (int i = 0; i < 16; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 4 == 0) ? NAN : (float)(i * 0.5);
    }
    
    for (int i = 0; i < 8; i++) {
        da[i] = (i % 2 == 0) ? NAN : (double)i;
        db[i] = (i % 3 == 0) ? NAN : (double)(i * 0.7);
    }
    
    int checksum = 0;
    
    /* Test 1: Scalar builtins */
    checksum += test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    checksum += test_scalar_builtins(fa[1], fb[1], da[1], db[1]);
    
    /* Test 2: SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, 16);
    
    /* Test 3: Inline assembly */
    checksum += test_inline_asm(fa[2], fb[2]);
    
    /* Test 4: Mixed conditions */
    checksum += test_mixed_conditions(fa, 16);
    
    /* Additional triggers with different values */
    for (int i = 0; i < 8; i++) {
        float x = (float)i;
        float y = (float)(i + 1);
        
        /* Force generation of various condition codes */
        if (__builtin_isgreater(x, y)) checksum++;
        if (__builtin_isless(x, y)) checksum++;
        if (__builtin_isunordered(x * 0.0f, y * 0.0f)) checksum++;
        if (__builtin_islessequal(x, y)) checksum++;
        if (__builtin_isgreaterequal(x, y)) checksum++;
    }
    
    /* Use global to prevent dead code elimination */
    global_trigger = (float)checksum;
    
    printf("Result: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
