/* Test program to cover condition code output in i386.cc */
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <stdint.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b) {
    int result = 0;
    
    /* UNORDERED */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED */
    if (__builtin_isordered(a, b)) result |= 2;
    
    /* UNEQ (unordered or equal) - not directly available, but can be synthesized */
    if (!__builtin_isless(a, b) && !__builtin_isgreater(a, b)) result |= 4;
    
    /* UNGE (not less than) */
    if (__builtin_isgreaterequal(a, b)) result |= 8;
    
    /* UNGT (greater and ordered) */
    if (__builtin_isgreater(a, b)) result |= 16;
    
    /* UNLE (less or equal and ordered) */
    if (__builtin_islessequal(a, b)) result |= 32;
    
    /* UNLT (less and ordered) */
    if (__builtin_isless(a, b)) result |= 64;
    
    /* LTGT (less or greater, but not equal and ordered) */
    if (a != b && !__builtin_isunordered(a, b)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(const float* arr1, const float* arr2, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i += 4) {
        __m128 a = _mm_loadu_ps(&arr1[i]);
        __m128 b = _mm_loadu_ps(&arr2[i]);
        
        /* UNORDERED */
        __m128 cmp_unord = _mm_cmpunord_ps(a, b);
        sum += _mm_movemask_ps(cmp_unord);
        
        /* ORDERED */
        __m128 cmp_ord = _mm_cmpord_ps(a, b);
        sum += _mm_movemask_ps(cmp_ord);
        
        /* UNGE (not less than) */
        __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);
        sum += _mm_movemask_ps(cmp_nlt);
        
        /* UNGT (not less or equal) */
        __m128 cmp_nle = _mm_cmpnle_ps(a, b);
        sum += _mm_movemask_ps(cmp_nle);
        
        /* UNLE (less or equal) */
        __m128 cmp_le = _mm_cmple_ps(a, b);
        sum += _mm_movemask_ps(cmp_le);
        
        /* UNLT (less than) */
        __m128 cmp_lt = _mm_cmplt_ps(a, b);
        sum += _mm_movemask_ps(cmp_lt);
        
        /* NEQ (not equal) - maps to LTGT for ordered comparison */
        __m128 cmp_neq = _mm_cmpneq_ps(a, b);
        sum += _mm_movemask_ps(cmp_neq);
    }
    
    return sum;
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result_u, result_o, result_nlt, result_nle, result_ule, result_ult, result_une;
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(result_u) : : "cc");
    
    /* ORDERED - "no" flag (not unordered) */
    asm volatile ("setno %0" : "=r"(result_o) : : "cc");
    
    /* UNGE - "nb" (not below) / "ae" (above or equal) */
    asm volatile ("setae %0" : "=r"(result_nlt) : : "cc");
    
    /* UNGT - "nbe" (not below or equal) / "a" (above) */
    asm volatile ("seta %0" : "=r"(result_nle) : : "cc");
    
    /* UNLE - "na" (not above) / "be" (below or equal) */
    asm volatile ("setbe %0" : "=r"(result_ule) : : "cc");
    
    /* UNLT - "nae" (not above or equal) / "b" (below) */
    asm volatile ("setb %0" : "=r"(result_ult) : : "cc");
    
    /* LTGT - "ne" (not equal) */
    asm volatile ("setne %0" : "=r"(result_une) : : "cc");
    
    /* Force evaluation of a and b to affect condition codes */
    asm volatile ("" : : "X"(a), "X"(b));
    
    return result_u + result_o + result_nlt + result_nle + result_ule + result_ult + result_une;
}

__attribute__((noinline))
int test_mixed_conditions(double x, double y) {
    int r = 0;
    
    /* Generate UNEQ through fpclassify */
    if (fpclassify(x) == FP_NAN || fpclassify(y) == FP_NAN) {
        r |= 1;
    }
    
    /* Generate various conditions through explicit checks */
    if (x >= y) r |= 2;      /* UNGE */
    if (x > y)  r |= 4;      /* UNGT */
    if (x <= y) r |= 8;      /* UNLE */
    if (x < y)  r |= 16;     /* UNLT */
    
    /* LTGT through inequality check */
    if (x != y) r |= 32;
    
    return r;
}

/* Force generation of specific RTL patterns */
__attribute__((noinline))
float test_rtl_patterns(float* f1, float* f2, int n) {
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* These comparisons should generate various condition codes */
        if (f1[i] != f2[i] && !__builtin_isunordered(f1[i], f2[i])) {
            sum += 1.0f;  /* LTGT */
        }
        if (__builtin_isless(f1[i], f2[i])) {
            sum += 2.0f;  /* UNLT */
        }
        if (__builtin_islessequal(f1[i], f2[i])) {
            sum += 3.0f;  /* UNLE */
        }
        if (__builtin_isgreater(f1[i], f2[i])) {
            sum += 4.0f;  /* UNGT */
        }
        if (__builtin_isgreaterequal(f1[i], f2[i])) {
            sum += 5.0f;  /* UNGE */
        }
    }
    
    return sum;
}

int main() {
    /* Create arrays with NaN values to ensure unordered comparisons */
    float arr1[16], arr2[16];
    double darr1[8], darr2[8];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < 16; i++) {
        arr1[i] = (i % 3 == 0) ? NAN : (float)i;
        arr2[i] = (i % 5 == 0) ? NAN : (float)(i * 2);
    }
    
    for (int i = 0; i < 8; i++) {
        darr1[i] = (i % 2 == 0) ? NAN : (double)i;
        darr2[i] = (i % 3 == 0) ? NAN : (double)(i * 3);
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(arr1[0], arr2[0]);
    checksum += test_scalar_builtins(1.0f, 2.0f);
    checksum += test_scalar_builtins(NAN, 3.0f);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(arr1, arr2, 16);
    
    /* Test inline assembly */
    checksum += test_inline_asm(arr1[1], arr2[1]);
    checksum += test_inline_asm(NAN, arr2[2]);
    
    /* Test mixed conditions */
    checksum += test_mixed_conditions(darr1[0], darr2[0]);
    checksum += test_mixed_conditions(1.0, 2.0);
    checksum += test_mixed_conditions(NAN, 3.0);
    
    /* Test RTL patterns */
    float fsum = test_rtl_patterns(arr1, arr2, 16);
    checksum += (int)fsum;
    
    /* Additional tests to ensure all condition codes are used */
    volatile float v1 = 1.0f, v2 = 2.0f, v3 = NAN;
    
    /* Force generation of UNEQ through complex expression */
    if (!(v1 < v2) && !(v1 > v2)) {
        checksum += 100;
    }
    
    /* Force generation of ORDERED check */
    if (!__builtin_isunordered(v1, v2)) {
        checksum += 200;
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
