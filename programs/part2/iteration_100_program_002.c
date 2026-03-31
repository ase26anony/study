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
    
    /* These should generate various condition codes */
    if (__builtin_isunordered(a, b)) result |= 1;    /* UNORDERED */
    if (!__builtin_isunordered(a, b)) result |= 2;   /* ORDERED */
    if (__builtin_isgreater(a, b)) result |= 4;      /* UNGT or LTGT */
    if (__builtin_isless(a, b)) result |= 8;         /* UNLT or LTGT */
    if (__builtin_isgreaterequal(a, b)) result |= 16; /* UNGE */
    if (__builtin_islessequal(a, b)) result |= 32;   /* UNLE */
    
    /* UNEQ: equal or unordered */
    if (!(a > b) && !(a < b)) result |= 64;          /* UNEQ */
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *a, float *b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* Use various unordered comparisons */
        __m128 cmp1 = _mm_cmpunord_ps(va, vb);      /* UNORDERED */
        __m128 cmp2 = _mm_cmpord_ps(va, vb);        /* ORDERED */
        __m128 cmp3 = _mm_cmpnlt_ps(va, vb);        /* UNGE (nlt) */
        __m128 cmp4 = _mm_cmpnle_ps(va, vb);        /* UNGT (nle) */
        __m128 cmp5 = _mm_cmpneq_ps(va, vb);        /* LTGT (une) */
        
        /* Mix them together to prevent optimization */
        sum = _mm_add_ps(sum, cmp1);
        sum = _mm_add_ps(sum, cmp2);
        sum = _mm_add_ps(sum, cmp3);
        sum = _mm_add_ps(sum, cmp4);
        sum = _mm_add_ps(sum, cmp5);
    }
    
    /* Extract result */
    float res[4];
    _mm_storeu_ps(res, sum);
    return (int)(res[0] + res[1] + res[2] + res[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int temp;
    
    /* Inline asm with various condition codes */
    asm volatile (
        /* UNORDERED (u) */
        "fucomi %%st(1), %%st\n\t"
        "setu %0\n\t"
        : "=r"(temp)
        : 
        : "st", "st(1)"
    );
    result += temp;
    
    asm volatile (
        /* ORDERED (no) */
        "fucomi %%st(1), %%st\n\t"
        "setno %0\n\t"
        : "=r"(temp)
        : 
        : "st", "st(1)"
    );
    result += temp;
    
    asm volatile (
        /* UNEQ (e) for equal or unordered */
        "fucomi %%st(1), %%st\n\t"
        "sete %0\n\t"
        : "=r"(temp)
        : 
        : "st", "st(1)"
    );
    result += temp;
    
    asm volatile (
        /* UNGE (ae) - above or equal (not less than) */
        "fucomi %%st(1), %%st\n\t"
        "setae %0\n\t"
        : "=r"(temp)
        : 
        : "st", "st(1)"
    );
    result += temp;
    
    asm volatile (
        /* UNGT (a) - above (not less than or equal) */
        "fucomi %%st(1), %%st\n\t"
        "seta %0\n\t"
        : "=r"(temp)
        : 
        : "st", "st(1)"
    );
    result += temp;
    
    asm volatile (
        /* UNLE (be) - below or equal */
        "fucomi %%st(1), %%st\n\t"
        "setbe %0\n\t"
        : "=r"(temp)
        : 
        : "st", "st(1)"
    );
    result += temp;
    
    asm volatile (
        /* UNLT (b) - below */
        "fucomi %%st(1), %%st\n\t"
        "setb %0\n\t"
        : "=r"(temp)
        : 
        : "st", "st(1)"
    );
    result += temp;
    
    return result;
}

__attribute__((noinline))
int test_double_operations(double a, double b) {
    int result = 0;
    
    /* Test with doubles to cover different FPU modes */
    if (__builtin_isunordered(a, b)) result |= 1;
    if (__builtin_isgreater(a, b)) result |= 2;
    if (__builtin_isless(a, b)) result |= 4;
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if ((a < b) || (a > b)) result |= 8;
    
    return result;
}

__attribute__((noinline))
int test_mixed_nan(float *arr, int n) {
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        /* Generate UNEQ: equal or unordered */
        if (!(arr[i] > arr[i+1]) && !(arr[i] < arr[i+1])) {
            count++;
        }
        
        /* Generate UNORDERED with NaN */
        if (__builtin_isunordered(arr[i], NAN)) {
            count++;
        }
    }
    
    return count;
}

int main() {
    /* Create arrays with NaN values to ensure unordered comparisons */
    float fa[32], fb[32];
    double da[16], db[16];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < 32; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 5 == 0) ? NAN : (float)(i * 0.5);
    }
    
    for (int i = 0; i < 16; i++) {
        da[i] = (i % 4 == 0) ? NAN : (double)i;
        db[i] = (i % 3 == 0) ? NAN : (double)(i * 0.7);
    }
    
    int checksum = 0;
    
    /* Test all functions */
    checksum += test_scalar_builtins(fa[0], fb[0]);
    checksum += test_scalar_builtins(NAN, 1.0f);
    checksum += test_scalar_builtins(1.0f, NAN);
    checksum += test_scalar_builtins(2.0f, 1.0f);
    checksum += test_scalar_builtins(1.0f, 2.0f);
    checksum += test_scalar_builtins(1.0f, 1.0f);
    
    checksum += test_sse_intrinsics(fa, fb, 32);
    
    checksum += test_inline_asm(fa[1], fb[1]);
    checksum += test_inline_asm(NAN, fb[2]);
    checksum += test_inline_asm(fa[3], NAN);
    
    checksum += test_double_operations(da[0], db[0]);
    checksum += test_double_operations(NAN, db[1]);
    checksum += test_double_operations(da[2], NAN);
    checksum += test_double_operations(3.0, 2.0);
    checksum += test_double_operations(2.0, 3.0);
    checksum += test_double_operations(2.0, 2.0);
    
    checksum += test_mixed_nan(fa, 16);
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
