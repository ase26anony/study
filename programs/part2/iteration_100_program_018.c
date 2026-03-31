/* Test program to cover floating-point condition code output in i386.cc */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b, double c, double d) {
    int result = 0;
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* ORDERED case */
    if (!__builtin_isunordered(a, b)) {
        result |= 2;
    }
    
    /* UNEQ case (unordered or equal) - use with fast-math */
    if (__builtin_islessequal(a, b) && __builtin_isgreaterequal(a, b)) {
        result |= 4;
    }
    
    /* UNGE case (not less than) */
    if (!__builtin_isless(a, b)) {
        result |= 8;
    }
    
    /* UNGT case (not less or equal) */
    if (!__builtin_islessequal(a, b)) {
        result |= 16;
    }
    
    /* UNLE case (unordered or less or equal) */
    if (__builtin_islessequal(a, b)) {
        result |= 32;
    }
    
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(a, b)) {
        result |= 64;
    }
    
    /* LTGT case (less or greater, ordered) */
    if (__builtin_isless(c, d) || __builtin_isgreater(c, d)) {
        result |= 128;
    }
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(const float* a, const float* b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED: _mm_cmpunord_ps */
        __m128 mask_unord = _mm_cmpunord_ps(va, vb);
        
        /* ORDERED: _mm_cmpord_ps */
        __m128 mask_ord = _mm_cmpord_ps(va, vb);
        
        /* UNGE: _mm_cmpnlt_ps (not less than) */
        __m128 mask_nlt = _mm_cmpnlt_ps(va, vb);
        
        /* UNGT: _mm_cmpnle_ps (not less or equal) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        
        /* UNLE: _mm_cmple_ps (less or equal, unordered) */
        __m128 mask_ule = _mm_cmple_ps(va, vb);
        
        /* UNLT: _mm_cmplt_ps (less than, unordered) */
        __m128 mask_ult = _mm_cmplt_ps(va, vb);
        
        /* LTGT: _mm_cmpneq_ps (not equal, ordered) */
        __m128 mask_une = _mm_cmpneq_ps(va, vb);
        
        /* Combine masks */
        __m128 combined = _mm_add_ps(mask_unord, mask_ord);
        combined = _mm_add_ps(combined, mask_nlt);
        combined = _mm_add_ps(combined, mask_nle);
        combined = _mm_add_ps(combined, mask_ule);
        combined = _mm_add_ps(combined, mask_ult);
        combined = _mm_add_ps(combined, mask_une);
        
        sum = _mm_add_ps(sum, combined);
    }
    
    /* Extract result */
    float result[4];
    _mm_storeu_ps(result, sum);
    return (int)(result[0] + result[1] + result[2] + result[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int r1, r2, r3, r4, r5, r6, r7, r8;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED: "u" flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setu %0"
        : "=r"(r1)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += r1;
    
    /* ORDERED: "no" flag (not overflow, for ordered) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setno %0"
        : "=r"(r2)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += r2;
    
    /* UNEQ: "e" flag (equal, includes unordered) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(r3)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += r3;
    
    /* UNGE: "ae" flag (above or equal, not less than) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r"(r4)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += r4;
    
    /* UNGT: "a" flag (above, not less or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(r5)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += r5;
    
    /* UNLE: "be" flag (below or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(r6)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += r6;
    
    /* UNLT: "b" flag (below, less than) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(r7)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += r7;
    
    /* LTGT: "ne" flag (not equal, ordered) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(r8)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += r8;
    
    return result;
}

__attribute__((noinline))
int test_avx_intrinsics(const double* a, const double* b, int n) {
    __m256d sum = _mm256_setzero_pd();
    
    for (int i = 0; i < n; i += 4) {
        __m256d va = _mm256_loadu_pd(&a[i]);
        __m256d vb = _mm256_loadu_pd(&b[i]);
        
        /* Use various AVX comparison predicates */
        __m256d mask1 = _mm256_cmp_pd(va, vb, _CMP_UNORD_Q);    /* UNORDERED */
        __m256d mask2 = _mm256_cmp_pd(va, vb, _CMP_ORD_Q);     /* ORDERED */
        __m256d mask3 = _mm256_cmp_pd(va, vb, _CMP_NLT_UQ);    /* UNGE */
        __m256d mask4 = _mm256_cmp_pd(va, vb, _CMP_NLE_UQ);    /* UNGT */
        __m256d mask5 = _mm256_cmp_pd(va, vb, _CMP_LE_OQ);     /* UNLE */
        __m256d mask6 = _mm256_cmp_pd(va, vb, _CMP_LT_OQ);     /* UNLT */
        __m256d mask7 = _mm256_cmp_pd(va, vb, _CMP_NEQ_OQ);    /* LTGT */
        __m256d mask8 = _mm256_cmp_pd(va, vb, _CMP_EQ_UQ);     /* UNEQ */
        
        /* Combine results */
        sum = _mm256_add_pd(sum, mask1);
        sum = _mm256_add_pd(sum, mask2);
        sum = _mm256_add_pd(sum, mask3);
        sum = _mm256_add_pd(sum, mask4);
        sum = _mm256_add_pd(sum, mask5);
        sum = _mm256_add_pd(sum, mask6);
        sum = _mm256_add_pd(sum, mask7);
        sum = _mm256_add_pd(sum, mask8);
    }
    
    double result[4];
    _mm256_storeu_pd(result, sum);
    return (int)(result[0] + result[1] + result[2] + result[3]);
}

/* Force generation of conditional jumps with these codes */
__attribute__((noinline))
int test_conditional_jumps(float a, float b) {
    volatile int result = 0;
    
    /* Generate conditional jumps with various codes */
    if (__builtin_isunordered(a, b)) result += 1;      /* UNORDERED */
    if (!__builtin_isunordered(a, b)) result += 2;     /* ORDERED */
    if (a <= b) result += 4;                           /* UNLE */
    if (a < b) result += 8;                            /* UNLT */
    if (!(a < b)) result += 16;                        /* UNGE */
    if (!(a <= b)) result += 32;                       /* UNGT */
    if (a != b) result += 64;                          /* LTGT */
    
    return result;
}

int main() {
    /* Create test data with NaN values to trigger unordered comparisons */
    float fa[16], fb[16];
    double da[16], db[16];
    
    /* Initialize with mix of normal numbers and NaN */
    for (int i = 0; i < 16; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 5 == 0) ? NAN : (float)(i * 0.5);
        da[i] = (i % 4 == 0) ? NAN : (double)i;
        db[i] = (i % 6 == 0) ? NAN : (double)(i * 0.3);
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    checksum += test_scalar_builtins(fa[1], fb[1], da[1], db[1]);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, 16);
    
    /* Test inline assembly */
    checksum += test_inline_asm(fa[2], fb[2]);
    checksum += test_inline_asm(fa[3], fb[3]);
    
    /* Test AVX intrinsics if available */
    checksum += test_avx_intrinsics(da, db, 16);
    
    /* Test conditional jumps */
    checksum += test_conditional_jumps(fa[4], fb[4]);
    checksum += test_conditional_jumps(fa[5], fb[5]);
    
    /* Use fpclassify and isnan */
    for (int i = 0; i < 16; i++) {
        if (fpclassify(fa[i]) == FP_NAN) checksum += i;
        if (isnan(da[i])) checksum += i * 2;
    }
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
