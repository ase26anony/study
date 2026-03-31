/* Test program to cover condition code output in i386.cc */
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
    if (!__builtin_isunordered(c, d)) result |= 2;     /* ORDERED */
    if (__builtin_isgreater(a, b)) result |= 4;        /* UNGT or GT */
    if (__builtin_isless(a, b)) result |= 8;           /* UNLT or LT */
    if (__builtin_isgreaterequal(a, b)) result |= 16;  /* UNGE or GE */
    if (__builtin_islessequal(a, b)) result |= 32;     /* UNLE or LE */
    
    /* Use fpclassify to potentially generate UNEQ/LTGT */
    if (fpclassify(a) == FP_NAN) result |= 64;
    if (isnan(b)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *fa, float *fb, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 a = _mm_loadu_ps(fa + i);
        __m128 b = _mm_loadu_ps(fb + i);
        
        /* Use various unordered comparisons */
        __m128 cmp1 = _mm_cmpunord_ps(a, b);    /* UNORDERED */
        __m128 cmp2 = _mm_cmpord_ps(a, b);      /* ORDERED */
        __m128 cmp3 = _mm_cmpnlt_ps(a, b);      /* UNGE (nlt) */
        __m128 cmp4 = _mm_cmpnle_ps(a, b);      /* UNGT (nle) */
        __m128 cmp5 = _mm_cmpneq_ps(a, b);      /* LTGT (une) for unordered */
        
        /* Mix them to prevent optimization */
        __m128 t1 = _mm_and_ps(a, cmp1);
        __m128 t2 = _mm_and_ps(b, cmp2);
        __m128 t3 = _mm_or_ps(cmp3, cmp4);
        __m128 t4 = _mm_xor_ps(t1, t2);
        
        sum = _mm_add_ps(sum, _mm_add_ps(t3, t4));
    }
    
    /* Extract result */
    float r[4];
    _mm_storeu_ps(r, sum);
    return (int)(r[0] + r[1] + r[2] + r[3]);
}

__attribute__((noinline))
int test_inline_asm(float x, float y) {
    int result = 0;
    int tmp;
    
    /* Inline asm with various condition codes */
    asm volatile (
        /* UNORDERED - "u" flag */
        "setu %0\n\t"
        : "=r"(tmp)
        : 
        : "cc"
    );
    result += tmp;
    
    asm volatile (
        /* ORDERED - "no" flag (not unordered) */
        "setno %0\n\t"
        : "=r"(tmp)
        : 
        : "cc"
    );
    result += tmp;
    
    /* UNGE - "ae" or "nb" (not below/not less) */
    asm volatile (
        "setae %0\n\t"
        : "=r"(tmp)
        : 
        : "cc"
    );
    result += tmp;
    
    /* UNGT - "a" or "nbe" (not below or equal) */
    asm volatile (
        "seta %0\n\t"
        : "=r"(tmp)
        : 
        : "cc"
    );
    result += tmp;
    
    /* UNLE - "be" (below or equal) for unordered semantics */
    asm volatile (
        "setbe %0\n\t"
        : "=r"(tmp)
        : 
        : "cc"
    );
    result += tmp;
    
    /* UNLT - "b" or "nae" (below/not above or equal) */
    asm volatile (
        "setb %0\n\t"
        : "=r"(tmp)
        : 
        : "cc"
    );
    result += tmp;
    
    /* LTGT - "ne" (not equal) for unordered not equal */
    asm volatile (
        "setne %0\n\t"
        : "=r"(tmp)
        : 
        : "cc"
    );
    result += tmp;
    
    return result;
}

__attribute__((noinline))
int test_avx_comparisons(double *da, double *db, int n) {
    __m256d sum = _mm256_setzero_pd();
    
    for (int i = 0; i < n; i += 4) {
        __m256d a = _mm256_loadu_pd(da + i);
        __m256d b = _mm256_loadu_pd(db + i);
        
        /* AVX comparisons with immediate predicates */
        /* _CMP_UNORD_Q = 3 (unordered) */
        __m256d cmp1 = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
        /* _CMP_ORD_Q = 7 (ordered) */
        __m256d cmp2 = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
        /* _CMP_NLT_UQ = 5 (not less than unordered) - UNGE */
        __m256d cmp3 = _mm256_cmp_pd(a, b, _CMP_NLT_UQ);
        /* _CMP_NLE_UQ = 6 (not less than or equal unordered) - UNGT */
        __m256d cmp4 = _mm256_cmp_pd(a, b, _CMP_NLE_UQ);
        /* _CMP_NEQ_UQ = 4 (not equal unordered) - LTGT */
        __m256d cmp5 = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
        /* _CMP_LE_UQ = 2 (less than or equal unordered) - UNLE */
        __m256d cmp6 = _mm256_cmp_pd(a, b, _CMP_LE_UQ);
        /* _CMP_LT_UQ = 1 (less than unordered) - UNLT */
        __m256d cmp7 = _mm256_cmp_pd(a, b, _CMP_LT_UQ);
        
        /* Combine results to prevent optimization */
        __m256d t1 = _mm256_and_pd(a, cmp1);
        __m256d t2 = _mm256_and_pd(b, cmp2);
        __m256d t3 = _mm256_or_pd(cmp3, cmp4);
        __m256d t4 = _mm256_xor_pd(cmp5, cmp6);
        __m256d t5 = _mm256_add_pd(t1, _mm256_add_pd(t2, _mm256_add_pd(t3, t4)));
        
        sum = _mm256_add_pd(sum, _mm256_mul_pd(t5, cmp7));
    }
    
    double r[4];
    _mm256_storeu_pd(r, sum);
    return (int)(r[0] + r[1] + r[2] + r[3]);
}

/* Mix of values including NaN to trigger unordered comparisons */
void init_arrays(float *fa, float *fb, double *da, double *db, int n) {
    for (int i = 0; i < n; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)(i * 1.1);
        fb[i] = (i % 5 == 0) ? NAN : (float)(i * 0.9);
        da[i] = (i % 4 == 0) ? NAN : (double)(i * 1.3);
        db[i] = (i % 7 == 0) ? NAN : (double)(i * 0.7);
    }
}

int main() {
    const int N = 256;
    float fa[N], fb[N];
    double da[N], db[N];
    
    init_arrays(fa, fb, da, db, N);
    
    int checksum = 0;
    
    /* Test scalar builtins with various inputs */
    checksum += test_scalar_builtins(1.0f, 2.0f, 3.0, 4.0);
    checksum += test_scalar_builtins(NAN, 2.0f, NAN, 4.0);
    checksum += test_scalar_builtins(1.0f, NAN, 3.0, NAN);
    checksum += test_scalar_builtins(NAN, NAN, NAN, NAN);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, N);
    
    /* Test inline assembly */
    checksum += test_inline_asm(1.0f, 2.0f);
    checksum += test_inline_asm(NAN, 2.0f);
    checksum += test_inline_asm(1.0f, NAN);
    
    /* Test AVX comparisons */
    checksum += test_avx_comparisons(da, db, N);
    
    /* Use checksum to prevent optimization */
    printf("Result: %d\n", checksum);
    
    return 0;
}
