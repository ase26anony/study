/* Test program to cover floating-point condition code output in i386.cc */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* SSE/AVX intrinsics */
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to ensure code generation */
#define NOINLINE __attribute__((noinline))

/* Global volatile to prevent optimization */
volatile int global_result = 0;

/* Function using scalar builtins */
NOINLINE int test_scalar_builtins(float a, float b, double c, double d) {
    int result = 0;
    
    /* These should generate various condition codes */
    if (__builtin_isunordered(a, b)) result |= 1;      /* UNORDERED */
    if (!__builtin_isunordered(c, d)) result |= 2;     /* ORDERED */
    if (__builtin_isgreater(a, b)) result |= 4;        /* UNGT or GT */
    if (__builtin_isless(a, b)) result |= 8;          /* UNLT or LT */
    if (__builtin_isgreaterequal(a, b)) result |= 16;  /* UNGE or GE */
    if (__builtin_islessequal(a, b)) result |= 32;     /* UNLE or LE */
    
    /* Use fpclassify/isnan to trigger condition codes */
    if (isnan(a)) result |= 64;
    if (fpclassify(b) == FP_NAN) result |= 128;
    
    return result;
}

/* Function using SSE intrinsics */
NOINLINE int test_sse_intrinsics(const float* a, const float* b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* Use various comparison intrinsics */
        __m128 cmp_unord = _mm_cmpunord_ps(va, vb);    /* UNORDERED */
        __m128 cmp_ord = _mm_cmpord_ps(va, vb);        /* ORDERED */
        __m128 cmp_nlt = _mm_cmpnlt_ps(va, vb);        /* UNGE (nlt) */
        __m128 cmp_nle = _mm_cmpnle_ps(va, vb);        /* UNGT (nle) */
        __m128 cmp_ule = _mm_cmpule_ps(va, vb);        /* UNLE (ule) */
        __m128 cmp_ult = _mm_cmplt_ps(va, vb);         /* UNLT (ult) */
        __m128 cmp_neq = _mm_cmpneq_ps(va, vb);        /* LTGT (une) */
        __m128 cmp_ueq = _mm_cmpueq_ps(va, vb);        /* UNEQ (ueq) */
        
        /* Mix results to prevent optimization */
        sum = _mm_add_ps(sum, cmp_unord);
        sum = _mm_add_ps(sum, cmp_ord);
        sum = _mm_add_ps(sum, cmp_nlt);
        sum = _mm_add_ps(sum, cmp_nle);
        sum = _mm_add_ps(sum, cmp_ule);
        sum = _mm_add_ps(sum, cmp_ult);
        sum = _mm_add_ps(sum, cmp_neq);
        sum = _mm_add_ps(sum, cmp_ueq);
    }
    
    /* Extract some result */
    float temp[4];
    _mm_storeu_ps(temp, sum);
    return (int)(temp[0] + temp[1] + temp[2] + temp[3]);
}

/* Function using inline assembly with condition codes */
NOINLINE int test_inline_asm(float a, float b) {
    int result = 0;
    int r1, r2, r3, r4, r5, r6, r7, r8;
    
    /* Use various condition codes in inline asm */
    asm volatile (
        /* UNORDERED (u) */
        "ucomiss %1, %2\n\t"
        "setu %0"
        : "=r"(r1) : "x"(a), "x"(b) : "cc"
    );
    
    /* ORDERED (no) - not directly available, use negation */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(r2) : "x"(a), "x"(b) : "al", "cc"
    );
    
    /* UNEQ (e) - equal or unordered */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(r3) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNGE (ae) - not less than */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r"(r4) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNGT (a) - greater than (not less or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(r5) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNLE (be) - less or equal or unordered */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(r6) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNLT (b) - less than or unordered */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(r7) : "x"(a), "x"(b) : "cc"
    );
    
    /* LTGT (ne) - not equal and ordered */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(r8) : "x"(a), "x"(b) : "cc"
    );
    
    result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    return result;
}

/* Function using double precision and mixed operations */
NOINLINE int test_double_mixed(double* arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n - 1; i++) {
        double a = arr[i];
        double b = arr[i + 1];
        
        /* Mix ordered and unordered comparisons */
        int cmp1 = isunordered(a, b) ? 1 : 0;          /* UNORDERED */
        int cmp2 = !isunordered(a, b) ? 2 : 0;         /* ORDERED */
        int cmp3 = (a > b) ? 4 : 0;                    /* GT/UNGT */
        int cmp4 = (a < b) ? 8 : 0;                    /* LT/UNLT */
        int cmp5 = (a >= b) ? 16 : 0;                  /* GE/UNGE */
        int cmp6 = (a <= b) ? 32 : 0;                  /* LE/UNLE */
        int cmp7 = (a != b) ? 64 : 0;                  /* NE/LTGT */
        int cmp8 = (a == b) ? 128 : 0;                 /* EQ/UNEQ */
        
        result += cmp1 + cmp2 + cmp3 + cmp4 + cmp5 + cmp6 + cmp7 + cmp8;
    }
    
    return result;
}

/* Main test driver */
int main() {
    const int N = 256;
    float* fa = (float*)aligned_alloc(16, N * sizeof(float));
    float* fb = (float*)aligned_alloc(16, N * sizeof(float));
    double* da = (double*)malloc(N * sizeof(double));
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < N; i++) {
        fa[i] = (i % 2 == 0) ? (float)i : NAN;
        fb[i] = (i % 3 == 0) ? (float)(i * 2) : NAN;
        da[i] = (i % 5 == 0) ? (double)i : NAN;
    }
    
    /* Run all tests */
    int result = 0;
    
    /* Test scalar builtins */
    result += test_scalar_builtins(fa[0], fb[0], da[0], da[1]);
    result += test_scalar_builtins(fa[1], fb[1], da[1], da[2]);
    
    /* Test SSE intrinsics */
    result += test_sse_intrinsics(fa, fb, N);
    
    /* Test inline assembly */
    result += test_inline_asm(fa[2], fb[2]);
    result += test_inline_asm(fa[3], fb[3]);
    
    /* Test double precision mixed */
    result += test_double_mixed(da, N);
    
    /* Additional tests with special values */
    float special_a[] = {0.0f, -0.0f, INFINITY, -INFINITY, NAN};
    float special_b[] = {NAN, INFINITY, -INFINITY, 0.0f, -0.0f};
    
    for (int i = 0; i < 5; i++) {
        result += test_scalar_builtins(special_a[i], special_b[i], 
                                      (double)special_a[i], (double)special_b[i]);
    }
    
    /* Store to volatile to prevent optimization */
    global_result = result;
    
    /* Print result to ensure execution */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(fa);
    free(fb);
    free(da);
    
    return 0;
}
