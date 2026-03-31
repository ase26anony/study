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
int test_scalar_builtins(float a, float b) {
    int result = 0;
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* ORDERED case */
    if (!__builtin_isunordered(a, b)) {  /* Ordered is opposite of unordered */
        result |= 2;
    }
    
    /* UNEQ case - unordered or equal */
    if (__builtin_isunordered(a, b) || a == b) {
        result |= 4;
    }
    
    /* UNGE case - not less than (greater or equal or unordered) */
    if (!(a < b)) {  /* With -ffast-math, this becomes UNGE */
        result |= 8;
    }
    
    /* UNGT case - not less than or equal (greater or unordered) */
    if (!(a <= b)) {  /* With -ffast-math, this becomes UNGT */
        result |= 16;
    }
    
    /* UNLE case - less than or equal or unordered */
    if (a <= b || __builtin_isunordered(a, b)) {
        result |= 32;
    }
    
    /* UNLT case - less than or unordered */
    if (a < b || __builtin_isunordered(a, b)) {
        result |= 64;
    }
    
    /* LTGT case - less than or greater than (ordered and not equal) */
    if ((a < b) || (a > b)) {  /* Ordered comparison, excludes equal and unordered */
        result |= 128;
    }
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(float *a, float *b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _CMP_UNORD_Q */
        __m128 mask_unord = _mm_cmpunord_ps(va, vb);
        sum = _mm_add_ps(sum, mask_unord);
        
        /* ORDERED - _CMP_ORD_Q */
        __m128 mask_ord = _mm_cmpord_ps(va, vb);
        sum = _mm_add_ps(sum, mask_ord);
        
        /* UNGE - _CMP_NLT_UQ (not less than, unordered quiet) */
        __m128 mask_nlt = _mm_cmpnlt_ps(va, vb);
        sum = _mm_add_ps(sum, mask_nlt);
        
        /* UNGT - _CMP_NLE_UQ (not less than or equal, unordered quiet) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        sum = _mm_add_ps(sum, mask_nle);
        
        /* UNLE - _CMP_LE_UQ (less than or equal, unordered quiet) */
        __m128 mask_ule = _mm_cmple_ps(va, vb);
        sum = _mm_add_ps(sum, mask_ule);
        
        /* UNLT - _CMP_LT_UQ (less than, unordered quiet) */
        __m128 mask_ult = _mm_cmplt_ps(va, vb);
        sum = _mm_add_ps(sum, mask_ult);
        
        /* LTGT - _CMP_NEQ_UQ (not equal, unordered quiet) */
        __m128 mask_une = _mm_cmpneq_ps(va, vb);
        sum = _mm_add_ps(sum, mask_une);
        
        /* UNEQ - _CMP_EQ_UQ (equal, unordered quiet) */
        __m128 mask_ueq = _mm_cmpeq_ps(va, vb);
        sum = _mm_add_ps(sum, mask_ueq);
    }
    
    /* Extract some result to prevent optimization */
    float result[4];
    _mm_storeu_ps(result, sum);
    return (int)(result[0] + result[1] + result[2] + result[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int tmp;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* ORDERED - "no" flag (not overflow, but we need ordered) */
    /* Using "np" (parity) for ordered check */
    asm volatile ("setnp %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNEQ - "e" flag (equal) with unordered semantics */
    /* We'll use a floating-point comparison sequence */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(tmp) : "x"(a), "x"(b) : "cc"
    );
    result += tmp;
    
    /* UNGE - "ae" flag (above or equal) */
    asm volatile ("setae %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNGT - "a" flag (above) */
    asm volatile ("seta %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNLE - "be" flag (below or equal) */
    asm volatile ("setbe %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* UNLT - "b" flag (below) */
    asm volatile ("setb %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    return result;
}

__attribute__((noinline))
int test_mixed_conditions(double x, double y) {
    int result = 0;
    
    /* Test with fpclassify and isnan */
    if (isnan(x) || isnan(y)) {
        result |= 1;
    }
    
    /* Ordered comparison with NaN handling */
    if (!isnan(x) && !isnan(y)) {
        if (x > y) result |= 2;
        if (x < y) result |= 4;
        if (x == y) result |= 8;
    }
    
    /* Generate UNGE/UNGT through arithmetic */
    double diff = x - y;
    if (!(diff < 0.0)) {  /* UNGE */
        result |= 16;
    }
    
    if (!(diff <= 0.0)) { /* UNGT */
        result |= 32;
    }
    
    return result;
}

int main() {
    /* Create test data with NaN values to trigger unordered comparisons */
    float a[16], b[16];
    double da[8], db[8];
    
    /* Initialize with mix of normal values and NaN */
    for (int i = 0; i < 16; i++) {
        a[i] = (float)(i * 0.1);
        b[i] = (float)((i % 3) * 0.2);
        
        /* Insert some NaN values */
        if (i % 5 == 0) {
            a[i] = NAN;
        }
        if (i % 7 == 0) {
            b[i] = NAN;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        da[i] = i * 0.25;
        db[i] = (i % 4) * 0.5;
        if (i % 3 == 0) da[i] = NAN;
    }
    
    int checksum = 0;
    
    /* Test scalar builtins with various inputs */
    checksum += test_scalar_builtins(a[0], b[0]);
    checksum += test_scalar_builtins(1.0f, NAN);
    checksum += test_scalar_builtins(NAN, 2.0f);
    checksum += test_scalar_builtins(NAN, NAN);
    checksum += test_scalar_builtins(3.0f, 3.0f);
    checksum += test_scalar_builtins(5.0f, 2.0f);
    checksum += test_scalar_builtins(2.0f, 5.0f);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(a, b, 16);
    
    /* Test inline assembly */
    checksum += test_inline_asm(a[1], b[1]);
    checksum += test_inline_asm(NAN, b[2]);
    checksum += test_inline_asm(a[3], NAN);
    
    /* Test mixed conditions with doubles */
    checksum += test_mixed_conditions(da[0], db[0]);
    checksum += test_mixed_conditions(1.0, NAN);
    checksum += test_mixed_conditions(NAN, 2.0);
    
    /* Use the checksum to prevent optimization */
    printf("Result: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
