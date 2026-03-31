/* test_condition_codes.c - Cover GCC i386 condition code output routines */
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
    
    /* UNORDERED case - __builtin_isunordered */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case - opposite of unordered */
    if (!__builtin_isunordered(a, b)) result |= 2;
    
    /* UNEQ case - unordered or equal */
    if (__builtin_isunordered(a, b) || a == b) result |= 4;
    
    /* UNGE case - not less than (nlt) */
    if (!(a < b)) result |= 8;
    
    /* UNGT case - not less than or equal (nle) */
    if (!(a <= b)) result |= 16;
    
    /* UNLE case - unordered or less than or equal (ule) */
    if (__builtin_isunordered(a, b) || a <= b) result |= 32;
    
    /* UNLT case - unordered or less than (ult) */
    if (__builtin_isunordered(a, b) || a < b) result |= 64;
    
    /* LTGT case - less than or greater than (une) */
    if (a != b) result |= 128;
    
    /* Additional builtins that may generate condition codes */
    if (__builtin_isgreater(c, d)) result |= 256;
    if (__builtin_isless(c, d)) result |= 512;
    if (__builtin_isgreaterequal(c, d)) result |= 1024;
    if (__builtin_islessequal(c, d)) result |= 2048;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *a, float *b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _mm_cmpunord_ps */
        __m128 cmp_unord = _mm_cmpunord_ps(va, vb);
        
        /* ORDERED - _mm_cmpord_ps */
        __m128 cmp_ord = _mm_cmpord_ps(va, vb);
        
        /* UNGE - _mm_cmpnlt_ps (not less than) */
        __m128 cmp_nlt = _mm_cmpnlt_ps(va, vb);
        
        /* UNGT - _mm_cmpnle_ps (not less than or equal) */
        __m128 cmp_nle = _mm_cmpnle_ps(va, vb);
        
        /* UNLE - _mm_cmple_ps with unordered handling */
        __m128 cmp_ule = _mm_cmple_ps(va, vb);
        
        /* UNLT - _mm_cmplt_ps with unordered handling */
        __m128 cmp_ult = _mm_cmplt_ps(va, vb);
        
        /* LTGT - _mm_cmpneq_ps (not equal) */
        __m128 cmp_une = _mm_cmpneq_ps(va, vb);
        
        /* Combine results */
        __m128 combined = _mm_add_ps(cmp_unord, cmp_ord);
        combined = _mm_add_ps(combined, cmp_nlt);
        combined = _mm_add_ps(combined, cmp_nle);
        combined = _mm_add_ps(combined, cmp_ule);
        combined = _mm_add_ps(combined, cmp_ult);
        combined = _mm_add_ps(combined, cmp_une);
        
        sum = _mm_add_ps(sum, combined);
    }
    
    /* Extract result */
    float result[4];
    _mm_storeu_ps(result, sum);
    return (int)(result[0] + result[1] + result[2] + result[3]);
}

__attribute__((noinline))
int test_avx_intrinsics(double *a, double *b, int n) {
#ifdef __AVX__
    __m256d sum = _mm256_setzero_pd();
    
    for (int i = 0; i < n; i += 4) {
        __m256d va = _mm256_loadu_pd(&a[i]);
        __m256d vb = _mm256_loadu_pd(&b[i]);
        
        /* Various AVX comparison intrinsics */
        __m256d cmp_unord = _mm256_cmp_pd(va, vb, _CMP_UNORD_Q);
        __m256d cmp_ord = _mm256_cmp_pd(va, vb, _CMP_ORD_Q);
        __m256d cmp_nlt = _mm256_cmp_pd(va, vb, _CMP_NLT_UQ);
        __m256d cmp_nle = _mm256_cmp_pd(va, vb, _CMP_NLE_UQ);
        __m256d cmp_ule = _mm256_cmp_pd(va, vb, _CMP_LE_OQ);
        __m256d cmp_ult = _mm256_cmp_pd(va, vb, _CMP_LT_OQ);
        __m256d cmp_une = _mm256_cmp_pd(va, vb, _CMP_NEQ_UQ);
        
        /* Combine */
        __m256d combined = _mm256_add_pd(cmp_unord, cmp_ord);
        combined = _mm256_add_pd(combined, cmp_nlt);
        combined = _mm256_add_pd(combined, cmp_nle);
        combined = _mm256_add_pd(combined, cmp_ule);
        combined = _mm256_add_pd(combined, cmp_ult);
        combined = _mm256_add_pd(combined, cmp_une);
        
        sum = _mm256_add_pd(sum, combined);
    }
    
    double result[4];
    _mm256_storeu_pd(result, sum);
    return (int)(result[0] + result[1] + result[2] + result[3]);
#else
    return 0;
#endif
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int tmp;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED - "u" flag */
    asm volatile (
        "setu %0"
        : "=r"(tmp)
        :
        : "cc"
    );
    result |= (tmp & 1) << 0;
    
    /* ORDERED - "no" flag (not unordered) */
    asm volatile (
        "setno %0"
        : "=r"(tmp)
        :
        : "cc"
    );
    result |= (tmp & 1) << 1;
    
    /* UNEQ - unordered or equal */
    asm volatile (
        "sete %0"
        : "=r"(tmp)
        :
        : "cc"
    );
    result |= (tmp & 1) << 2;
    
    /* UNGE - not less than */
    asm volatile (
        "setnl %0"
        : "=r"(tmp)
        :
        : "cc"
    );
    result |= (tmp & 1) << 3;
    
    /* UNGT - not less than or equal */
    asm volatile (
        "setnle %0"
        : "=r"(tmp)
        :
        : "cc"
    );
    result |= (tmp & 1) << 4;
    
    /* UNLE - unordered or less than or equal */
    asm volatile (
        "setle %0"
        : "=r"(tmp)
        :
        : "cc"
    );
    result |= (tmp & 1) << 5;
    
    /* UNLT - unordered or less than */
    asm volatile (
        "setl %0"
        : "=r"(tmp)
        :
        : "cc"
    );
    result |= (tmp & 1) << 6;
    
    /* LTGT - less than or greater than */
    asm volatile (
        "setne %0"
        : "=r"(tmp)
        :
        : "cc"
    );
    result |= (tmp & 1) << 7;
    
    return result;
}

