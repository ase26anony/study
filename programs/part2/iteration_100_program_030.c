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
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* ORDERED case */
    if (__builtin_isordered(c, d)) {
        result |= 2;
    }
    
    /* UNEQ case (unordered or equal) - use with fast-math */
    if (!(c > d) && !(c < d)) {  /* May become UNEQ with -ffast-math */
        result |= 4;
    }
    
    /* UNGE case (not less than) */
    if (__builtin_isgreaterequal(a, b)) {
        result |= 8;
    }
    
    /* UNGT case (not less than or equal) */
    if (__builtin_isgreater(c, d)) {
        result |= 16;
    }
    
    /* UNLE case (unordered or less than or equal) */
    if (__builtin_islessequal(a, b)) {
        result |= 32;
    }
    
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(c, d)) {
        result |= 64;
    }
    
    /* LTGT case (less than or greater than, ordered) */
    if (__builtin_islessgreater(a, b)) {
        result |= 128;
    }
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float* arr1, float* arr2, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 a = _mm_loadu_ps(&arr1[i]);
        __m128 b = _mm_loadu_ps(&arr2[i]);
        
        /* UNORDERED: _CMP_UNORD_Q */
        __m128 cmp_unord = _mm_cmpunord_ps(a, b);
        
        /* ORDERED: _CMP_ORD_Q */
        __m128 cmp_ord = _mm_cmpord_ps(a, b);
        
        /* UNGE: _CMP_NLT_UQ (not less than, unordered quiet) */
        __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);
        
        /* UNGT: _CMP_NLE_UQ (not less than or equal, unordered quiet) */
        __m128 cmp_nle = _mm_cmpnle_ps(a, b);
        
        /* UNLE: _CMP_LE_UQ (less than or equal, unordered quiet) */
        __m128 cmp_ule = _mm_cmple_ps(a, b);
        
        /* UNLT: _CMP_LT_UQ (less than, unordered quiet) */
        __m128 cmp_ult = _mm_cmplt_ps(a, b);
        
        /* LTGT: _CMP_NEQ_UQ (not equal, unordered quiet) */
        __m128 cmp_une = _mm_cmpneq_ps(a, b);
        
        /* UNEQ: _CMP_EQ_UQ (equal, unordered quiet) */
        __m128 cmp_ueq = _mm_cmpeq_ps(a, b);
        
        /* Combine results */
        __m128 combined = _mm_add_ps(cmp_unord, cmp_ord);
        combined = _mm_add_ps(combined, cmp_nlt);
        combined = _mm_add_ps(combined, cmp_nle);
        combined = _mm_add_ps(combined, cmp_ule);
        combined = _mm_add_ps(combined, cmp_ult);
        combined = _mm_add_ps(combined, cmp_une);
        combined = _mm_add_ps(combined, cmp_ueq);
        
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
    
    /* ORDERED: "no" flag (not overflow, for ordered) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setno %0"
        : "=r"(r2)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* UNEQ: "e" flag (equal) for unordered equal */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(r3)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* UNGE: "ae" flag (above or equal) for not less than */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r"(r4)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* UNGT: "a" flag (above) for not less than or equal */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(r5)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* UNLE: "be" flag (below or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(r6)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* UNLT: "b" flag (below) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(r7)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* LTGT: "ne" flag (not equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(r8)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(double* darr1, double* darr2, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        double a = darr1[i];
        double b = darr2[i];
        
        /* Mix of comparisons that may generate different condition codes */
        sum += __builtin_isunordered(a, b) ? 1 : 0;
        sum += __builtin_isordered(a, b) ? 2 : 0;
        sum += (a >= b) ? 4 : 0;  /* May become UNGE */
        sum += (a > b) ? 8 : 0;   /* May become UNGT */
        sum += (a <= b) ? 16 : 0; /* May become UNLE */
        sum += (a < b) ? 32 : 0;  /* May become UNLT */
        sum += (a != b) ? 64 : 0; /* May become LTGT */
        sum += (a == b) ? 128 : 0; /* May become UNEQ */
    }
    
    return sum;
}

int main() {
    const int N = 256;
    float* farr1 = (float*)aligned_alloc(16, N * sizeof(float));
    float* farr2 = (float*)aligned_alloc(16, N * sizeof(float));
    double* darr1 = (double*)aligned_alloc(16, N * sizeof(double));
    double* darr2 = (double*)aligned_alloc(16, N * sizeof(double));
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < N; i++) {
        farr1[i] = (i % 10 == 0) ? NAN : (float)(i * 0.1);
        farr2[i] = (i % 7 == 0) ? NAN : (float)(i * 0.2 + 0.5);
        darr1[i] = (i % 11 == 0) ? NAN : (double)(i * 0.3);
        darr2[i] = (i % 13 == 0) ? NAN : (double)(i * 0.4 + 1.0);
    }
    
    /* Test all functions */
    int checksum = 0;
    
    checksum += test_scalar_builtins(farr1[0], farr2[0], darr1[0], darr2[0]);
    checksum += test_sse_intrinsics(farr1, farr2, N);
    checksum += test_inline_asm(farr1[1], farr2[1]);
    checksum += test_mixed_comparisons(darr1, darr2, N);
    
    /* Additional tests with specific values to trigger different paths */
    float special_values[] = {NAN, INFINITY, -INFINITY, 0.0f, 1.0f, -1.0f};
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            checksum += test_scalar_builtins(special_values[i], special_values[j],
                                            (double)special_values[i], 
                                            (double)special_values[j]);
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(farr1);
    free(farr2);
    free(darr1);
    free(darr2);
    
    return 0;
}
