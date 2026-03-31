/* Test program to cover condition code output in i386.cc */
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b, double c, double d) {
    int result = 0;
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case */
    if (__builtin_isordered(c, d)) result |= 2;
    
    /* UNEQ case - unordered or equal */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) result |= 4;
    
    /* UNGE case - not less than (greater or equal or unordered) */
    if (!__builtin_isless(a, b)) result |= 8;
    
    /* UNGT case - not less than or equal (greater or unordered) */
    if (!__builtin_islessequal(a, b)) result |= 16;
    
    /* UNLE case - less or equal or unordered */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) result |= 32;
    
    /* UNLT case - less than or unordered */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) result |= 64;
    
    /* LTGT case - less than or greater than (ordered and not equal) */
    if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) result |= 128;
    
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
        
        /* UNLE - _mm_cmple_ps with unordered handling */
        __m128 mask_ule = _mm_cmple_ps(va, vb);
        
        /* UNLT - _mm_cmplt_ps with unordered handling */
        __m128 mask_ult = _mm_cmplt_ps(va, vb);
        
        /* LTGT - _mm_cmpneq_ps (not equal, ordered) */
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
    
    /* UNORDERED - "u" flag */
    asm volatile (
        "setu %0"
        : "=r"(temp)
        :
        : "cc"
    );
    result += temp;
    
    /* ORDERED - "no" flag (not overflow, used for ordered) */
    asm volatile (
        "setno %0"
        : "=r"(temp)
        :
        : "cc"
    );
    result += temp;
    
    /* UNEQ - "e" flag (equal) with unordered context */
    asm volatile (
        "sete %0"
        : "=r"(temp)
        :
        : "cc"
    );
    result += temp;
    
    /* UNGE - "ge" flag (greater or equal) */
    asm volatile (
        "setge %0"
        : "=r"(temp)
        :
        : "cc"
    );
    result += temp;
    
    /* UNGT - "g" flag (greater) */
    asm volatile (
        "setg %0"
        : "=r"(temp)
        :
        : "cc"
    );
    result += temp;
    
    /* UNLE - "le" flag (less or equal) */
    asm volatile (
        "setle %0"
        : "=r"(temp)
        :
        : "cc"
    );
    result += temp;
    
    /* UNLT - "l" flag (less) */
    asm volatile (
        "setl %0"
        : "=r"(temp)
        :
        : "cc"
    );
    result += temp;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile (
        "setne %0"
        : "=r"(temp)
        :
        : "cc"
    );
    result += temp;
    
    return result;
}

__attribute__((noinline))
int test_avx_comparisons(float *a, float *b, int n) {
    __m256 sum = _mm256_setzero_ps();
    
    for (int i = 0; i < n; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        
        /* Use various AVX comparison predicates */
        __m256 mask;
        
        /* CMP_UNORD_Q - UNORDERED */
        mask = _mm256_cmp_ps(va, vb, _CMP_UNORD_Q);
        sum = _mm256_add_ps(sum, mask);
        
        /* CMP_ORD_Q - ORDERED */
        mask = _mm256_cmp_ps(va, vb, _CMP_ORD_Q);
        sum = _mm256_add_ps(sum, mask);
        
        /* CMP_NLT_UQ - UNGE */
        mask = _mm256_cmp_ps(va, vb, _CMP_NLT_UQ);
        sum = _mm256_add_ps(sum, mask);
        
        /* CMP_NLE_UQ - UNGT */
        mask = _mm256_cmp_ps(va, vb, _CMP_NLE_UQ);
        sum = _mm256_add_ps(sum, mask);
        
        /* CMP_LE_OS - UNLE (ordered signaling) */
        mask = _mm256_cmp_ps(va, vb, _CMP_LE_OS);
        sum = _mm256_add_ps(sum, mask);
        
        /* CMP_LT_OS - UNLT (ordered signaling) */
        mask = _mm256_cmp_ps(va, vb, _CMP_LT_OS);
        sum = _mm256_add_ps(sum, mask);
        
        /* CMP_NEQ_OS - LTGT (ordered signaling, not equal) */
        mask = _mm256_cmp_ps(va, vb, _CMP_NEQ_OS);
        sum = _mm256_add_ps(sum, mask);
        
        /* CMP_EQ_UQ - UNEQ (unordered quiet) */
        mask = _mm256_cmp_ps(va, vb, _CMP_EQ_UQ);
        sum = _mm256_add_ps(sum, mask);
    }
    
    /* Extract result */
    float result[8];
    _mm256_storeu_ps(result, sum);
    float total = 0;
    for (int i = 0; i < 8; i++) {
        total += result[i];
    }
    return (int)total;
}

__attribute__((noinline))
int test_mixed_operations(double x, double y) {
    int result = 0;
    
    /* Test with fpclassify and isnan */
    if (fpclassify(x) == FP_NAN) result += 1;
    if (isnan(y)) result += 2;
    
    /* Complex expression that might generate various condition codes */
    if ((x != y) && !isnan(x) && !isnan(y)) result += 4;
    if ((x >= y) || isnan(x) || isnan(y)) result += 8;
    if ((x <= y) && !isnan(x) && !isnan(y)) result += 16;
    
    return result;
}

int main() {
    /* Create arrays with NaN values to ensure unordered comparisons */
    const int SIZE = 64;
    float fa[SIZE], fb[SIZE];
    double da[SIZE], db[SIZE];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < SIZE; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 5 == 0) ? NAN : (float)(i * 0.5);
        da[i] = (i % 4 == 0) ? NAN : (double)i;
        db[i] = (i % 6 == 0) ? NAN : (double)(i * 0.3);
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, SIZE);
    
    /* Test inline assembly */
    checksum += test_inline_asm(fa[1], fb[1]);
    
    /* Test AVX comparisons if available */
    checksum += test_avx_comparisons(fa, fb, SIZE);
    
    /* Test mixed operations */
    checksum += test_mixed_operations(da[2], db[2]);
    
    /* Additional tests with different values */
    for (int i = 0; i < 10; i++) {
        checksum += test_scalar_builtins(
            (float)i, 
            (float)(i * 0.7), 
            (double)(i + 0.5), 
            (double)(i * 0.3)
        );
    }
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}
