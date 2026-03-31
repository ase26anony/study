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
    if (__builtin_isgreater(a, b)) result |= 4;      /* UNGT or GT */
    if (__builtin_isless(a, b)) result |= 8;         /* UNLT or LT */
    if (__builtin_isgreaterequal(a, b)) result |= 16; /* UNGE or GE */
    if (__builtin_islessequal(a, b)) result |= 32;   /* UNLE or LE */
    
    /* Use fpclassify which may generate UNEQ/LTGT */
    if (fpclassify(a) == FP_NAN) result |= 64;
    if (fpclassify(b) == FP_INFINITE) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_vector_intrinsics(float *a, float *b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* Use various unordered comparisons */
        __m128 cmp1 = _mm_cmpunord_ps(va, vb);   /* UNORDERED */
        __m128 cmp2 = _mm_cmpord_ps(va, vb);     /* ORDERED */
        __m128 cmp3 = _mm_cmpnlt_ps(va, vb);     /* UNGE (nlt) */
        __m128 cmp4 = _mm_cmpnle_ps(va, vb);     /* UNGT (nle) */
        __m128 cmp5 = _mm_cmpneq_ps(va, vb);     /* UNEQ or NEQ */
        
        /* Mix them to prevent optimization */
        __m128 t1 = _mm_and_ps(cmp1, cmp2);
        __m128 t2 = _mm_or_ps(cmp3, cmp4);
        __m128 res = _mm_add_ps(t1, t2);
        sum = _mm_add_ps(sum, res);
    }
    
    /* Extract some result */
    float temp[4];
    _mm_storeu_ps(temp, sum);
    return (int)(temp[0] + temp[1] + temp[2] + temp[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int r1, r2, r3, r4, r5, r6, r7, r8;
    
    /* Inline assembly with various condition codes */
    asm volatile (
        /* UNORDERED - 'u' flag */
        "setu %0\n\t"
        /* ORDERED - 'no' flag (not unordered) */
        "setno %1\n\t"
        /* UNEQ - unordered or equal (parity flag) */
        "setp %2\n\t"
        /* UNGE - not less than (nl) */
        "setnl %3\n\t"
        /* UNGT - not less or equal (nle) */
        "setnle %4\n\t"
        /* UNLE - less or equal (le) for unordered semantics */
        "setle %5\n\t"
        /* UNLT - less than (l) for unordered semantics */
        "setl %6\n\t"
        /* LTGT - not equal (ne) for unordered semantics */
        "setne %7"
        : "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4), 
          "=r"(r5), "=r"(r6), "=r"(r7), "=r"(r8)
        : 
        : "cc"
    );
    
    result = r1 + (r2 << 1) + (r3 << 2) + (r4 << 3) + 
             (r5 << 4) + (r6 << 5) + (r7 << 6) + (r8 << 7);
    
    /* More inline asm with explicit condition code constraints */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "fstp %%st(0)"
        : "=@ccu"(r1), "=@ccno"(r2), "=@ccp"(r3)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    
    return result + r1 + r2 + r3;
}

__attribute__((noinline))
int test_mixed_comparisons(double x, double y) {
    volatile double a = x;
    volatile double b = y;
    int result = 0;
    
    /* Force generation of LTGT (une) */
    if (!(a == b) && !__builtin_isunordered(a, b))
        result |= 1;
    
    /* Force UNEQ (ueq) */
    if (__builtin_isunordered(a, b) || a == b)
        result |= 2;
    
    /* Complex expression for UNLE/UNLT */
    if (a < b || __builtin_isunordered(a, b))
        result |= 4;
    
    if (a <= b || __builtin_isunordered(a, b))
        result |= 8;
    
    return result;
}

/* Main function with runtime values */
int main() {
    /* Create arrays with NaN values to ensure unordered comparisons */
    float fa[32], fb[32];
    double da[16], db[16];
    
    /* Initialize with mix of normal and NaN values */
    for (int i = 0; i < 32; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 5 == 0) ? NAN : (float)(i * 2);
    }
    
    for (int i = 0; i < 16; i++) {
        da[i] = (i % 4 == 0) ? NAN : (double)i;
        db[i] = (i % 7 == 0) ? NAN : (double)(i * 3);
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(fa[0], fb[0]);
    checksum += test_scalar_builtins(1.0f, NAN);
    checksum += test_scalar_builtins(NAN, 2.0f);
    checksum += test_scalar_builtins(NAN, NAN);
    
    /* Test vector intrinsics */
    checksum += test_vector_intrinsics(fa, fb, 32);
    
    /* Test inline assembly */
    checksum += test_inline_asm(fa[1], fb[1]);
    checksum += test_inline_asm(NAN, 1.0f);
    
    /* Test mixed comparisons */
    checksum += test_mixed_comparisons(da[0], db[0]);
    checksum += test_mixed_comparisons(NAN, db[1]);
    checksum += test_mixed_comparisons(da[2], NAN);
    checksum += test_mixed_comparisons(NAN, NAN);
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
