/* Test program to cover condition code output in i386.cc */
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
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* ORDERED case */
    if (__builtin_isordered(c, d)) {
        result |= 2;
    }
    
    /* UNEQ case - unordered or equal */
    if (!__builtin_isgreater(c, d) && !__builtin_isless(c, d)) {
        result |= 4;
    }
    
    /* UNGE case - not less than (greater or equal or unordered) */
    if (!__builtin_isless(a, b)) {
        result |= 8;
    }
    
    /* UNGT case - not less or equal (greater or unordered) */
    if (!__builtin_islessequal(c, d)) {
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
    
    /* LTGT case - less or greater (ordered and not equal) */
    if (__builtin_isless(c, d) || __builtin_isgreater(c, d)) {
        result |= 128;
    }
    
    return result;
}

__attribute__((noinline))
int test_sse_intrinsics(const float* a, const float* b, int n) {
    __m128 sum = _mm_setzero_ps();
    
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        
        /* UNORDERED - _CMP_UNORD_Q */
        __m128 cmp_unord = _mm_cmpunord_ps(va, vb);
        
        /* ORDERED - _CMP_ORD_Q */
        __m128 cmp_ord = _mm_cmpord_ps(va, vb);
        
        /* UNGE - _CMP_NLT_UQ (not less than, unordered quiet) */
        __m128 cmp_nlt = _mm_cmpnlt_ps(va, vb);
        
        /* UNGT - _CMP_NLE_UQ (not less or equal, unordered quiet) */
        __m128 cmp_nle = _mm_cmpnle_ps(va, vb);
        
        /* UNLE - _CMP_LE_UQ (less or equal, unordered quiet) */
        __m128 cmp_ule = _mm_cmple_ps(va, vb);
        
        /* UNLT - _CMP_LT_UQ (less than, unordered quiet) */
        __m128 cmp_ult = _mm_cmplt_ps(va, vb);
        
        /* UNEQ - _CMP_EQ_UQ (equal, unordered quiet) */
        __m128 cmp_ueq = _mm_cmpeq_ps(va, vb);
        
        /* LTGT - _CMP_NEQ_UQ (not equal, unordered quiet) */
        __m128 cmp_une = _mm_cmpneq_ps(va, vb);
        
        /* Mix results to prevent optimization */
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_unord, va));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_ord, vb));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_nlt, va));
        sum = _mm_add_ps(sum, _mm_and_ps(cmp_nle, vb));
    }
    
    /* Extract result */
    float r[4];
    _mm_storeu_ps(r, sum);
    return (int)(r[0] + r[1] + r[2] + r[3]);
}

__attribute__((noinline))
int test_inline_asm(float a, float b) {
    int result = 0;
    int tmp;
    
    /* UNORDERED - "u" flag */
    asm volatile ("setu %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* ORDERED - "no" flag (not overflow) - using different condition */
    asm volatile ("setno %0" : "=r"(tmp) : : "cc");
    result += tmp;
    
    /* Various floating point conditions */
    asm volatile (
        "comiss %1, %2\n\t"
        "seta %0"  /* above (greater, not equal, ordered) */
        : "=r"(tmp) : "x"(a), "x"(b) : "cc");
    result += tmp;
    
    asm volatile (
        "comiss %1, %2\n\t"
        "setb %0"  /* below (less than, ordered) */
        : "=r"(tmp) : "x"(a), "x"(b) : "cc");
    result += tmp;
    
    asm volatile (
        "comiss %1, %2\n\t"
        "setp %0"  /* parity (unordered) */
        : "=r"(tmp) : "x"(a), "x"(b) : "cc");
    result += tmp;
    
    return result;
}

__attribute__((noinline))
int test_mixed_comparisons(double* arr1, double* arr2, int n) {
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        /* Generate various condition codes through different comparisons */
        if (isnan(arr1[i]) || isnan(arr2[i])) {
            count++;  /* UNORDERED cases */
        }
        
        if (arr1[i] > arr2[i]) {
            count++;  /* GT (may become UNGT with fast-math) */
        }
        
        if (arr1[i] < arr2[i]) {
            count++;  /* LT (may become UNLT with fast-math) */
        }
        
        if (arr1[i] >= arr2[i]) {
            count++;  /* GE (may become UNGE with fast-math) */
        }
        
        if (arr1[i] <= arr2[i]) {
            count++;  /* LE (may become UNLE with fast-math) */
        }
        
        if (arr1[i] == arr2[i]) {
            count++;  /* EQ (may become UNEQ with fast-math) */
        }
        
        if (arr1[i] != arr2[i]) {
            count++;  /* NEQ (may become LTGT with fast-math) */
        }
    }
    
    return count;
}

int main() {
    /* Create test data with NaN values to trigger unordered comparisons */
    float fa[16], fb[16];
    double da[16], db[16];
    
    for (int i = 0; i < 16; i++) {
        fa[i] = (i % 3 == 0) ? NAN : (float)i;
        fb[i] = (i % 5 == 0) ? NAN : (float)(i * 2);
        da[i] = (i % 4 == 0) ? NAN : (double)i;
        db[i] = (i % 6 == 0) ? NAN : (double)(i * 3);
    }
    
    int checksum = 0;
    
    /* Test scalar builtins */
    checksum += test_scalar_builtins(fa[0], fb[0], da[0], db[0]);
    
    /* Test SSE intrinsics */
    checksum += test_sse_intrinsics(fa, fb, 16);
    
    /* Test inline assembly */
    checksum += test_inline_asm(fa[1], fb[1]);
    
    /* Test mixed comparisons */
    checksum += test_mixed_comparisons(da, db, 16);
    
    /* Additional tests with constant propagation */
    volatile float v1 = NAN;
    volatile float v2 = 1.0f;
    
    if (__builtin_isunordered(v1, v2)) checksum++;
    if (__builtin_isgreater(v2, v1)) checksum++;
    if (__builtin_isless(v1, v2)) checksum++;
    if (__builtin_islessequal(v1, v2)) checksum++;
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