__attribute__((noinline))
int test_fpclassify(float a, double b) {
    int result = 0;
    
    /* fpclassify may generate condition codes */
    int fa = fpclassify(a);
    int fb = fpclassify(b);
    
    if (fa == FP_NAN) result |= 1;
    if (fa == FP_INFINITE) result |= 2;
    if (fa == FP_ZERO) result |= 4;
    if (fa == FP_SUBNORMAL) result |= 8;
    if (fa == FP_NORMAL) result |= 16;
    
    if (fb == FP_NAN) result |= 32;
    if (fb == FP_INFINITE) result |= 64;
    if (fb == FP_ZERO) result |= 128;
    if (fb == FP_SUBNORMAL) result |= 256;
    if (fb == FP_NORMAL) result |= 512;
    
    /* isnan/isinf may also generate condition codes */
    if (isnan(a)) result |= 1024;
    if (isinf(b)) result |= 2048;
    
    return result;
}

int main() {
    /* Initialize with NaN values to trigger unordered comparisons */
    float fa[] = {1.0f, NAN, 3.0f, -NAN, 5.0f, INFINITY, -INFINITY, 0.0f};
    float fb[] = {NAN, 2.0f, -NAN, 4.0f, INFINITY, 5.0f, 0.0f, -INFINITY};
    
    double da[] = {1.0, NAN, 3.0, -NAN, 5.0, INFINITY, -INFINITY, 0.0};
    double db[] = {NAN, 2.0, -NAN, 4.0, INFINITY, 5.0, 0.0, -INFINITY};
    
    int checksum = 0;
    
    /* Test scalar builtins with various inputs */
    for (int i = 0; i < 8; i++) {
        checksum += test_scalar_builtins(fa[i], fb[i], da[i], db[i]);
    }
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, 8);
    
    /* Test AVX intrinsics if available */
    checksum += test_avx_intrinsics(da, db, 8);
    
    /* Test inline assembly */
    for (int i = 0; i < 8; i++) {
        checksum += test_inline_asm(fa[i], fb[i]);
    }
    
    /* Test fpclassify/isnan/isinf */
    for (int i = 0; i < 8; i++) {
        checksum += test_fpclassify(fa[i], da[i]);
    }
    
    /* Additional mixed tests */
    volatile float v1 = NAN;
    volatile float v2 = 1.0f;
    
    /* Force generation of condition codes in loops */
    for (int i = 0; i < 100; i++) {
        if (__builtin_isunordered(v1, v2)) checksum++;
        if (!__builtin_isunordered(v1, v2)) checksum--;
        if (__builtin_isgreater(v1, v2)) checksum += 2;
        if (__builtin_isless(v1, v2)) checksum -= 2;
        
        /* Mix with regular comparisons */
        if (v1 < v2) checksum += 3;
        if (v1 > v2) checksum -= 3;
        if (v1 == v2) checksum += 4;
        if (v1 != v2) checksum -= 4;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
