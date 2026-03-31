/* Test program to cover floating-point condition code output in i386.cc */
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
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case */
    if (__builtin_isordered(c, d)) result |= 2;
    
    /* UNEQ case (unordered or equal) */
    if (!__builtin_islessgreater(a, b)) result |= 4;
    
    /* UNGE case (not less than) */
    if (__builtin_isgreaterequal(a, b)) result |= 8;
    
    /* UNGT case (not less than or equal) */
    if (__builtin_isgreater(c, d)) result |= 16;
    
    /* UNLE case (unordered or less than or equal) */
    if (__builtin_islessequal(a, b)) result |= 32;
    
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(c, d)) result |= 64;
    
    /* LTGT case (less than or greater than, but not unordered) */
    if (__builtin_islessgreater(c, d)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *fa, float *fb, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 a = _mm_loadu_ps(&fa[i]);
        __m128 b = _mm_loadu_ps(&fb[i]);
        
        /* UNORDERED: _CMP_UNORD_Q */
        __m128 cmp_unord = _mm_cmpunord_ps(a, b);
        
        /* ORDERED: _CMP_ORD_Q */
        __m128 cmp_ord = _mm_cmpord_ps(a, b);
        
        /* UNGE: _CMP_NLT_UQ (not less than) */
        __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);
        
        /* UNGT: _CMP_NLE_UQ (not less than or equal) */
        __m128 cmp_nle = _mm_cmpnle_ps(a, b);
        
        /* UNLE: _CMP_LE_UQ (less than or equal, unordered) */
        __m128 cmp_ule = _mm_cmple_ps(a, b);
        
        /* UNLT: _CMP_LT_UQ (less than, unordered) */
        __m128 cmp_ult = _mm_cmplt_ps(a, b);
        
        /* LTGT: _CMP_NEQ_UQ (not equal, unordered) */
        __m128 cmp_une = _mm_cmpneq_ps(a, b);
        
        /* UNEQ: _CMP_EQ_UQ (equal, unordered) */
        __m128 cmp_ueq = _mm_cmpeq_ps(a, b);
        
        /* Combine results */
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_unord, a));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_ord, b));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_nlt, a));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_nle, b));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_ule, a));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_ult, b));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_une, a));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_ueq, b));
    }
    
    /* Extract result */
    float r[4];
    _mm_storeu_ps(r, sum);
    return (int)(r[0] + r[1] + r[2] + r[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int temp;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED: "u" flag */
    asm volatile ("setu %0" : "=r"(temp) : :);
    result += temp;
    
    /* ORDERED: "no" flag (not overflow) - used for ordered */
    asm volatile ("setno %0" : "=r"(temp) : :);
    result += temp;
    
    /* UNEQ: "e" flag (equal) with unordered semantics */
    asm volatile ("sete %0" : "=r"(temp) : :);
    result += temp;
    
    /* UNGE: "ge" flag (greater or equal) */
    asm volatile ("setge %0" : "=r"(temp) : :);
    result += temp;
    
    /* UNGT: "g" flag (greater) */
    asm volatile ("setg %0" : "=r"(temp) : :);
    result += temp;
    
    /* UNLE: "le" flag (less or equal) */
    asm volatile ("setle %0" : "=r"(temp) : :);
    result += temp;
    
    /* UNLT: "l" flag (less) */
    asm volatile ("setl %0" : "=r"(temp) : :);
    result += temp;
    
    /* LTGT: "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(temp) : :);
    result += temp;
    
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(double *da, double *db, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        double a = da[i];
        double b = db[i];
        
        /* Generate various condition codes through different comparisons */
        if (a != b) {  /* LTGT/UNE */
            result++;
        }
        
        if (a >= b) {  /* UNGE */
            result += 2;
        }
        
        if (a <= b) {  /* UNLE */
            result += 3;
        }
        
        if (isnan(a) || isnan(b)) {  /* UNORDERED */
            result += 4;
        }
        
        if (!isnan(a) && !isnan(b)) {  /* ORDERED */
            result += 5;
        }
        
        if (a == b) {  /* UNEQ */
            result += 6;
        }
        
        if (a > b) {  /* UNGT */
            result += 7;
        }
        
        if (a < b) {  /* UNLT */
            result += 8;
        }
    }
    
    return result;
}

int main() {
    /* Initialize with NaN and normal values to trigger unordered comparisons */
    float fa[16], fb[16];
    double da[16], db[16];
    
    for (int i = 0; i < 16; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 4 == 0) ? NAN : (float)(i * 2);
        da[i] = (i % 5 == 0) ? NAN : (double)i;
        db[i] = (i % 6 == 0) ? NAN : (double)(i * 3);
    }
    
    /* Call all test functions to generate various condition codes */
    int checksum = 0;
    
    checksum += test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    checksum += test_sse_intrinsics(fa, fb, 16);
    checksum += test_inline_asm(fa[1], fb[1]);
    checksum += test_mixed_comparisons(da, db, 16);
    
    /* Use checksum to prevent optimization */
    printf("Result: %d\n", checksum);
    
    return 0;
}
