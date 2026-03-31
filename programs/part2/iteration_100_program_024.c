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
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case */
    if (!__builtin_isunordered(a, b)) result |= 2;
    
    /* UNEQ case (unordered or equal) - use with fast-math */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) result |= 4;
    
    /* UNGE case (not less than) */
    if (!__builtin_isless(a, b)) result |= 8;
    
    /* UNGT case (not less than or equal) */
    if (!__builtin_islessequal(a, b)) result |= 16;
    
    /* UNLE case (unordered or less than or equal) */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) result |= 32;
    
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) result |= 64;
    
    /* LTGT case (less than or greater than, but not equal) */
    if (__builtin_isless(c, d) || __builtin_isgreater(c, d)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *a, float *b, int n) {
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
        
        /* UNGT: _mm_cmpnle_ps (not less than or equal) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        
        /* UNLE: _mm_cmple_ps (less than or equal) with unordered handling */
        __m128 mask_ule = _mm_cmple_ps(va, vb);
        
        /* UNLT: _mm_cmplt_ps (less than) with unordered handling */
        __m128 mask_ult = _mm_cmplt_ps(va, vb);
        
        /* LTGT: _mm_cmpneq_ps (not equal) */
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
    int temp;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED: "u" flag */
    asm volatile ("setu %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* ORDERED: "no" flag (not overflow, but we need ordered) */
    /* Using multiple asm statements to trigger different codes */
    
    /* UNEQ: "e" flag (equal) with unordered context */
    asm volatile ("sete %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNGE: "ge" flag (greater or equal) */
    asm volatile ("setge %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNGT: "g" flag (greater) */
    asm volatile ("setg %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNLE: "le" flag (less or equal) */
    asm volatile ("setle %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNLT: "l" flag (less) */
    asm volatile ("setl %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* LTGT: "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(temp) : : "cc");
    result += temp;
    
    return result;
}

__attribute__((noinline))
int test_avx_intrinsics(double *a, double *b, int n) {
    __m256d sum = _mm256_setzero_pd();
    
    for (int i = 0; i < n; i += 4) {
        __m256d va = _mm256_loadu_pd(&a[i]);
        __m256d vb = _mm256_loadu_pd(&b[i]);
        
        /* AVX versions of the comparisons */
        __m256d mask_unord = _mm256_cmp_pd(va, vb, _CMP_UNORD_Q);
        __m256d mask_ord = _mm256_cmp_pd(va, vb, _CMP_ORD_Q);
        __m256d mask_nlt = _mm256_cmp_pd(va, vb, _CMP_NLT_UQ);  /* UNGE */
        __m256d mask_nle = _mm256_cmp_pd(va, vb, _CMP_NLE_UQ);  /* UNGT */
        __m256d mask_ule = _mm256_cmp_pd(va, vb, _CMP_LE_UQ);   /* UNLE */
        __m256d mask_ult = _mm256_cmp_pd(va, vb, _CMP_LT_UQ);   /* UNLT */
        __m256d mask_une = _mm256_cmp_pd(va, vb, _CMP_NEQ_UQ);  /* LTGT */
        
        /* Combine masks */
        __m256d combined = _mm256_add_pd(mask_unord, mask_ord);
        combined = _mm256_add_pd(combined, mask_nlt);
        combined = _mm256_add_pd(combined, mask_nle);
        combined = _mm256_add_pd(combined, mask_ule);
        combined = _mm256_add_pd(combined, mask_ult);
        combined = _mm256_add_pd(combined, mask_une);
        
        sum = _mm256_add_pd(sum, combined);
    }
    
    /* Extract result */
    double result[4];
    _mm256_storeu_pd(result, sum);
    return (int)(result[0] + result[1] + result[2] + result[3]);
}

int main() {
    /* Create test data with NaN values to trigger unordered comparisons */
    float fdata1[16], fdata2[16];
    double ddata1[16], ddata2[16];
    
    for (int i = 0; i < 16; i++) {
        fdata1[i] = (i % 3 == 0) ? NAN : (float)i;
        fdata2[i] = (i % 4 == 0) ? NAN : (float)(i * 2);
        ddata1[i] = (i % 3 == 0) ? NAN : (double)i;
        ddata2[i] = (i % 4 == 0) ? NAN : (double)(i * 2);
    }
    
    /* Test all comparison types */
    int checksum = 0;
    
    checksum += test_scalar_builtins(fdata1[0], fdata2[0], ddata1[0], ddata2[0]);
    checksum += test_sse_intrinsics(fdata1, fdata2, 16);
    checksum += test_inline_asm(fdata1[1], fdata2[1]);
    checksum += test_avx_intrinsics(ddata1, ddata2, 16);
    
    /* Use the result to prevent optimization */
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
