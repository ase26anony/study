/* Test program to cover condition code output in i386.cc */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
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
    if (!(a < b)) {
        result |= 8;
    }
    
    /* UNGT case - not less or equal (greater or unordered) */
    if (!(a <= b)) {
        result |= 16;
    }
    
    /* UNLE case - less or equal or unordered */
    if (a <= b || __builtin_isunordered(a, b)) {
        result |= 32;
    }
    
    /* UNLT case - less than or unordered */
    if (a < b || __builtin_isunordered(a, b)) {
        result |= 64;
    }
    
    /* LTGT case - less than or greater than (ordered and not equal) */
    if ((a < b) || (a > b)) {
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
        
        /* UNORDERED - _mm_cmpunord_ps */
        __m128 mask_unord = _mm_cmpunord_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(mask_unord, va));
        
        /* ORDERED - _mm_cmpord_ps */
        __m128 mask_ord = _mm_cmpord_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(mask_ord, vb));
        
        /* UNGE - _mm_cmpnlt_ps (not less than) */
        __m128 mask_nlt = _mm_cmpnlt_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(mask_nlt, _mm_set1_ps(1.0f)));
        
        /* UNGT - _mm_cmpnle_ps (not less or equal) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(mask_nle, _mm_set1_ps(2.0f)));
        
        /* UNLE - _mm_cmple_ps with unordered handling */
        __m128 mask_le = _mm_cmple_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(mask_le, _mm_set1_ps(3.0f)));
        
        /* UNLT - _mm_cmplt_ps with unordered handling */
        __m128 mask_lt = _mm_cmplt_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(mask_lt, _mm_set1_ps(4.0f)));
        
        /* LTGT - _mm_cmpneq_ps (not equal, ordered) */
        __m128 mask_neq = _mm_cmpneq_ps(va, vb);
        sum = _mm_add_ps(sum, _mm_and_ps(mask_neq, _mm_set1_ps(5.0f)));
    }
    
    /* Extract result */
    float temp[4];
    _mm_storeu_ps(temp, sum);
    return (int)(temp[0] + temp[1] + temp[2] + temp[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int tmp;
    
    /* Inline assembly with various condition codes */
    
    /* UNORDERED - 'u' flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setu %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp;
    
    /* ORDERED - 'nu' flag (not unordered) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setnu %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp * 2;
    
    /* UNEQ - unordered or equal */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0\n\t"
        "jp 1f\n\t"
        "jmp 2f\n\t"
        "1:\n\t"
        "mov $1, %0\n\t"
        "2:"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp * 4;
    
    /* UNGE - not less than (greater or equal or unordered) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setnb %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp * 8;
    
    /* UNGT - not less or equal (greater or unordered) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setnbe %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp * 16;
    
    /* UNLE - less or equal or unordered */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp * 32;
    
    /* UNLT - less than or unordered */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp * 64;
    
    /* LTGT - less than or greater than (ordered and not equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setne %0\n\t"
        "jp 1f\n\t"
        "jmp 2f\n\t"
        "1:\n\t"
        "mov $0, %0\n\t"
        "2:"
        : "=r"(tmp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += tmp * 128;
    
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(double x, double y) {
    int result = 0;
    
    /* Force generation of various condition codes through control flow */
    if (isnan(x) || isnan(y)) {
        result = 1;  /* UNORDERED path */
    } else if (x > y) {
        result = 2;  /* UNGT/GT path */
    } else if (x < y) {
        result = 3;  /* UNLT/LT path */
    } else if (x == y) {
        result = 4;  /* EQ/UNEQ path */
    }
    
    /* Use fpclassify to generate condition codes */
    int cx = fpclassify(x);
    int cy = fpclassify(y);
    
    if (cx == FP_NAN || cy == FP_NAN) {
        result |= 0x10;  /* UNORDERED */
    }
    if (cx == FP_INFINITE && cy == FP_INFINITE) {
        result |= 0x20;  /* EQ/UNEQ */
    }
    
    return result;
}

int main() {
    float f1 = 1.0f;
    float f2 = 2.0f;
    float f_nan = NAN;
    float f_inf = INFINITY;
    
    /* Test scalar builtins with various inputs */
    int sum = 0;
    
    /* Normal numbers */
    sum += test_scalar_builtins(f1, f2);
    
    /* With NaN */
    sum += test_scalar_builtins(f1, f_nan);
    sum += test_scalar_builtins(f_nan, f2);
    sum += test_scalar_builtins(f_nan, f_nan);
    
    /* With infinity */
    sum += test_scalar_builtins(f_inf, f1);
    sum += test_scalar_builtins(f1, f_inf);
    sum += test_scalar_builtins(f_inf, f_inf);
    
    /* Test SSE intrinsics */
    const int ARRAY_SIZE = 64;
    float a[ARRAY_SIZE];
    float b[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(i * 2) * 0.1f;
        /* Insert some NaN values */
        if (i % 7 == 0) a[i] = f_nan;
        if (i % 11 == 0) b[i] = f_nan;
    }
    
    sum += test_sse_intrinsics(a, b, ARRAY_SIZE);
    
    /* Test inline assembly */
    sum += test_inline_asm(f1, f2);
    sum += test_inline_asm(f1, f_nan);
    sum += test_inline_asm(f_nan, f2);
    
    /* Test mixed comparisons */
    sum += test_mixed_comparisons(1.0, 2.0);
    sum += test_mixed_comparisons(NAN, 2.0);
    sum += test_mixed_comparisons(1.0, NAN);
    sum += test_mixed_comparisons(INFINITY, 1.0);
    
    /* Print result to prevent optimization */
    printf("Result checksum: %d\n", sum);
    
    return 0;
}
