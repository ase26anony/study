/* test_condition_codes.c - Cover GCC x86 condition code output routines */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining to ensure code generation */
__attribute__((noinline)) 
int test_scalar_builtins(float a, float b, double c, double d) {
    int result = 0;
    
    /* UNORDERED case - __builtin_isunordered */
    if (__builtin_isunordered(a, b)) result |= 1;
    
    /* ORDERED case - __builtin_isordered */
    if (__builtin_isordered(c, d)) result |= 2;
    
    /* UNEQ case - unordered or equal */
    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) result |= 4;
    
    /* UNGE case - not less than (nlt) */
    if (!__builtin_isless(a, b)) result |= 8;
    
    /* UNGT case - not less than or equal (nle) */
    if (!__builtin_islessequal(a, b)) result |= 16;
    
    /* UNLE case - unordered or less than or equal (ule) */
    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) result |= 32;
    
    /* UNLT case - unordered or less than (ult) */
    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) result |= 64;
    
    /* LTGT case - less than or greater than (une) */
    if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) result |= 128;
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(const float* a, const float* b, int n) {
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
        
        /* UNLE - _mm_cmple_ps (less than or equal) with unordered handling */
        __m128 mask_ule = _mm_cmple_ps(va, vb);
        
        /* UNLT - _mm_cmplt_ps (less than) with unordered handling */
        __m128 mask_ult = _mm_cmplt_ps(va, vb);
        
        /* LTGT - _mm_cmpneq_ps (not equal) */
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
    asm volatile ("setu %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* ORDERED - "no" flag (not overflow, but used for ordered) */
    asm volatile ("setno %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNEQ - "e" flag (equal) with unordered context */
    asm volatile ("sete %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNGE - "nl" flag (not less) */
    asm volatile ("setnl %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNGT - "nle" flag (not less or equal) */
    asm volatile ("setnle %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNLE - "le" flag (less or equal) */
    asm volatile ("setle %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* UNLT - "l" flag (less than) */
    asm volatile ("setl %0" : "=r"(temp) : : "cc");
    result += temp;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile ("setne %0" : "=r"(temp) : : "cc");
    result += temp;
    
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(double* arr, int n) {
    int count = 0;
    
    for (int i = 0; i < n - 1; i++) {
        double a = arr[i];
        double b = arr[i + 1];
        
        /* Mix of comparisons that should generate different condition codes */
        if (isnan(a) || isnan(b)) {
            count++;  /* UNORDERED path */
        } else if (a > b) {
            count += 2;  /* GT path (may become UNGT with fast-math) */
        } else if (a < b) {
            count += 3;  /* LT path (may become UNLT with fast-math) */
        } else if (a == b) {
            count += 4;  /* EQ path (may become UNEQ with fast-math) */
        }
        
        /* Complex condition that might generate LTGT */
        if ((a < b) != (a > b)) {
            count += 5;
        }
    }
    
    return count;
}

/* Force array initialization to prevent constant propagation */
__attribute__((noinline))
void init_arrays(float* a, float* b, double* c, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix of normal numbers and NaN */
        if (i % 7 == 0) {
            a[i] = NAN;
            b[i] = (float)i;
        } else if (i % 5 == 0) {
            a[i] = (float)i;
            b[i] = NAN;
        } else {
            a[i] = (float)(i * 1.1);
            b[i] = (float)(i * 0.9);
        }
        
        if (i < n/2) {
            c[i] = (double)i * 2.5;
        } else {
            c[i] = NAN;
        }
    }
}

int main() {
    const int N = 256;
    float a[N], b[N];
    double c[N];
    
    /* Initialize with mixed values including NaN */
    init_arrays(a, b, c, N);
    
    int checksum = 0;
    
    /* Test scalar builtins with various inputs */
    checksum += test_scalar_builtins(a[0], b[0], c[0], c[1]);
    checksum += test_scalar_builtins(NAN, 1.0f, 2.0, NAN);
    checksum += test_scalar_builtins(1.0f, NAN, NAN, NAN);
    checksum += test_scalar_builtins(3.0f, 2.0f, 1.0, 2.0);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(a, b, N);
    
    /* Test inline assembly */
    checksum += test_inline_asm(a[1], b[1]);
    checksum += test_inline_asm(NAN, 1.0f);
    
    /* Test mixed comparisons */
    checksum += test_mixed_comparisons(c, N);
    
    /* Use checksum to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    return 0;
}
