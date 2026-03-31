/* Test program to cover condition code output cases in i386.cc */
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
    
    /* UNORDERED case - __builtin_isunordered */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case - __builtin_islessequal with fast-math may generate ordered */
    if (__builtin_islessequal(a, b)) result |= 2;
    
    /* UNEQ case - equal and unordered? Use isnan checks */
    if (isnan(a) && isnan(b)) result |= 4;
    
    /* UNGE case (nlt) - __builtin_isgreaterequal with fast-math */
    if (__builtin_isgreaterequal(c, d)) result |= 8;
    
    /* UNGT case (nle) - __builtin_isgreater with fast-math */
    if (__builtin_isgreater(c, d)) result |= 16;
    
    /* UNLE case (ule) - __builtin_islessequal with unordered */
    if (!__builtin_isgreater(a, b)) result |= 32;
    
    /* UNLT case (ult) - __builtin_isless with unordered */
    if (__builtin_isless(a, b)) result |= 64;
    
    /* LTGT case (une) - not equal and ordered */
    if (a != b && !__builtin_isunordered(a, b)) result |= 128;
    
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
        
        /* UNGE (nlt) - _mm_cmpnlt_ps */
        __m128 mask_nlt = _mm_cmpnlt_ps(va, vb);
        
        /* UNGT (nle) - _mm_cmpnle_ps */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        
        /* UNLE - _mm_cmpnge_ps (not greater or equal) */
        __m128 mask_ule = _mm_cmpnge_ps(va, vb);
        
        /* UNLT - _mm_cmpngt_ps (not greater than) */
        __m128 mask_ult = _mm_cmpngt_ps(va, vb);
        
        /* LTGT (une) - _mm_cmpneq_ps (not equal) */
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
    int r;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(r) : : "cc");
    result += r;
    
    /* ORDERED - "no" flag (not overflow) but we need ordered FP */
    /* Use fcomi instruction with appropriate condition */
    asm volatile (
        "fcomi %%st(1), %%st\n\t"
        "setnp %0"
        : "=r"(r) : : "cc"
    );
    result += r;
    
    /* UNEQ - "e" flag for equal, but with unordered */
    asm volatile ("sete %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNGE - "ge" flag (greater or equal) */
    asm volatile ("setge %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNGT - "g" flag (greater) */
    asm volatile ("setg %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNLE - "le" flag (less or equal) */
    asm volatile ("setle %0" : "=r"(r) : : "cc");
    result += r;
    
    /* UNLT - "l" flag (less) */
    asm volatile ("setl %0" : "=r"(r) : : "cc");
    result += r;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(r) : : "cc");
    result += r;
    
    return result;
}

__attribute__((noinline))
int test_avx_comparisons(float *a, float *b, int n) {
    if (n < 8) return 0;
    
    __m256 sum = _mm256_setzero_ps();
    
    for (int i = 0; i < n; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        
        /* AVX versions of the comparisons */
        __m256 mask_unord = _mm256_cmp_ps(va, vb, _CMP_UNORD_Q);
        __m256 mask_ord = _mm256_cmp_ps(va, vb, _CMP_ORD_Q);
        __m256 mask_nlt = _mm256_cmp_ps(va, vb, _CMP_NLT_UQ);  /* UNGE */
        __m256 mask_nle = _mm256_cmp_ps(va, vb, _CMP_NLE_UQ);  /* UNGT */
        __m256 mask_ule = _mm256_cmp_ps(va, vb, _CMP_LE_OQ);   /* Ordered less or equal */
        __m256 mask_ult = _mm256_cmp_ps(va, vb, _CMP_LT_OQ);   /* Ordered less than */
        __m256 mask_une = _mm256_cmp_ps(va, vb, _CMP_NEQ_UQ);  /* LTGT */
        
        /* Combine */
        __m256 combined = _mm256_add_ps(mask_unord, mask_ord);
        combined = _mm256_add_ps(combined, mask_nlt);
        combined = _mm256_add_ps(combined, mask_nle);
        combined = _mm256_add_ps(combined, mask_ule);
        combined = _mm256_add_ps(combined, mask_ult);
        combined = _mm256_add_ps(combined, mask_une);
        
        sum = _mm256_add_ps(sum, combined);
    }
    
    float result[8];
    _mm256_storeu_ps(result, sum);
    return (int)(result[0] + result[1] + result[2] + result[3] +
                 result[4] + result[5] + result[6] + result[7]);
}

int main() {
    /* Create arrays with NaN values to trigger unordered comparisons */
    float fa[32], fb[32];
    double da[16], db[16];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < 32; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i * 1.5f;
        fb[i] = (i % 5 == 0) ? NAN : (float)i * 0.7f;
    }
    
    for (int i = 0; i < 16; i++) {
        da[i] = (i % 4 == 0) ? NAN : (double)i * 2.3;
        db[i] = (i % 6 == 0) ? NAN : (double)i * 1.1;
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    checksum += test_scalar_builtins(fa[1], fb[1], da[1], db[1]);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, 32);
    
    /* Test inline assembly */
    checksum += test_inline_asm(fa[2], fb[2]);
    
    /* Test AVX comparisons if available */
    checksum += test_avx_comparisons(fa, fb, 32);
    
    /* Additional tests with different values */
    for (int i = 0; i < 8; i++) {
        checksum += test_scalar_builtins(
            fa[i*2], fb[i*2+1], 
            da[i], db[(i+1)%8]
        );
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
