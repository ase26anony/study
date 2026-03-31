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
    if (!__builtin_isunordered(a, b)) result |= 2;
    
    /* UNEQ case (unordered or equal) */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) result |= 4;
    
    /* UNGE case (not less than) */
    if (!__builtin_isless(a, b)) result |= 8;
    
    /* UNGT case (not less than or equal) */
    if (!__builtin_islessequal(a, b)) result |= 16;
    
    /* UNLE case (unordered or less than or equal) */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) result |= 32;
    
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) result |= 64;
    
    /* LTGT case (less than or greater than, but not equal and not unordered) */
    if ((__builtin_isless(c, d) || __builtin_isgreater(c, d)) && 
        !__builtin_isunordered(c, d)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *a, float *b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _mm_cmpunord_ps */
        __m128 mask_unord = _mm_cmpunord_ps(va, vb);
        
        /* ORDERED - _mm_cmpord_ps */
        __m128 mask_ord = _mm_cmpord_ps(va, vb);
        
        /* UNGE - _mm_cmpnlt_ps (not less than) */
        __m128 mask_nlt = _mm_cmpnlt_ps(va, vb);
        
        /* UNGT - _mm_cmpnle_ps (not less than or equal) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        
        /* UNLE - _mm_cmple_ps (less than or equal) - will be transformed */
        __m128 mask_ule = _mm_cmple_ps(va, vb);
        
        /* UNLT - _mm_cmplt_ps (less than) - will be transformed */
        __m128 mask_ult = _mm_cmplt_ps(va, vb);
        
        /* LTGT - _mm_cmpneq_ps (not equal) - can generate une/ltgt */
        __m128 mask_neq = _mm_cmpneq_ps(va, vb);
        
        /* UNEQ - _mm_cmpeq_ps (equal) - will be transformed with unordered */
        __m128 mask_eq = _mm_cmpeq_ps(va, vb);
        
        /* Combine masks */
        __m128 combined = _mm_add_ps(mask_unord, mask_ord);
        combined = _mm_add_ps(combined, mask_nlt);
        combined = _mm_add_ps(combined, mask_nle);
        combined = _mm_add_ps(combined, mask_ule);
        combined = _mm_add_ps(combined, mask_ult);
        combined = _mm_add_ps(combined, mask_neq);
        combined = _mm_add_ps(combined, mask_eq);
        
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
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(r1) : : "cc");
    result += r1;
    
    /* ORDERED - "no" flag (not overflow, but used for ordered) */
    /* Note: x86 uses "np" (parity) for unordered, "p" for ordered */
    asm volatile ("setnp %0" : "=r"(r2) : : "cc");
    result += r2;
    
    /* UNEQ - "e" flag (equal) with unordered handling */
    asm volatile ("sete %0" : "=r"(r3) : : "cc");
    result += r3;
    
    /* UNGE - "nl" flag (not less than) */
    asm volatile ("setnl %0" : "=r"(r4) : : "cc");
    result += r4;
    
    /* UNGT - "nle" flag (not less than or equal) */
    asm volatile ("setnle %0" : "=r"(r5) : : "cc");
    result += r5;
    
    /* UNLE - "le" flag (less than or equal) */
    asm volatile ("setle %0" : "=r"(r6) : : "cc");
    result += r6;
    
    /* UNLT - "l" flag (less than) */
    asm volatile ("setl %0" : "=r"(r7) : : "cc");
    result += r7;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(r8) : : "cc");
    result += r8;
    
    return result;
}

__attribute__((noinline))
int test_avx_intrinsics(double *a, double *b, int n) {
    __m256d sum = _mm256_setzero_pd();
    
    for (int i = 0; i < n; i += 4) {
        __m256d va = _mm256_loadu_pd(&a[i]);
        __m256d vb = _mm256_loadu_pd(&b[i]);
        
        /* Use various AVX comparison predicates */
        __m256d mask1 = _mm256_cmp_pd(va, vb, _CMP_UNORD_Q);    /* UNORDERED */
        __m256d mask2 = _mm256_cmp_pd(va, vb, _CMP_ORD_Q);     /* ORDERED */
        __m256d mask3 = _mm256_cmp_pd(va, vb, _CMP_NLT_UQ);    /* UNGE */
        __m256d mask4 = _mm256_cmp_pd(va, vb, _CMP_NLE_UQ);    /* UNGT */
        __m256d mask5 = _mm256_cmp_pd(va, vb, _CMP_LE_OS);     /* UNLE */
        __m256d mask6 = _mm256_cmp_pd(va, vb, _CMP_LT_OS);     /* UNLT */
        __m256d mask7 = _mm256_cmp_pd(va, vb, _CMP_NEQ_OS);    /* LTGT */
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

int main() {
    /* Create arrays with NaN values to ensure unordered comparisons */
    float fa[16], fb[16];
    double da[16], db[16];
    
    for (int i = 0; i < 16; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 5 == 0) ? NAN : (float)(i * 0.5);
        da[i] = (i % 4 == 0) ? NAN : (double)i;
        db[i] = (i % 6 == 0) ? NAN : (double)(i * 0.3);
    }
    
    int checksum = 0;
    
    /* Test scalar builtins with various inputs */
    checksum += test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    checksum += test_scalar_builtins(1.0f, NAN, 2.0, NAN);
    checksum += test_scalar_builtins(NAN, 1.0f, NAN, 2.0);
    checksum += test_scalar_builtins(1.0f, 2.0f, 1.0, 2.0);
    checksum += test_scalar_builtins(2.0f, 1.0f, 2.0, 1.0);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, 16);
    
    /* Test AVX intrinsics if available */
    checksum += test_avx_intrinsics(da, db, 16);
    
    /* Test inline assembly */
    checksum += test_inline_asm(fa[1], fb[1]);
    checksum += test_inline_asm(NAN, 1.0f);
    checksum += test_inline_asm(1.0f, NAN);
    
    /* Additional tests with fpclassify and isnan */
    for (int i = 0; i < 16; i++) {
        if (isnan(fa[i])) checksum++;
        if (fpclassify(fa[i]) == FP_NAN) checksum++;
        if (isunordered(fa[i], fb[i])) checksum++;
        if (!isgreater(fa[i], fb[i])) checksum++;
        if (!isless(fa[i], fb[i])) checksum++;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
