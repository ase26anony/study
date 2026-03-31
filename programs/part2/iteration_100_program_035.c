/* test_condition_codes.c - Cover GCC i386 condition code output routines */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to ensure code generation */
#define NOINLINE __attribute__((noinline))

/* Global volatile to prevent optimization */
volatile int global_result = 0;

/* Function 1: Scalar builtins with various condition codes */
NOINLINE int test_scalar_builtins(float a, float b) {
    int result = 0;
    
    /* UNORDERED case */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* ORDERED case */
    if (!__builtin_isunordered(a, b)) {
        result |= 2;
    }
    
    /* UNEQ case (unordered or equal) */
    if (__builtin_isunordered(a, b) || a == b) {
        result |= 4;
    }
    
    /* UNGE case (not less than) - with fast-math this may use nlt */
    if (__builtin_isgreaterequal(a, b)) {
        result |= 8;
    }
    
    /* UNGT case (not less or equal) - with fast-math this may use nle */
    if (__builtin_isgreater(a, b)) {
        result |= 16;
    }
    
    /* UNLE case (unordered or less or equal) */
    if (__builtin_islessequal(a, b)) {
        result |= 32;
    }
    
    /* UNLT case (unordered or less than) */
    if (__builtin_isless(a, b)) {
        result |= 64;
    }
    
    /* LTGT case (less than or greater than, but not equal/unordered) */
    if (a != b && !__builtin_isunordered(a, b)) {
        result |= 128;
    }
    
    return result;
}

/* Function 2: SSE/AVX vector intrinsics */
NOINLINE int test_vector_intrinsics(float *a, float *b, int n) {
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
        
        /* UNGT - _mm_cmpnle_ps (not less or equal) */
        __m128 mask_nle = _mm_cmpnle_ps(va, vb);
        
        /* UNLE - _mm_cmple_ps (less or equal) */
        __m128 mask_ule = _mm_cmple_ps(va, vb);
        
        /* UNLT - _mm_cmplt_ps (less than) */
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

/* Function 3: Inline assembly with condition code constraints */
NOINLINE int test_inline_asm(float a, float b) {
    int result = 0;
    int temp;
    
    /* UNORDERED - "u" flag */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setu %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* ORDERED - "np" flag (not parity) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setnp %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNEQ - "e" flag (equal) - unordered equal */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "sete %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNGE - "ae" flag (above or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setae %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNGT - "a" flag (above) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "seta %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNLE - "be" flag (below or equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setbe %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* UNLT - "b" flag (below) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setb %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    /* LTGT - "ne" flag (not equal) */
    asm volatile (
        "ucomiss %1, %2\n\t"
        "setne %0"
        : "=r"(temp)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += temp;
    
    return result;
}

/* Function 4: Mixed operations to trigger various patterns */
NOINLINE int test_mixed_operations(double *arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n - 1; i++) {
        double a = arr[i];
        double b = arr[i + 1];
        
        /* Use fpclassify to potentially generate condition codes */
        int ca = fpclassify(a);
        int cb = fpclassify(b);
        
        /* Various comparisons that might use different condition codes */
        if (isunordered(a, b)) result++;
        if (isgreater(a, b)) result++;
        if (isless(a, b)) result++;
        if (islessequal(a, b)) result++;
        if (isgreaterequal(a, b)) result++;
        
        /* Direct comparisons that might generate UNEQ/LTGT */
        if (a == b) result++;
        if (a != b) result++;
        
        /* Check for NaN specifically */
        if (isnan(a) || isnan(b)) result++;
    }
    
    return result;
}

/* Main test driver */
int main() {
    /* Initialize test data with mix of normal values and NaN */
    float scalar_a = 1.5f;
    float scalar_b = NAN;
    float scalar_c = 2.5f;
    float scalar_d = 2.5f;
    
    /* Array data for vector tests */
    #define ARRAY_SIZE 64
    float array_a[ARRAY_SIZE];
    float array_b[ARRAY_SIZE];
    double array_d[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (i % 8 == 0) ? NAN : (float)i * 0.1f;
        array_b[i] = (i % 5 == 0) ? NAN : (float)i * 0.2f;
        array_d[i] = (i % 7 == 0) ? NAN : (double)i * 0.3;
    }
    
    /* Run all tests */
    int checksum = 0;
    
    /* Test 1: Scalar builtins with different value combinations */
    checksum += test_scalar_builtins(scalar_a, scalar_b);  /* Normal vs NaN */
    checksum += test_scalar_builtins(scalar_b, scalar_c);  /* NaN vs Normal */
    checksum += test_scalar_builtins(scalar_c, scalar_d);  /* Equal normals */
    checksum += test_scalar_builtins(scalar_a, scalar_c);  /* Different normals */
    
    /* Test 2: Vector intrinsics */
    checksum += test_vector_intrinsics(array_a, array_b, ARRAY_SIZE);
    
    /* Test 3: Inline assembly */
    checksum += test_inline_asm(scalar_a, scalar_b);
    checksum += test_inline_asm(scalar_c, scalar_d);
    
    /* Test 4: Mixed operations */
    checksum += test_mixed_operations(array_d, ARRAY_SIZE);
    
    /* Store to volatile global to prevent optimization */
    global_result = checksum;
    
    /* Print result to ensure execution */
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
