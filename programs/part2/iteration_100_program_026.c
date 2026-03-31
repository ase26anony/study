/* Test program to cover floating-point condition code output in i386.cc */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b) {
    int result = 0;
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* ORDERED case */
    if (!__builtin_isunordered(a, b)) {
        result |= 2;
    }
    
    /* UNEQ case - unordered or equal */
    if (__builtin_isunordered(a, b) || a == b) {
        result |= 4;
    }
    
    /* UNGE case - not less than (greater or equal or unordered) */
    if (!__builtin_isless(a, b)) {
        result |= 8;
    }
    
    /* UNGT case - not less or equal (greater or unordered) */
    if (!__builtin_islessequal(a, b)) {
        result |= 16;
    }
    
    /* UNLE case - less or equal or unordered */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) {
        result |= 32;
    }
    
    /* UNLT case - less than or unordered */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
        result |= 64;
    }
    
    /* LTGT case - less than or greater than (ordered and not equal) */
    if ((a < b) || (a > b)) {
        result |= 128;
    }
    
    return result;
}

__attribute__((noinline))
int test_vector_intrinsics(float *a, float *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _CMP_UNORD_Q */
        __m128 mask_unord = _mm_cmpunord_ps(va, vb);
        int unord_mask = _mm_movemask_ps(mask_unord);
        sum += unord_mask;
        
        /* ORDERED - _CMP_ORD_Q */
        __m128 mask_ord = _mm_cmpord_ps(va, vb);
        int ord_mask = _mm_movemask_ps(mask_ord);
        sum += ord_mask;
        
        /* UNGE - _CMP_NLT_UQ (not less than, unordered quiet) */
        __m128 mask_nlt = _mm_cmpnlt_ps(va, vb);
        int nlt_mask = _mm_movemask_ps(mask_nlt);
        sum += nlt_mask;
        
        /* UNGT - _CMP_NLE_UQ (not less or equal, unordered quiet) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        int nle_mask = _mm_movemask_ps(mask_nle);
        sum += nle_mask;
        
        /* UNLE - _CMP_LE_UQ (less or equal, unordered quiet) */
        __m128 mask_ule = _mm_cmple_ps(va, vb);
        int ule_mask = _mm_movemask_ps(mask_ule);
        sum += ule_mask;
        
        /* UNLT - _CMP_LT_UQ (less than, unordered quiet) */
        __m128 mask_ult = _mm_cmplt_ps(va, vb);
        int ult_mask = _mm_movemask_ps(mask_ult);
        sum += ult_mask;
        
        /* UNEQ - _CMP_EQ_UQ (equal, unordered quiet) */
        __m128 mask_ueq = _mm_cmpeq_ps(va, vb);
        int ueq_mask = _mm_movemask_ps(mask_ueq);
        sum += ueq_mask;
        
        /* LTGT - _CMP_NEQ_UQ (not equal, unordered quiet) */
        __m128 mask_une = _mm_cmpneq_ps(va, vb);
        int une_mask = _mm_movemask_ps(mask_une);
        sum += une_mask;
    }
    
    return sum;
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int tmp;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* ORDERED - "no" flag (not overflow, used for ordered) */
    asm volatile ("setno %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNEQ - "e" flag (equal) - unordered equal */
    asm volatile ("sete %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNGE - "ge" flag (greater or equal) */
    asm volatile ("setge %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNGT - "g" flag (greater) */
    asm volatile ("setg %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNLE - "le" flag (less or equal) */
    asm volatile ("setle %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNLT - "l" flag (less) */
    asm volatile ("setl %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(double x, double y) {
    int result = 0;
    
    /* Test various comparison patterns that might generate different condition codes */
    
    /* Ordered comparisons */
    result += (x < y) ? 1 : 0;      /* LT */
    result += (x <= y) ? 2 : 0;     /* LE */
    result += (x > y) ? 4 : 0;      /* GT */
    result += (x >= y) ? 8 : 0;     /* GE */
    result += (x == y) ? 16 : 0;    /* EQ */
    result += (x != y) ? 32 : 0;    /* NE */
    
    /* Unordered-aware comparisons */
    result += (!(x < y)) ? 64 : 0;   /* UNGE */
    result += (!(x <= y)) ? 128 : 0; /* UNGT */
    result += (!(x > y)) ? 256 : 0;  /* UNLE */
    result += (!(x >= y)) ? 512 : 0; /* UNLT */
    
    /* Special cases with NaN */
    if (isnan(x) || isnan(y)) {
        result |= 1024;
    }
    
    /* fpclassify usage */
    int cx = fpclassify(x);
    int cy = fpclassify(y);
    if (cx == FP_NAN || cy == FP_NAN) {
        result |= 2048;
    }
    
    return result;
}

/* Initialize arrays with mix of normal values and NaN */
void init_arrays(float *a, float *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (float)(i * 1.1);
        b[i] = (float)(i * 0.9);
        
        /* Insert NaN at specific positions */
        if (i % 7 == 0) {
            a[i] = NAN;
        }
        if (i % 11 == 0) {
            b[i] = NAN;
        }
    }
}

int main() {
    const int ARRAY_SIZE = 128;
    float a[ARRAY_SIZE];
    float b[ARRAY_SIZE];
    
    /* Initialize with mixed data including NaN */
    init_arrays(a, b, ARRAY_SIZE);
    
    int checksum = 0;
    
    /* Test scalar builtins with various inputs */
    checksum += test_scalar_builtins(1.0f, 2.0f);
    checksum += test_scalar_builtins(NAN, 2.0f);
    checksum += test_scalar_builtins(1.0f, NAN);
    checksum += test_scalar_builtins(NAN, NAN);
    checksum += test_scalar_builtins(2.0f, 2.0f);
    
    /* Test vector intrinsics */
    checksum += test_vector_intrinsics(a, b, ARRAY_SIZE);
    
    /* Test inline assembly */
    checksum += test_inline_asm(1.0f, 2.0f);
    checksum += test_inline_asm(NAN, 2.0f);
    
    /* Test mixed comparisons */
    checksum += test_mixed_comparisons(1.0, 2.0);
    checksum += test_mixed_comparisons(NAN, 2.0);
    checksum += test_mixed_comparisons(1.0, NAN);
    
    /* Additional tests with volatile to prevent optimization */
    volatile float v1 = 1.0f;
    volatile float v2 = 2.0f;
    volatile float vnan = NAN;
    
    checksum += (__builtin_isunordered(v1, v2) ? 1 : 0);
    checksum += (__builtin_isunordered(v1, vnan) ? 2 : 0);
    checksum += (__builtin_isgreater(v1, v2) ? 4 : 0);
    checksum += (__builtin_isless(v1, v2) ? 8 : 0);
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
