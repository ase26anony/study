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
    
    /* Each builtin should generate different condition codes */
    if (__builtin_isunordered(a, b)) result |= 1;    /* UNORDERED */
    if (!__builtin_isunordered(a, b)) result |= 2;   /* ORDERED */
    if (__builtin_isgreater(a, b)) result |= 4;      /* UNGT or GT */
    if (__builtin_isless(a, b)) result |= 8;         /* UNLT or LT */
    if (__builtin_isgreaterequal(a, b)) result |= 16; /* UNGE or GE */
    if (__builtin_islessequal(a, b)) result |= 32;   /* UNLE or LE */
    
    /* Force generation of UNEQ and LTGT through complex conditions */
    if (__builtin_isunordered(c, d) || c == d) result |= 64;  /* May generate UNEQ */
    if (c != d && !__builtin_isunordered(c, d)) result |= 128; /* May generate LTGT */
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *a, float *b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* Use various unordered comparison intrinsics */
        __m128 cmp_unord = _mm_cmpunord_ps(va, vb);    /* UNORDERED */
        __m128 cmp_ord = _mm_cmpord_ps(va, vb);        /* ORDERED */
        __m128 cmp_nlt = _mm_cmpnlt_ps(va, vb);        /* UNGE (nlt) */
        __m128 cmp_nle = _mm_cmpnle_ps(va, vb);        /* UNGT (nle) */
        __m128 cmp_ule = _mm_cmpule_ps(va, vb);        /* UNLE (ule) */
        __m128 cmp_ult = _mm_cmpult_ps(va, vb);        /* UNLT (ult) */
        __m128 cmp_neq = _mm_cmpneq_ps(va, vb);        /* LTGT (une) */
        
        /* Mix results to prevent optimization */
        sum = _mm_add_ps(sum, cmp_unord);
        sum = _mm_add_ps(sum, cmp_ord);
        sum = _mm_add_ps(sum, cmp_nlt);
        sum = _mm_add_ps(sum, cmp_nle);
        sum = _mm_add_ps(sum, cmp_ule);
        sum = _mm_add_ps(sum, cmp_ult);
        sum = _mm_add_ps(sum, cmp_neq);
    }
    
    /* Extract result */
    float res[4];
    _mm_storeu_ps(res, sum);
    return (int)(res[0] + res[1] + res[2] + res[3]);
}

__attribute__((noinline))
int test_inline_asm(float x, float y) {
    int result = 0;
    int out;
    
    /* Inline asm with various condition codes */
    asm volatile ("ucomiss %1, %2\n\t"
                  "setu %0" 
                  : "=r"(out) : "x"(x), "x"(y));
    result += out;
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "setg %0" 
                  : "=r"(out) : "x"(x), "x"(y));
    result += out;
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "setl %0" 
                  : "=r"(out) : "x"(x), "x"(y));
    result += out;
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "sete %0" 
                  : "=r"(out) : "x"(x), "x"(y));
    result += out;
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "setne %0" 
                  : "=r"(out) : "x"(x), "x"(y));
    result += out;
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "seta %0" 
                  : "=r"(out) : "x"(x), "x"(y));
    result += out;
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "setae %0" 
                  : "=r"(out) : "x"(x), "x"(y));
    result += out;
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "setb %0" 
                  : "=r"(out) : "x"(x), "x"(y));
    result += out;
    
    asm volatile ("ucomiss %1, %2\n\t"
                  "setbe %0" 
                  : "=r"(out) : "x"(x), "x"(y));
    result += out;
    
    return result;
}

__attribute__((noinline))
int test_avx_intrinsics(double *a, double *b, int n) {
    __m256d sum = _mm256_setzero_pd();
    
    for (int i = 0; i < n; i += 4) {
        __m256d va = _mm256_loadu_pd(&a[i]);
        __m256d vb = _mm256_loadu_pd(&b[i]);
        
        /* AVX unordered comparisons */
        __m256d cmp_unord = _mm256_cmpunord_pd(va, vb);
        __m256d cmp_ord = _mm256_cmpord_pd(va, vb);
        __m256d cmp_nlt = _mm256_cmpnlt_pd(va, vb);
        __m256d cmp_nle = _mm256_cmpnle_pd(va, vb);
        __m256d cmp_ule = _mm256_cmpule_pd(va, vb);
        __m256d cmp_ult = _mm256_cmpult_pd(va, vb);
        __m256d cmp_neq = _mm256_cmpneq_pd(va, vb);
        
        sum = _mm256_add_pd(sum, cmp_unord);
        sum = _mm256_add_pd(sum, cmp_ord);
        sum = _mm256_add_pd(sum, cmp_nlt);
        sum = _mm256_add_pd(sum, cmp_nle);
        sum = _mm256_add_pd(sum, cmp_ule);
        sum = _mm256_add_pd(sum, cmp_ult);
        sum = _mm256_add_pd(sum, cmp_neq);
    }
    
    double res[4];
    _mm256_storeu_pd(res, sum);
    return (int)(res[0] + res[1] + res[2] + res[3]);
}

__attribute__((noinline))
int test_mixed_conditions(float *arr, int n) {
    int count = 0;
    
    for (int i = 0; i < n - 1; i++) {
        float x = arr[i];
        float y = arr[i + 1];
        
        /* Mixed conditions to trigger various code paths */
        if (isnan(x) || isnan(y)) count++;  /* May use UNORDERED */
        if (!isnan(x) && !isnan(y)) count++; /* May use ORDERED */
        
        if (x > y) count++;      /* May use UNGT */
        if (x < y) count++;      /* May use UNLT */
        if (x >= y) count++;     /* May use UNGE */
        if (x <= y) count++;     /* May use UNLE */
        
        /* Complex condition for UNEQ */
        if ((isnan(x) && isnan(y)) || x == y) count++;
        
        /* Complex condition for LTGT */
        if (x != y && !(isnan(x) || isnan(y))) count++;
    }
    
    return count;
}

int main() {
    const int N = 128;
    float fa[N], fb[N];
    double da[N], db[N];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < N; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 5 == 0) ? NAN : (float)(i * 0.5);
        da[i] = (i % 4 == 0) ? NAN : (double)i;
        db[i] = (i % 7 == 0) ? NAN : (double)(i * 0.3);
    }
    
    int checksum = 0;
    
    /* Test all functions */
    checksum += test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    checksum += test_sse_intrinsics(fa, fb, N);
    checksum += test_inline_asm(fa[1], fb[1]);
    checksum += test_avx_intrinsics(da, db, N);
    checksum += test_mixed_conditions(fa, N);
    
    /* Use checksum to prevent optimization */
    printf("Result: %d\n", checksum);
    
    return 0;
}
