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
    
    /* These should generate various condition codes */
    if (__builtin_isunordered(a, b)) result |= 1;      /* UNORDERED */
    if (!__builtin_isunordered(a, b)) result |= 2;     /* ORDERED */
    if (__builtin_isgreater(a, b)) result |= 4;        /* UNGT or GT */
    if (__builtin_isless(a, b)) result |= 8;           /* UNLT or LT */
    if (__builtin_isgreaterequal(a, b)) result |= 16;  /* UNGE or GE */
    if (__builtin_islessequal(a, b)) result |= 32;     /* UNLE or LE */
    
    /* Double precision variants */
    if (__builtin_isunordered(c, d)) result |= 64;
    if (__builtin_isgreater(c, d)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_vector_intrinsics(const float* a, const float* b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* Use various unordered comparisons */
        __m128 cmp1 = _mm_cmpunord_ps(va, vb);    /* UNORDERED */
        __m128 cmp2 = _mm_cmpord_ps(va, vb);      /* ORDERED */
        __m128 cmp3 = _mm_cmpnlt_ps(va, vb);      /* UNGE (nlt) */
        __m128 cmp4 = _mm_cmpnle_ps(va, vb);      /* UNGT (nle) */
        __m128 cmp5 = _mm_cmpneq_ps(va, vb);      /* UNEQ or NEQ */
        
        /* Mix them together to prevent optimization */
        __m128 t1 = _mm_and_ps(va, cmp1);
        __m128 t2 = _mm_and_ps(vb, cmp2);
        __m128 t3 = _mm_or_ps(t1, t2);
        __m128 t4 = _mm_add_ps(cmp3, cmp4);
        __m128 t5 = _mm_add_ps(t3, t4);
        
        sum = _mm_add_ps(sum, t5);
    }
    
    /* Extract result */
    float res[4];
    _mm_storeu_ps(res, sum);
    return (int)(res[0] + res[1] + res[2] + res[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int r1, r2, r3, r4, r5, r6, r7, r8;
    
    /* Inline assembly with various condition codes */
    asm volatile (
        /* UNORDERED (u) */
        "ucomiss %1, %2\n\t"
        "setu %0"
        : "=r"(r1) : "x"(a), "x"(b) : "cc"
    );
    
    /* ORDERED (no) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setno %0"
        : "=r"(r2) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNEQ (e) for equal or unordered */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(r3) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNGE (ae) - above or equal (not less than) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r"(r4) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNGT (a) - above (not less than or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(r5) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNLE (be) - below or equal */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(r6) : "x"(a), "x"(b) : "cc"
    );
    
    /* UNLT (b) - below */
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

__attribute__((noinline))
int test_mixed_comparisons(float* arr, int n) {
    int count = 0;
    
    for (int i = 0; i < n - 1; i++) {
        float x = arr[i];
        float y = arr[i + 1];
        
        /* Mix of comparisons that should generate different condition codes */
        if (isunordered(x, y)) count++;           /* UNORDERED */
        if (!isunordered(x, y)) count++;          /* ORDERED */
        if (isgreater(x, y)) count++;             /* UNGT */
        if (isless(x, y)) count++;                /* UNLT */
        if (isgreaterequal(x, y)) count++;        /* UNGE */
        if (islessequal(x, y)) count++;           /* UNLE */
        
        /* Explicit check for LTGT (ordered and not equal) */
        if (!isunordered(x, y) && x != y) count++; /* LTGT */
        
        /* Check for UNEQ (unordered or equal) */
        if (isunordered(x, y) || x == y) count++;  /* UNEQ */
    }
    
    return count;
}

int main() {
    const int N = 256;
    float arr1[N], arr2[N];
    double darr1[N], darr2[N];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < N; i++) {
        arr1[i] = (i % 10 == 0) ? NAN : (float)(i * 0.1);
        arr2[i] = (i % 7 == 0) ? NAN : (float)(i * 0.2 + 0.1);
        darr1[i] = (i % 11 == 0) ? NAN : (double)(i * 0.3);
        darr2[i] = (i % 13 == 0) ? NAN : (double)(i * 0.4 + 0.2);
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(arr1[0], arr2[0], darr1[0], darr2[0]);
    checksum += test_scalar_builtins(arr1[1], arr2[1], darr1[1], darr2[1]);
    
    /* Test vector intrinsics */
    checksum += test_vector_intrinsics(arr1, arr2, N);
    
    /* Test inline assembly */
    checksum += test_inline_asm(arr1[2], arr2[2]);
    checksum += test_inline_asm(arr1[3], arr2[3]);
    
    /* Test mixed comparisons */
    checksum += test_mixed_comparisons(arr1, N);
    checksum += test_mixed_comparisons(arr2, N);
    
    /* Use checksum to prevent optimization */
    printf("Result: %d\n", checksum);
    
    return 0;
}
