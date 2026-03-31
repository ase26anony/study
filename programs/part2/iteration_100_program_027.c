/* Test program to cover floating-point condition code output in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b, double c, double d) {
    int result = 0;
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case */
    if (__builtin_isordered(c, d)) result |= 2;
    
    /* UNEQ case (unordered or equal) - using isnan to force */
    if (isnan(a) || a == b) result |= 4;
    
    /* UNGE case (not less than) */
    if (__builtin_isgreaterequal(a, b)) result |= 8;
    
    /* UNGT case (not less than or equal) */
    if (__builtin_isgreater(c, d)) result |= 16;
    
    /* UNLE case (unordered or less than or equal) */
    if (__builtin_islessequal(a, b)) result |= 32;
    
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(c, d)) result |= 64;
    
    /* LTGT case (less than or greater than, but not equal) */
    if (a != b && !isnan(a) && !isnan(b)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *fa, float *fb, int n) {
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
        
        /* UNGE (nlt) */
        cmp = _mm_cmpnlt_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNGT (nle) */
        cmp = _mm_cmpnle_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNLE (unordered or less than or equal) */
        cmp = _mm_cmple_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* UNLT (unordered or less than) */
        cmp = _mm_cmplt_ps(a, b);
        result += _mm_movemask_ps(cmp);
        
        /* LTGT (unequal, ordered) - use cmpneq */
        cmp = _mm_cmpneq_ps(a, b);
        result += _mm_movemask_ps(cmp);
    }
    
    return result;
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int tmp;
    
    /* Inline assembly with condition code constraints */
    
    /* UNORDERED - "u" flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setu %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp;
    
    /* ORDERED - "np" flag (not parity = ordered) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp;
    
    /* UNEQ - "e" flag (equal) with unordered */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp;
    
    /* UNGE - "ae" flag (above or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp;
    
    /* UNGT - "a" flag (above) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp;
    
    /* UNLE - "be" flag (below or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp;
    
    /* UNLT - "b" flag (below) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp;
    
    return result;
}

__attribute__((noinline))
int test_avx_intrinsics(float *fa, float *fb, int n) {
#ifdef __AVX__
    __m256 a, b, cmp;
    int result = 0;
    
    for (int i = 0; i < n; i += 8) {
        a = _mm256_loadu_ps(&fa[i]);
        b = _mm256_loadu_ps(&fb[i]);
        
        /* Various condition codes using AVX */
        cmp = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);
        result += _mm256_movemask_ps(cmp);
        
        cmp = _mm256_cmp_ps(a, b, _CMP_ORD_Q);
        result += _mm256_movemask_ps(cmp);
        
        cmp = _mm256_cmp_ps(a, b, _CMP_NLT_UQ);
        result += _mm256_movemask_ps(cmp);
        
        cmp = _mm256_cmp_ps(a, b, _CMP_NLE_UQ);
        result += _mm256_movemask_ps(cmp);
        
        cmp = _mm256_cmp_ps(a, b, _CMP_LE_OS);
        result += _mm256_movemask_ps(cmp);
        
        cmp = _mm256_cmp_ps(a, b, _CMP_LT_OS);
        result += _mm256_movemask_ps(cmp);
        
        cmp = _mm256_cmp_ps(a, b, _CMP_NEQ_OS);
        result += _mm256_movemask_ps(cmp);
    }
    
    return result;
#else
    return 0;
#endif
}

int main() {
    const int N = 256;
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    
    /* Initialize with mix of normal numbers and NaN */
    for (int i = 0; i < N; i++) {
        fa[i] = (i % 2 == 0) ? (float)i : NAN;
        fb[i] = (i % 3 == 0) ? (float)(i * 2) : NAN;
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(fa[0], fb[0], fa[1], fb[1]);
    checksum += test_scalar_builtins(fa[2], fb[2], fa[3], fb[3]);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, N);
    
    /* Test inline assembly */
    checksum += test_inline_asm(fa[4], fb[4]);
    checksum += test_inline_asm(fa[5], fb[5]);
    
    /* Test AVX intrinsics if available */
    checksum += test_avx_intrinsics(fa, fb, N);
    
    /* Additional tests with doubles */
    double da = NAN, db = 3.14, dc = 2.71, dd = NAN;
    checksum += test_scalar_builtins(da, db, dc, dd);
    
    /* Force generation of condition codes in loops */
    volatile float v = 0.0f;
    for (int i = 0; i < 10; i++) {
        if (__builtin_isunordered(fa[i], fb[i])) v += 1.0f;
        if (__builtin_isgreater(fa[i], fb[i])) v += 2.0f;
        if (__builtin_isless(fa[i], fb[i])) v += 3.0f;
    }
    
    printf("Checksum: %d (dummy value: %f)\n", checksum, v);
    
    free(fa);
    free(fb);
    
    return 0;
}
